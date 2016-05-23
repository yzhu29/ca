#include <iostream>

using namespace std;

#include <math.h>
#define SHARINGTABLES //@
#define LOOPPREDICTOR	//loop predictor enable
#define NNN 1			// number of entries allocated on a TAGE misprediction

// number of tagged tables
#define NHIST 15


#define HYSTSHIFT 2 // bimodal hysteresis shared by 4 entries
//#define LOGB 14 // log of number of entries in bimodal predictor
#define LOGB 16

#define PERCWIDTH 6 //Statistical coorector maximum counter width

//The statistical corrector components

//global branch GEHL
//#define LOGGNB 9
#define LOGGNB 11
// LOGGNB: log of number of entries in global conditional branch history GEHL predictor
#define GNB 4
// GNB is the number of tables of global conditional branch history GEHL
int Gm[GNB] = {16,11, 6,3};
// 16, 11, 6, 3 are lengths of each table
// Each Gm entry contains the length of a table
int8_t GGEHLA[GNB][(1 << LOGGNB)];
// GGEHLA[][]: The "PHT" table of each predictor.
int8_t *GGEHL[GNB];
// GGEHL: each GGEHL will point to a GGEHLA which is the "PHT" table of each predictor.

//large local history
//#define LOGLNB 10
#define LOGLNB 12
#define LNB 3
int Lm[LNB] = {11,6,3};
int8_t LGEHLA[LNB][(1 << LOGLNB)];
int8_t *LGEHL[LNB];

//#define  LOGLOCAL 8
#define  LOGLOCAL 10
#define NLOCAL (1<<LOGLOCAL)
// NLOCAL: number of local history entries
#define INDLOCAL (PC & (NLOCAL-1))
long long L_shist[NLOCAL];
// Each L_shist entry contains a local history


// small local history
//#define LOGSNB 9
#define LOGSNB 11
#define SNB 4
int Sm[SNB] =  {16,11, 6, 3};
int8_t SGEHLA[SNB][(1 << LOGSNB)];
int8_t *SGEHL[SNB];
//#define LOGSECLOCAL 4
#define LOGSECLOCAL 6
#define NSECLOCAL (1<<LOGSECLOCAL)	//Number of second local histories
#define INDSLOCAL  (((PC ^ (PC >>5))) & (NSECLOCAL-1))
long long S_slhist[NSECLOCAL];


//#define LOGTNB 9
#define LOGTNB 11
#define TNB 3
int Tm[TNB] =  {11, 6, 3};
int8_t TGEHLA[TNB][(1 << LOGTNB)];
int8_t *TGEHL[TNB];
#define INDTLOCAL  (((PC ^ (PC >>3))) & (NSECLOCAL-1))	// differen hash for thehistory
long long T_slhist[NSECLOCAL];

//return-stack associated history component
#define PNB 4
//#define LOGPNB 9
#define LOGPNB 11
int Pm[PNB] ={16,11,6,3};
int8_t PGEHLA[PNB][(1 << LOGPNB)];
int8_t *PGEHL[PNB];
long long HSTACK[16];
int pthstack;


//parameters of the loop predictor
//#define LOGL 5
#define LOGL 7
#define WIDTHNBITERLOOP 10	// we predict only loops with less than 1K iterations
#define LOOPTAG 10		//tag width in the loop predictor


//update threshold for the statistical corrector
//#define LOGSIZEUP 5
#define LOGSIZEUP 7
int Pupdatethreshold[(1 << LOGSIZEUP)];	//size is fixed by LOGSIZEUP
#define INDUPD (PC & ((1 << LOGSIZEUP) - 1))

// The three counters used to choose between TAGE ang SC on High Conf TAGE/Low Conf SC
int8_t  FirstH, SecondH, ThirdH;
#define CONFWIDTH 7	//for the counters in the choser


#define PHISTWIDTH 27		// width of the path history used in TAGE




#define UWIDTH 2 // u counter width on TAGE
#define CWIDTH 3		// predictor counter width on the TAGE tagged tables

