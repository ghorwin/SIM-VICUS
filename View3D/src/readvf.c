/*subfile:  readvf.c  *********************************************************/
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

/*  Function to read View3D 3.2 view factor files.  */

#include <stdio.h>
#include "types.h"
IX error(IX severity, I1 *file, IX line, ...);

/***  ReadF0s.c  *************************************************************/

/*  Read view factors + area + emit; text format.  Save in square array.  */

void ReadF0s(I1 *fileName, IX nSrf, R4 *area, R4 *emit, R4 **F)
{
  FILE *vfin;
  I1 header[36];
  IX n;    /* row */
  IX m;    /* column */

  vfin = fopen(fileName, "r");
  fgets(header, 35, vfin);
  for(n=1; n<=nSrf; n++) {
    fscanf(vfin, "%f", &area[n]);
  }

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    for(m=1; m<=nSrf; m++) {     /* process column values */
      fscanf(vfin, "%f", &F[n][m]);
    }
  }

  for(n=1; n<=nSrf; n++) {
    fscanf(vfin, "%f", &emit[n]);
  }
  fclose(vfin);

}  /* end of SaveF0s */

/***  ReadF0t.c  *************************************************************/

/*  Read view factors + area + emit; text format. Save in triangular array.  */

void ReadF0t(I1 *fileName, IX nSrf, R4 *area, R4 *emit, R8 **AF)
{
  FILE *vfin;
  I1 header[36];
  IX n;    /* row */
  IX m;    /* column */

  vfin = fopen(fileName, "r");
  fgets(header, 35, vfin);
  for(n=1; n<=nSrf; n++) {
    fscanf(vfin, "%f", &area[n]);
  }

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    R4 F;
    for(m=1; m<=n; m++) {     /* process column values */
      fscanf(vfin, "%f", &F);
      AF[n][m] = F * area[n];
    }
    for(; m<=nSrf; m++) {
      fscanf(vfin, "%f", &F);
    }
  }

  for(n=1; n<=nSrf; n++) {
    fscanf(vfin, "%f", &emit[n]);
  }
  fclose(vfin);

}  /* end of SaveF0t */

/***  ReadF1s.c  *************************************************************/

/*  Read view factors + area + emit; binary format.  Save in square array.  */

void ReadF1s(I1 *fileName, IX nSrf, R4 *area, R4 *emit, R4 **F)
{
  FILE *vfin;
  I1 header[36];
  IX n;    /* row */

  vfin = fopen(fileName, "rb");
  fread(header, sizeof(I1), 32, vfin);
  fread(area+1, sizeof(R4), nSrf, vfin);

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    fread(F[n]+1, sizeof(R4), nSrf, vfin);
  }

  fread(emit+1, sizeof(R4), nSrf, vfin);
  fclose(vfin);

}  /* end of SaveF1s */

/***  ReadF1t.c  *************************************************************/

/* Read view factors + area + emit; binary format. Save in triangular array. */

void ReadF1t(I1 *fileName, IX nSrf, R4 *area, R4 *emit, R8 **AF)
{
  FILE *vfin;
  I1 header[36];
  IX n;    /* row */
  IX m;    /* column */

  vfin = fopen(fileName, "rb");
  fread(header, sizeof(I1), 32, vfin);
  fread(area+1, sizeof(R4), nSrf, vfin);

  for(n=1; n<=nSrf; n++) {     /* process AF values for row n */
    fread(emit+1, sizeof(R4), nSrf, vfin);  /* read F into emit */
    for(m=1; m<=n; m++) {     /* process column values */
      AF[n][m] = emit[m] * area[n];
    }
  }

  fread(emit+1, sizeof(R4), nSrf, vfin);
  fclose(vfin);

}  /* end of SaveF1t */

/***  ReadVF.c  **************************************************************/

/*  Read view factors file.  */

void ReadVF(I1 *fileName, I1 *program, I1 *version,
            IX *format, IX *encl, IX *didemit, IX *nSrf,
            R4 *area, R4 *emit, R8 **AF, R4 **F, IX init, IX shape)
{
  if(init) {
    I1 header[36];
    FILE *vfin = fopen(fileName, "r");
    fgets(header, 35, vfin);
    sscanf(header, "%s %s %d %d %d %d",
           program, version, format, encl, didemit, nSrf);
    fclose(vfin);
  } else {
    IX ns = *nSrf;
    if(*format == 0) {
      if(shape == 0) {
        ReadF0t(fileName, ns, area, emit, AF);
      } else {
        ReadF0s(fileName, ns, area, emit, F);
      }
    } else if(*format == 1) {
      if(shape == 0) {
        ReadF1t(fileName, ns, area, emit, AF);
      } else {
        ReadF1s(fileName, ns, area, emit, F);
      }
    }
    else {
      error(3, __FILE__, __LINE__, "Undefined format: %d", *format);
    }
  }

}  /* end ReadVF */

