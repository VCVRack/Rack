/*-----------------------------------------------------------------------*/
/* MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2010           */
/*-----------------------------------------------------------------------*/
/* Only rcvr_spi(), xmit_spi(), disk_timerproc() and some macros         */
/* are platform dependent.                                               */
/*-----------------------------------------------------------------------*/


#include <avr/io.h>
#include "diskio.h"
#include "mmc.h"


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Port Controls  (Platform dependent) */

#define CS_LOW()  MMC_CS_PORT &= ~(1 << MMC_CS_BIT)     /* MMC CS = L */
#define CS_HIGH() MMC_CS_PORT |= (1 << MMC_CS_BIT)      /* MMC CS = H */

#define SOCKWP    0   /* Write protected. yes:true, no:false, default:false */
#define SOCKINS   1  /* Card detected.   yes:true, no:false, default:true */

#define FCLK_SLOW() SPCR = 0x52   /* Set slow clock (100k-400k) */
#define FCLK_FAST() SPCR = 0x50   /* Set fast clock (depends on the CSD) */


/* Definitions for MMC/SDC command */
#define CMD0  (0)     /* GO_IDLE_STATE */
#define CMD1  (1)     /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41) /* SEND_OP_COND (SDC) */
#define CMD8  (8)     /* SEND_IF_COND */
#define CMD9  (9)     /* SEND_CSD */
#define CMD10 (10)    /* SEND_CID */
#define CMD12 (12)    /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13) /* SD_STATUS (SDC) */
#define CMD16 (16)    /* SET_BLOCKLEN */
#define CMD17 (17)    /* READ_SINGLE_BLOCK */
#define CMD18 (18)    /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)    /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)    /* WRITE_BLOCK */
#define CMD25 (25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD55 (55)    /* APP_CMD */
#define CMD58 (58)    /* READ_OCR */



static volatile
DSTATUS Stat = STA_NOINIT;  /* Disk status */

static volatile
BYTE Timer1, Timer2;  /* 100Hz decrement timer */

static
BYTE CardType;      /* Card type flags */


/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

#define xmit_spi(dat)   SPDR=(dat); loop_until_bit_is_set(SPSR,SPIF)



/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
BYTE rcvr_spi (void)
{
  SPDR = 0xFF;
  loop_until_bit_is_set(SPSR, SPIF);
  return SPDR;
}