#define HISTBUFFERLENGTH 4096	// we use a 4K entries history buffer to store the branch history


//the counter(s) to chose between longest match and alternate prediction on TAGE when weak counters
//#define LOGSIZEUSEALT 8
#define LOGSIZEUSEALT 10
#define SIZEUSEALT  (1<<(LOGSIZEUSEALT))
#define INDUSEALT (PC & (SIZEUSEALT -1))
int8_t use_alt_on_na[SIZEUSEALT][2];



long long GHIST;


//The two BIAS tables in the SC component
//#define LOGBIAS 7
#define LOGBIAS 9
int8_t Bias[(1<<(LOGBIAS+1))];
#define INDBIAS (((PC<<1) + pred_inter) & ((1<<(LOGBIAS+1)) -1))
int8_t BiasSK[(1<<(LOGBIAS+1))];
#define INDBIASSK ((((PC^(PC>>LOGBIAS))<<1) + pred_inter) & ((1<<(LOGBIAS+1)) -1))

int m[NHIST+1]={0,6,10,18,25,35,55,69,105,155,230,354,479,642,1012,1347}; // history lengths
#ifndef SHARINGTABLES

int TB[NHIST + 1]={0,7,9,9,9,10,11,11,12,12,12,13,14,15,15,15};		// tag width for the different tagged tables
//int logg[NHIST + 1]={0,10,10,10,11,10,10,10,10,10,9,9,9,8,7,7};		// log of number entries of the different tagged tables
int logg[NHIST + 1]={0,12,12,12,13,12,12,12,12,12,11,11,11,10,9,9};
#endif // SHARINGTABLES

#ifdef SHARINGTABLES //@

#define STEP0 1 //@
#define STEP1 3 //@
#define STEP2 8 //@
#define STEP3 14 //@
#define LOGG 12 //@
    int logg[NHIST + 1];		// log of number entries of the different tagged tables //@
    int TB[NHIST + 1];		// tag width for the different tagged tables //@
#endif
int
predictorsize ()
{
  int STORAGESIZE = 0;
  int inter=0;

#ifdef SHARINGTABLES
    STORAGESIZE += (1 << (logg[1])) * (CWIDTH + 1 + TB[1]);
    STORAGESIZE += (1 << (logg[STEP1])) * (CWIDTH + 1 + TB[STEP1]);
    STORAGESIZE += (1 << (logg[STEP2])) * (CWIDTH + 1 + TB[STEP2]);
    STORAGESIZE += (1 << (logg[STEP3])) * (CWIDTH + 1 + TB[STEP3]);
#endif

#ifndef SHARINGTABLES
   for (int i = 1; i <= NHIST; i += 1)
    {
      STORAGESIZE += (1 << (logg[i])) * (CWIDTH + UWIDTH + TB[i]);

    }
#endif
 STORAGESIZE += 2 * (SIZEUSEALT) * 4;
STORAGESIZE += (1 << LOGB) + (1 << (LOGB - HYSTSHIFT));
STORAGESIZE+= m[NHIST];
STORAGESIZE += PHISTWIDTH;
STORAGESIZE += 10 ; //the TICK counter

fprintf (stderr, " (TAGE %d) ", STORAGESIZE);

#ifdef LOOPPREDICTOR

  inter= (1 << LOGL) * (2 * WIDTHNBITERLOOP + LOOPTAG + 4 + 4 + 1);fprintf (stderr, " (LOOP %d) ", inter);
  STORAGESIZE+= inter;

#endif


  inter = 8 * (1 << LOGSIZEUP) ; //the update threshold counters
inter += (PERCWIDTH) * 4 * (1 << (LOGBIAS));
inter += (GNB-2) * (1 << (LOGGNB)) * (PERCWIDTH - 1) + (1 << (LOGGNB-1))*(2*PERCWIDTH-1);
inter += (LNB-2) * (1 << (LOGLNB)) * (PERCWIDTH - 1) + (1 << (LOGLNB-1))*(2*PERCWIDTH-1);

#ifndef REALISTIC
inter += (SNB-2) * (1 << (LOGSNB)) * (PERCWIDTH - 1) + (1 << (LOGSNB-1))*(2*PERCWIDTH-1);
inter += (TNB-2) * (1 << (LOGTNB)) * (PERCWIDTH - 1) + (1 << (LOGTNB-1))*(2*PERCWIDTH-1);
inter += (PNB-2) * (1 << (LOGPNB)) * (PERCWIDTH - 1) + (1 << (LOGPNB-1))*(2*PERCWIDTH-1);



inter += 16*16; // the history stack
inter += 4; // the history stack pointer
     inter += 16; //global histories for SC

  inter += NSECLOCAL * (Sm[0]+Tm[0]);
#endif
  inter += NLOCAL * Lm[0];
inter += 3*CONFWIDTH; //the 3 counters in the choser
STORAGESIZE+= inter;
fprintf (stderr, " (SC %d) ", inter);



#ifdef PRINTSIZE
  fprintf (stderr, " (TOTAL %d) ", STORAGESIZE);
#endif


  return (STORAGESIZE);


}

