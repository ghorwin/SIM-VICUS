/*subfile:  savevf.c  *********************************************************/
/*                                                                            */
/*  This file is part of View3D.                                              */
/*                                                                            */
/*  View3D is distributed in the hope that it will be useful, but             */
/*  WITHOUT ANY WARRANTY; without even the implied warranty of                */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                      */
/*                                                                            */
/*  This file has not been substantially changed from the original            */
/*  public domain version made available with the disclaimer below,           */
/*  and is thus in the public domain.                                         */
/*                                                                            */
/*  Original NIST Disclaimer:                                                 */
/*                                                                            */
/*  This software was developed at the National Institute of Standards        */
/*  and Technology by employees of the Federal Government in the              */
/*  course of their official duties. Pursuant to title 17 Section 105         */
/*  of the United States Code this software is not subject to                 */
/*  copyright protection and is in the public domain. These programs          */
/*  are experimental systems. NIST assumes no responsibility                  */
/*  whatsoever for their use by other parties, and makes no                   */
/*  guarantees, expressed or implied, about its quality, reliability,         */
/*  or any other characteristic.  We would appreciate acknowledgment          */
/*  if the software is used. This software can be redistributed and/or        */
/*  modified freely provided that any derivative works bear some              */
/*  notice that they are derived from it, and any modified versions           */
/*  bear some notice that they have been modified.                            */
/*                                                                            */
/******************************************************************************/

/*  Main program for batch processing of 3-D view factors.  */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "view3d.h"
#include "prtyp.h"

/***  SaveF0.c  **************************************************************/

/*  Save view factors as square array; + area + emit; text format.  */

void SaveF0(I1 *fileName, I1 *header, IX nSrf,
            R4 *area, R4 *emit, R8 **AF, R4 *F)
{
  FILE *vfout;
  IX n;    /* row */
  IX m;    /* column */

  vfout = fopen(fileName, "w");
  fprintf(vfout, "%s", header);
  fprintf(vfout, "%g", area[1]);
  for(n=2; n<=nSrf; n++) {
    fprintf(vfout, " %g", area[n]);
  }
  fprintf(vfout, "\n");

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    R8 Ainv = 1.0 / area[n];
    for(m=1; m<=nSrf; m++) {     /* process column values */
      if(m < n) {
        F[m] = (R4)(AF[n][m] * Ainv);
      } else {
        F[m] = (R4)(AF[m][n] * Ainv);
      }
    }
    fprintf(vfout, "%.6f", F[1]);   /* write row */
    for(m=2; m<=nSrf; m++) {
      fprintf(vfout, " %.6f", F[m]);
    }
    fprintf(vfout, "\n");
  }

  fprintf(vfout, "%.3f", emit[1]);
  for(n=2; n<=nSrf; n++) {
    fprintf(vfout, " %.3f", emit[n]);
  }
  fprintf(vfout, "\n");
  fclose(vfout);

}  /* end of SaveF0 */

/***  SaveF1.c  **************************************************************/

/*  Save view factors as square array; binary format. */

void SaveF1(I1 *fileName, I1 *header, IX nSrf,
            R4 *area, R4 *emit, R8 **AF, R4 *F)
{
  FILE *vfout;
  IX n;    /* row */

  vfout = fopen(fileName, "wb");
  fwrite(header, sizeof(I1), 32, vfout);
  fwrite(area+1, sizeof(R4), nSrf, vfout);

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    IX m;    /* column */
    R8 Ainv = 1.0 / area[n];
    for(m=1; m<=nSrf; m++) {     /* process column values */
      if(m < n) {
        F[m] = (R4)(AF[n][m] * Ainv);
      } else {
        F[m] = (R4)(AF[m][n] * Ainv);
      }
    }
    fwrite(F+1, sizeof(R4), nSrf, vfout);   /* write row */
  }

  fwrite(emit+1, sizeof(R4), nSrf, vfout);
  fclose(vfout);

}  /* end of SaveF1 */

/***  SaveAF.c  **************************************************************/

/*  Save view factors from 3D calculations.  */

void SaveAF(I1 *fileName, I1 *header, IX nSrf, I1 *title, I1 **name,
            R4 *area, R4 *emit, R8 **AF)
{
  FILE *vfout;
  IX n;    /* row */

  vfout = fopen(fileName, "w");
  fprintf(vfout, "%s", header);
  fprintf(vfout, "T %s\n", title);
  fprintf(vfout, "!  #     area          emit    name\n");

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    IX m;    /* column */
    fprintf(vfout, "%4d %14.6e %7.3f   %s\n",
            n, area[n], emit[n], name[n]);

    for(m=1; m<=n; m++) { /* process column values */
      fprintf(vfout, "%14.6e", AF[n][m]);
      if(m%5) {
        fputc(' ', vfout);
      } else {
        fputc('\n', vfout);
      }
    }
    if(m%5 != 1) {
      fputc('\n', vfout);
    }
  }
  fputc('\n', vfout);
  fclose(vfout);

}  /* end of SaveAF */

/***  SaveVF.c  **************************************************************/

/*  Save computed view factors.  */

void SaveVF(I1 *fileName, I1 *program, I1 *version,
            IX format, IX encl, IX didemit, IX nSrf,
            R4 *area, R4 *emit, R8 **AF, R4 *vtmp)
{
  I1 header[32];
  IX j;

  /* fill output file header line */
  sprintf(header, "%s %s %d %d %d %d",
          program, version, format, encl, didemit, nSrf);
  for(j=strlen(header); j<30; j++) {
    header[j] = ' ';
  }
  header[30] = '\n';
  header[31] = '\0';

  if(format == 0) { /* simple text file */
    SaveF0(fileName, header, nSrf, area, emit, AF, vtmp);
  } else if(format == 1) { /* simple binary file */
    header[30] = '\r';
    header[31] = '\n';
    SaveF1(fileName, header, nSrf, area, emit, AF, vtmp);
  } else {
    error(3, __FILE__, __LINE__, "Undefined format: %d", format);
    //    SaveAF(fileName, header, nSrf, title, name, area, emit, AF);
  }

}  /* end SaveVF */

