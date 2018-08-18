/* TriadexEngine object */

/* triadex0 was back from gdb online */

class TriadexEngine {
  public:
   void setIntervalAndTheme(int *, int *);   /* as slot #s, see slider */
   void halfStep(void);             /* half step the engine */
   void reset(void);                /* clear counters and shift register */

   unsigned int bitValue(int);      /* need for lamp control - add setBitValue one day? */
   float getNote(void);             /* the note as (octave << 3) + scale[0..7] so /12 is a CV <- NOT! */
   
   void getTriadexState(int *);        /* JSON kludge? */
   void setTriadexState(int *);

/* everything else is */
  private:

   unsigned int calculateLFSRBit();
   int *interval; int *theme; /* kludge - sorry Mom! */

/* Triadex state variables */
   unsigned int count6, count32, count3;
   unsigned int lfsrBits;

/* fixed major scale. 'C' appears twice in a run up - Triadex! */
   const int scale[8] = {0, 2, 4, 5, 7, 9, 11, 12};

/* slider 0..39 select interval and theme bits, follow
   CONST and 0 or 1 OFF, ON
   COUNT32 and bit select mask - count32
   COUNT6 and bit select mask - count6
   LFSR and bit select mask - lfsrBits
*/
   enum bitType {
      CONST, COUNT32, COUNT6, LFSR
   };

   const struct {
      int controlBitType;
      unsigned int arg;
   }  slider[40] = {
   /*  0 */     {LFSR, 0x1 << 30},
   /*  1 */    {LFSR, 0x1 << 29},
   /*  2 */    {LFSR, 0x1 << 28},
   /*  3 */    {LFSR, 0x1 << 27},
   /*  4 */    {LFSR, 0x1 << 26},
   /*  5 */    {LFSR, 0x1 << 25},
   /*  6 */    {LFSR, 0x1 << 24},
   /*  7 */    {LFSR, 0x1 << 23},
   /*  8 */    {LFSR, 0x1 << 22},
   /*  9 */    {LFSR, 0x1 << 21},
   /* 10 */    {LFSR, 0x1 << 20},
   /* 11 */    {LFSR, 0x1 << 19},
   /* 12 */    {LFSR, 0x1 << 18},
   /* 13 */    {LFSR, 0x1 << 17},
   /* 14 */    {LFSR, 0x1 << 16},
   /* 15 */    {LFSR, 0x1 << 15},
   /* 16 */    {LFSR, 0x1 << 14},
   /* 17 */    {LFSR, 0x1 << 13},
   /* 18 */    {LFSR, 0x1 << 12},
   /* 19 */    {LFSR, 0x1 << 11},
   /* 20 */    {LFSR, 0x1 << 10},
   /* 21 */    {LFSR, 0x1 << 9},
   /* 22 */    {LFSR, 0x1 << 8},
   /* 23 */    {LFSR, 0x1 << 7},
   /* 24 */    {LFSR, 0x1 << 6},
   /* 25 */    {LFSR, 0x1 << 5},
   /* 26 */    {LFSR, 0x1 << 4},
   /* 27 */    {LFSR, 0x1 << 3},
   /* 28 */    {LFSR, 0x1 << 2},
   /* 29 */    {LFSR, 0x1 << 1},
   /* 30 */    {LFSR, 0x1 },
   /* 31 */    {COUNT6, 0x2},
   /* 32 */    {COUNT6, 0x1},
   /* 33 */    {COUNT32, 0x1 << 4},
   /* 34 */    {COUNT32, 0x1 << 3},
   /* 35 */    {COUNT32, 0x1 << 2},
   /* 36 */    {COUNT32, 0x1 << 1},
   /* 37 */    {COUNT32, 0x1 },
   /* 38 */    {CONST, 1},
   /* 39 */    {CONST, 0},
   };
}; 

void TriadexEngine::getTriadexState(int *vals)
{
   vals[0] = count32;
   vals[1] = count3;
   vals[2] = count6;
   vals[3] = lfsrBits;
}

void TriadexEngine::setTriadexState(int *vals)
{
   count32 = vals[0];
   count3 = vals[1];
   count6 = vals[2];
   lfsrBits = vals[3];
}

unsigned int TriadexEngine::bitValue(int slot)
{
   switch (slider[slot].controlBitType) {
   case CONST :
      return !!slider[slot].arg;
      
   case COUNT6 :
      return !!(count6 & slider[slot].arg);

   case COUNT32 :
      return !!(count32 & slider[slot].arg);
   
   case LFSR :
      return !!(lfsrBits & slider[slot].arg);
   }
   
   return (-1);   /* shouldn't */
}

void TriadexEngine::setIntervalAndTheme(int *interval, int *theme)
{
   this->interval = interval;
   this->theme = theme;
}

float TriadexEngine::getNote()
{
   int note = 0;

   for (int i = 0, j = 1; i < 4; i++, j <<= 1)
      note += bitValue(interval[i]) ? j : 0; /* can ya bool << i ? */
   
   return ((scale[note & 0x7] + ((note & 0x8) ? 12 : 0)) / 12.0f);
}

unsigned int TriadexEngine::calculateLFSRBit()
{
   unsigned int bit = 1u;      /* XNOR, right? :-) */
   
   for (int i = 0; i < 4; i++) //{
      bit ^= bitValue(theme[i]);

   return (bit);
}

void TriadexEngine::halfStep()
{
   unsigned int bit;

   count32++; count32 &= 0x1f;
   if (count32 & 0x1)
      return;        /* rising edge, we done */

   if (++count3 == 3) {
      count3 = 0;
      count6++;
      count6 &= 0x3;
   }

   bit = calculateLFSRBit();
   lfsrBits <<= 1;
   lfsrBits |= bit;
   lfsrBits &= 0x7fffffff;
}

/* to see lights before any step Seg faulting... */
void TriadexEngine::reset()
{
   count3 = 0;
   count6 = 0;
   count32 = 0;
   
   lfsrBits = 0x0;
}

/* teset program stripped, see learningcpp/triadex.cpp */