int main()
{
#ifdef SHARINGTABLES
    //@ begin
    // log2 of number entries in the tagged components
	// LOGG = 11
    logg[1] = LOGG + 1;
	// if sharing: logg[1] = 12;
	// if not sharing: logg[1] = logg[2] = 11
    logg[STEP1] = LOGG + 3;
	// if sharing: logg[3] = 14
	// if not sharing: logg[3] = 11, logg[4] to logg[6] = 12
    //logg[STEP2] = LOGG + 2; // logg[8] = 13
    logg[STEP2] = LOGG + 1; // logg[8] = 13
	// if not sharing: logg[7] to logg [10] = 11
    logg[STEP3] = LOGG - 1; // logg[14] = 10
	// if not sharing: logg[10] to logg[15] = 10
    for (int i = 2; i <= STEP1 - 1; i++)
      logg[i] = logg[1] - 3;	/* grouped together 4Kentries */	// logg[2] = 9
    for (int i = STEP1 + 1; i <= STEP2 - 1; i++)
      logg[i] = logg[STEP1] - 3;	/*grouped together 16K entries */	// logg[4] = logg[5] = logg[6] = logg[7] = 11
    for (int i = STEP2 + 1; i <= STEP3 - 1; i++)
      logg[i] = logg[STEP2] - 3;	/*grouped together 8K entries */ // logg[9] = logg[10] = logg[11] = logg[12] = logg[13] = 10
    for (int i = STEP3 + 1; i <= NHIST; i++)
      logg[i] = logg[STEP3] - 3;	/*grouped together 1Kentries */ // logg[15] = 7
	// if sharing: total # of entries = 2^12 + 2^14 + 2^13 + 2^10 = 29K entries
	// if not sharing: total # of entries = 3*2^11 + 3*2^12 + 4*2^11 + 6*2^10 = 32Kentries

	//grouped tables have the same tag width
// 1 (STEP0): 1 to 2 point to gtable[1]
// STEP 1: 3 to 7 point to gtable[STEP1]
// STEP 2: 8 to 13 point to gtable[STEP2]
// STEP 3: 14 to 15 point to gtable[STEP3]
    for (int i = 1; i <= STEP1 - 1; i++)
      {
	//gtable[i] = gtable[1];
	TB[i] = 8;
      }
    for (int i = STEP1; i <= STEP2 - 1; i++)
      {
	//gtable[i] = gtable[STEP1];
	TB[i] = 11;
      }
    for (int i = STEP2; i <= STEP3 - 1; i++)
      {
	//gtable[i] = gtable[STEP2];
	TB[i] = 13;
      }
    for (int i = STEP3; i <= NHIST; i++)
      {
	//gtable[i] = gtable[STEP3];
	TB[i] = 14;
      }
	// @end

#endif // SHARINGTABLES

    predictorsize ();
    cout << "Hello world!" << endl;
    return 0;
}
