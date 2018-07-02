/* Card type flags (CardType) */
#define CT_MMC    0x01    /* MMC ver 3 */
#define CT_SD1    0x02    /* SD ver 1 */
#define CT_SD2    0x04    /* SD ver 2 */
#define CT_SDC    (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK  0x08    /* Block addressing */

#ifdef __cplusplus
extern "C" {
#endif

#include "diskio.h"

extern void disk_timerproc (void);

#ifdef __cplusplus
}
#endif
