/*subfile:  viewht  **********************************************************/

/*  ViewWHT  Program to compute heat transfer rates.  */

#include <stdio.h>
#include <string.h> /* prototype: strcpy */
#include <stdlib.h> /* prototype: exit */
#include <ctype.h>  /* prototype: toupper */
#include <math.h>   /* prototype: sqrt */
#include "types.h" 
#include "prtyp.h" 

typedef struct surface {       /* structure for common surface data */
  R4 area;   /* surface area */
  R4 emit;   /* surface emittance */
  R4 Tabs;   /* temperature [K] */
  R4 T4;     /* temperature^4 */
} SURFACE;

void ReadVF(I1 *fileName, I1 *program, I1 *version,
            IX *format, IX *encl, IX *didemit, IX *nSrf,
            R4 *area, R4 *emit, R8 **AF, R4 **F, IX init, IX shape);
void ReadTK(I1 *fileName, IX nSrf, R4 *TK);

FILE *_ulog; /* log file */
FILE *_uout; /* output file */
I1 _string[LINELEN];  /* buffer for a character string */

/***  main.c  ****************************************************************/

IX main(IX argc, I1 **argv)
{
  I1 program[16];     /* program name */
  I1 version[8];      /* program version */
  IX format;          /* view factor file format; 0=text, 1=binary, ... */
  I1 inFile[_MAX_PATH]=""; /* input file name */
  I1 outFile[_MAX_PATH]=""; /* output file name */
  I1 tkFile[_MAX_PATH]=""; /* temperature file name */
  R4 **VF=NULL;    /* square array of view factors */
  R4 *TK=NULL;     /* vector of surface temperatures [K] */
  R4 *T4=NULL;     /* vector of surface temperatures ^ 4 */
  R4 *area=NULL;   /* vector of surface areas [1:nSrf] */
  R4 *emit=NULL;   /* vector of surface emittances [1:nSrf] */
  R4 *sumq=NULL;   /* vector of surface heat fluxes [1:nSrf] */
  R8 sumQ;
  R8 SB_const = 5.6697e-8;
  R8 **atmp=NULL;
  IX nSrf;    /* number of radiating surfaces */
  IX encl;    /* true = surfaces should form enclosure, not used */
  IX didemit; /* 1 = emittences included in view factors */
  R4 time0;
  IX n, m;

  if(argc > 1 && argv[1][0] == '?') { /* arcg = 1 -> enter file names */
    fputs("\n\
          VIEWHT - radiant heat transfer calculation.\n\n\
          ViewHT  VF_file  TK_file  output_file \n\n\
          VF_file of view factors is created by VIEW3D or VIEW2D. \n\
          TK_file sets the surface temperatures. \n",
          stderr);
    exit(1);
  }
  /* open log file */
  PgmInit(argv[0]);
  MemRem("Initial");

  if(argc > 1) {
    strcpy(inFile, argv[1]);
  }
  FileOpen("Enter name of gray View Factors file", inFile, "r", 0);
  fprintf(_ulog, "Data:  %s\n", inFile);

  if(argc > 2) {
    strcpy(tkFile, argv[2]);
  }
  FileOpen("Enter name of Temperatures file", tkFile, "r", 0);
  fprintf(_ulog, "T[K]:  %s\n", tkFile);

  if(argc > 3) {
    strcpy(outFile, argv[3]);
  }
  FileOpen("Enter name of Heat Flux output file", outFile, "w", 0);
  fprintf(_ulog, "Out:  %s\n", outFile);

  fprintf(stderr, "\n ViewHt running...\n\n");

  /* read view factor file */
  time0 = CPUtime(0.0);
  ReadVF(inFile, program, version, &format, &encl, &didemit, &nSrf,
         area, emit, atmp, VF, 1, 1);

  fprintf(_ulog, " total radiating surfaces: %3d\n", nSrf);
  fprintf(_ulog, "     enclosure designator: %3d\n", encl);
  fprintf(_ulog, "     emittance designator: %3d\n\n", didemit);

  VF = Alc_MC(0, nSrf, 0, nSrf, sizeof(R8), __FILE__, __LINE__);
  TK = Alc_V(1, nSrf, sizeof(R4), __FILE__, __LINE__);
  T4 = Alc_V(1, nSrf, sizeof(R4), __FILE__, __LINE__);
  area = Alc_V(1, nSrf, sizeof(R4), __FILE__, __LINE__);
  emit = Alc_V(1, nSrf, sizeof(R4), __FILE__, __LINE__);
  sumq = Alc_V(1, nSrf, sizeof(R4), __FILE__, __LINE__);

  ReadVF(inFile, program, version, &format, &encl, &didemit, &nSrf,
         area, emit, atmp, VF, 0, 1);

  ReadTK(tkFile, nSrf, TK);   /* read temperatures file */

  for(n=1; n<=nSrf; n++) {
    T4[n] = TK[n];
    T4[n] *= T4[n];
    T4[n] *= T4[n];
  }

  for(n=1; n<=nSrf; n++) { /* compute heat fluxes */
    R8 sum=0.0;
    for(m=1; m<=nSrf; m++) {
      sum += SB_const * VF[n][m] * (T4[m] - T4[n]);
    }
    sumq[n] = (R4)sum;
  }

  _uout = fopen(outFile, "w");
  if(!didemit) {
    for(n=1; n<=nSrf; n++) {
      emit[n] = 1.0;
    }
  }
  fprintf(_uout, "\nSrf #     area    emit    T [K]    q [W/m^2]      Q [W]\n");
  for(sumQ=0.0,n=1; n<=nSrf; n++) {
    fprintf(_uout, " %4d %8.3f %7.3f  %7.2f  %11.3e  %12.4e\n",
            n, area[n], emit[n], TK[n], sumq[n], sumq[n] * area[n]);
    sumQ += sumq[n] * area[n];
  }
  fprintf(_uout, "                                                 =========\n");
  fprintf(_uout, "                                     balance:  %11.3e\n", sumQ);
  fclose(_uout);

  fprintf(_ulog, "\n%7.2f seconds for all operations.\n\n", CPUtime(time0));

  Fre_V(sumq, 1, nSrf, sizeof(R4), __FILE__, __LINE__);
  Fre_V(area, 1, nSrf, sizeof(R4), __FILE__, __LINE__);
  Fre_V(emit, 1, nSrf, sizeof(R4), __FILE__, __LINE__);
  Fre_V(T4, 1, nSrf, sizeof(R4), __FILE__, __LINE__);
  Fre_V(TK, 1, nSrf, sizeof(R4), __FILE__, __LINE__);
  Fre_MC(VF, 0, nSrf, 0, nSrf, sizeof(R8), __FILE__, __LINE__);
  MemRem("Final");

  fprintf(stderr, "\nDone!\n");

  return 0;

}  /* end main - ViewHT */

/***  ReadTK.c  **************************************************************/

/*  Read surface temperatures [K].  */

void ReadTK(I1 *fileName, IX nSrf, R4 *TK)
{
  IX n, n1, n2;
  R4 Tabs;
  FILE *tfile;

  tfile = fopen(fileName, "r");
  for(;;) {
    fgets(_string, LINELEN, tfile);
    sscanf(_string, "%d %d %f", &n1, &n2, &Tabs);
    if(n1 < 1) {
      break;
    }
    if(n2 < 1) {
      break;
    }
    if(n1>nSrf) {
      n1 = nSrf;
    }
    if(n2>nSrf) {
      n2 = nSrf;
    }
    if(n2<n1) {
      n = n1;
      n1 = n2;
      n2 = n;
    }

    for(n=n1; n<=n2; n++) {
      TK[n] = Tabs;
    }
  }

}  /* end of ReadTK */