/* Alternative macro to receive data fast */
#define rcvr_spi_m(dst) SPDR=0xFF; loop_until_bit_is_set(SPSR,SPIF); *(dst)=SPDR



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void) /* 1:OK, 0:Timeout */
{
  Timer2 = 50;  /* Wait for ready in timeout of 500ms */
  rcvr_spi();
  do
    if (rcvr_spi() == 0xFF) return 1;
  while (Timer2);

  return 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
  CS_HIGH();
  rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static
int select (void) /* 1:Successful, 0:Timeout */
{
  CS_LOW();
  if (!wait_ready()) {
    deselect();
    return 0;
  }
  return 1;
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
int power_status(void)    /* Socket power state: 0=off, 1=on */
{
  return 1;
}

static
void power_on (void)
{
  for (Timer1 = 2; Timer1; ); /* Wait for 20ms */
#ifndef MMC_NO_SPI_INITIALIZATION
  PORTB = 0b10110101;     /* Enable drivers */
  DDRB  = 0b11000111;
  SPCR = 0x52;      /* Enable SPI function in mode 0 */
  SPSR = 0x01;      /* SPI 2x mode */
#endif  // MMC_NO_SPI_INITIALIZATION
}


static
void power_off (void)
{
#ifndef MMC_NO_SPI_INITIALIZATION
  SPCR = 0;       /* Disable SPI function */
  DDRB  = 0b11000000;   /* Disable drivers */
  PORTB = 0b10110000;
#endif  // MMC_NO_SPI_INITIALIZATION
  Stat |= STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (
  BYTE *buff,     /* Data buffer to store received data */
  UINT btr      /* Byte count (must be multiple of 4) */
)
{
  BYTE token;


  Timer1 = 20;
  do {              /* Wait for data packet in timeout of 200ms */
    token = rcvr_spi();
  } while ((token == 0xFF) && Timer1);
  if(token != 0xFE) return 0;   /* If not valid data token, retutn with error */

  do {              /* Receive the data block into buffer */
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
  } while (btr -= 4);
  rcvr_spi();           /* Discard CRC */
  rcvr_spi();

  return 1;           /* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

static
int xmit_datablock (
  const BYTE *buff, /* 512 byte data block to be transmitted */
  BYTE token      /* Data/Stop token */
)
{
  BYTE resp, wc;


  if (!wait_ready()) return 0;

  xmit_spi(token);          /* Xmit data token */
  if (token != 0xFD) {  /* Is data token */
    wc = 0;
    do {              /* Xmit the 512 byte data block to MMC */
      xmit_spi(*buff++);
      xmit_spi(*buff++);
    } while (--wc);
    xmit_spi(0xFF);         /* CRC (Dummy) */
    xmit_spi(0xFF);
    resp = rcvr_spi();        /* Reveive data response */
    if ((resp & 0x1F) != 0x05)    /* If not accepted, return with error */
      return 0;
  }

  return 1;
}



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (   /* Returns R1 resp (bit7==1:Send failed) */
  BYTE cmd,   /* Command index */
  DWORD arg   /* Argument */
)
{
  BYTE n, res;


  if (cmd & 0x80) { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;
    res = send_cmd(CMD55, 0);
    if (res > 1) return res;
  }

  /* Select the card and wait for ready */
  deselect();
  if (!select()) return 0xFF;
  
  /* Send command packet */
  rcvr_spi();
  xmit_spi(0x40 | cmd);       /* Start + Command index */
  xmit_spi((BYTE)(arg >> 24));    /* Argument[31..24] */
  xmit_spi((BYTE)(arg >> 16));    /* Argument[23..16] */
  xmit_spi((BYTE)(arg >> 8));     /* Argument[15..8] */
  xmit_spi((BYTE)arg);        /* Argument[7..0] */
  n = 0x01;             /* Dummy CRC + Stop */
  if (cmd == CMD0) n = 0x95;      /* Valid CRC for CMD0(0) */
  if (cmd == CMD8) n = 0x87;      /* Valid CRC for CMD8(0x1AA) */
  xmit_spi(n);

  /* Receive command response */
  if (cmd == CMD12) rcvr_spi();   /* Skip a stuff byte when stop reading */
  n = 10;               /* Wait for a valid response in timeout of 10 attempts */
  do
    res = rcvr_spi();
  while ((res & 0x80) && --n);

  return res;     /* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE drv    /* Physical drive nmuber (0) */
)
{
  BYTE n, cmd, ty, ocr[4];


  if (drv) return STA_NOINIT;     /* Supports only single drive */
  if (Stat & STA_NODISK) return Stat; /* No card in the socket */

  power_on();             /* Force socket power on */
  FCLK_SLOW();
  for (n = 10; n; n--) rcvr_spi();  /* 80 dummy clocks */
  ty = 0;
  if (send_cmd(CMD0, 0) == 1) {     /* Enter Idle state */
    Timer1 = 100;           /* Initialization timeout of 1000 msec */
    if (send_cmd(CMD8, 0x1AA) == 1) { /* SDv2? */
      for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();    /* Get trailing return value of R7 resp */
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {       /* The card can work at vdd range of 2.7-3.6V */
        while (Timer1 && send_cmd(ACMD41, 1UL << 30));  /* Wait for leaving idle state (ACMD41 with HCS bit) */
        if (Timer1 && send_cmd(CMD58, 0) == 0) {    /* Check CCS bit in the OCR */
          for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
          ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* SDv2 */
        }
      }
    } else {              /* SDv1 or MMCv3 */
      if (send_cmd(ACMD41, 0) <= 1)   {
        ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
      } else {
        ty = CT_MMC; cmd = CMD1;  /* MMCv3 */
      }
      while (Timer1 && send_cmd(cmd, 0));     /* Wait for leaving idle state */
      if (!Timer1 || send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
        ty = 0;
    }
  }
  CardType = ty;
  deselect();

  if (ty) {     /* Initialization succeded */
    Stat &= ~STA_NOINIT;    /* Clear STA_NOINIT */
    FCLK_FAST();
  } else {      /* Initialization failed */
    power_off();
  }

  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv    /* Physical drive nmuber (0) */
)
{
  if (drv) return STA_NOINIT;   /* Supports only single drive */
  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
  BYTE drv,     /* Physical drive nmuber (0) */
  BYTE *buff,     /* Pointer to the data buffer to store read data */
  DWORD sector,   /* Start sector number (LBA) */
  BYTE count      /* Sector count (1..255) */
)
{
  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert to byte address if needed */

  if (count == 1) { /* Single block read */
    if ((send_cmd(CMD17, sector) == 0)  /* READ_SINGLE_BLOCK */
      && rcvr_datablock(buff, 512))
      count = 0;
  }
  else {        /* Multiple block read */
    if (send_cmd(CMD18, sector) == 0) { /* READ_MULTIPLE_BLOCK */
      do {
        if (!rcvr_datablock(buff, 512)) break;
        buff += 512;
      } while (--count);
      send_cmd(CMD12, 0);       /* STOP_TRANSMISSION */
    }
  }
  deselect();

  return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
  BYTE drv,     /* Physical drive nmuber (0) */
  const BYTE *buff, /* Pointer to the data to be written */
  DWORD sector,   /* Start sector number (LBA) */
  BYTE count      /* Sector count (1..255) */
)
{
  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if (Stat & STA_PROTECT) return RES_WRPRT;

  if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert to byte address if needed */

  if (count == 1) { /* Single block write */
    if ((send_cmd(CMD24, sector) == 0)  /* WRITE_BLOCK */
      && xmit_datablock(buff, 0xFE))
      count = 0;
  }
  else {        /* Multiple block write */
    if (CardType & CT_SDC) send_cmd(ACMD23, count);
    if (send_cmd(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
      do {
        if (!xmit_datablock(buff, 0xFC)) break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(0, 0xFD)) /* STOP_TRAN token */
        count = 1;
    }
  }
  deselect();

  return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE drv,   /* Physical drive nmuber (0) */
  BYTE ctrl,    /* Control code */
  void *buff    /* Buffer to send/receive control data */
)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = (BYTE*)buff;
  WORD csize;


  if (drv) return RES_PARERR;

  res = RES_ERROR;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (ctrl) {
    case CTRL_SYNC :    /* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
      if (select()) {
        deselect();
        res = RES_OK;
      }
      break;

    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
        if ((csd[0] >> 6) == 1) { /* SDC ver 2.00 */
          csize = csd[9] + ((WORD)csd[8] << 8) + 1;
          *(DWORD*)buff = (DWORD)csize << 10;
        } else {          /* SDC ver 1.XX or MMC*/
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
          *(DWORD*)buff = (DWORD)csize << (n - 9);
        }
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = 512;
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE : /* Get erase block size in unit of sector (DWORD) */
      if (CardType & CT_SD2) {  /* SDv2? */
        if (send_cmd(ACMD13, 0) == 0) { /* Read SD status */
          rcvr_spi();
          if (rcvr_datablock(csd, 16)) {        /* Read partial block */
            for (n = 64 - 16; n; n--) rcvr_spi(); /* Purge trailing data */
            *(DWORD*)buff = 16UL << (csd[10] >> 4);
            res = RES_OK;
          }
        }
      } else {          /* SDv1 or MMCv3 */
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {  /* Read CSD */
          if (CardType & CT_SD1) {  /* SDv1 */
            *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
          } else {          /* MMCv3 */
            *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
          }
          res = RES_OK;
        }
      }
      break;

    case MMC_GET_TYPE :   /* Get card type flags (1 byte) */
      *ptr = CardType;
      res = RES_OK;
      break;

    case MMC_GET_CSD :    /* Receive CSD as a data block (16 bytes) */
      if (send_cmd(CMD9, 0) == 0    /* READ_CSD */
        && rcvr_datablock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_CID :    /* Receive CID as a data block (16 bytes) */
      if (send_cmd(CMD10, 0) == 0   /* READ_CID */
        && rcvr_datablock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_OCR :    /* Receive OCR as an R3 resp (4 bytes) */
      if (send_cmd(CMD58, 0) == 0) {  /* READ_OCR */
        for (n = 4; n; n--) *ptr++ = rcvr_spi();
        res = RES_OK;
      }
      break;

    case MMC_GET_SDSTAT : /* Receive SD statsu as a data block (64 bytes) */
      if (send_cmd(ACMD13, 0) == 0) { /* SD_STATUS */
        rcvr_spi();
        if (rcvr_datablock(ptr, 64))
          res = RES_OK;
      }
      break;

    default:
      res = RES_PARERR;
  }

  deselect();

  return res;
}

/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure                                      */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void disk_timerproc (void)
{
  BYTE n, s;


  n = Timer1;       /* 100Hz decrement timer */
  if (n) Timer1 = --n;
  n = Timer2;
  if (n) Timer2 = --n;

  s = Stat;

  if (SOCKWP)       /* Write protected */
    s |= STA_PROTECT;
  else          /* Write enabled */
    s &= ~STA_PROTECT;

  if (SOCKINS)      /* Card inserted */
    s &= ~STA_NODISK;
  else          /* Socket empty */
    s |= (STA_NODISK | STA_NOINIT);

  Stat = s;       /* Update MMC status */
}
