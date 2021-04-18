/*subfile:  viewpp.c  *********************************************************/
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

/*  Functions for post-processing of view factors. */

#include <stdio.h>
#include <string.h> /* prototype: memset, strncpy */
#include <math.h>   /* prototype: fabs */
#include <float.h>  /* define: FLT_EPSILON */
#include "types.h"
#include "view3d.h"
#include "prtyp.h" 

extern FILE *_ulog; /* log file */
extern IX _list;    /* output control, higher value = more output */

/***  DelNull.c  *************************************************************/

/*  Delete NULS surfaces from AF and supporting vectors.
//  Return reduced number of surfaces, nSrf.  */

IX DelNull(const IX nSrf, SRFDAT3D *srf, IX *base, IX *cmbn,
           R4 *emit, R4 *area, I1 **name, R8 **AF)
{
  IX j, n, m;

  for(j=0,n=1; n<=nSrf; n++) {
    if(srf[n].type == NULS) { /* adjust surface areas */
      area[base[n]] -= area[n];   /* requires subsurface after base */
    } else {
      IX i=0;
      base[++j] = base[n];
      cmbn[j] = cmbn[n];
      emit[j] = emit[n];
      area[j] = area[n];
      strcpy(name[j], name[n]);
      for(m=1; m<=n; m++) {  /* remove null surface AF values */
        if(srf[m].type != NULS) {
          AF[j][++i] = AF[n][m];
        }
      }
    }
  }

  return j;    /* J is the reduced number of surfaces */

}  /* end DelNull */

/***  Combine.c  *************************************************************/

/*  Combine surface interchange areas.
 *  Note: AF[row][col] with col <= row in lower triangle matrix.  */

IX Combine(const IX nSrf, const IX *cmbn, R4 *area, I1 **name, R8 **AF)
/*  nSrf; number of surfaces
 *  cmbn; combination surface numbers
 *  area; surface areas
 *  name; surface names
 *  AF;   radiation interchange factors
 */
{
  IX i, j, m, n;

  for(n=1; n<=nSrf; n++) { /* adjust AF and area */
    if(cmbn[n]==0) {
      continue;
    }
    i = cmbn[n];
    for(m=1; m<=nSrf; m++) {
      if(m > i) {
        if(m > n) {
          AF[m][i] += AF[m][n];
        } else {
          AF[m][i] += AF[n][m];
        }
      } else if(m < i) {
        if(m > n) {
          AF[i][m] += AF[m][n];
        } else {
          AF[i][m] += AF[n][m];
        }
      } else {
        if(m > n) {
          AF[i][m] += 2.0 * AF[m][n];
        } else {
          AF[i][m] += 2.0 * AF[n][m];
        }
      }
    }
    area[i] += area[n];
  }
  /* report new surface numbers */
  if(_list>0) {
    fprintf(_ulog, " New,   Old surface numbers\n");
    for(i=0,n=1; n<=nSrf; n++) {
      if(cmbn[n]) {
        continue;
      }
      fprintf(_ulog,"%4d: %3d", ++i, n);
      for(m=1; m<=nSrf; m++) {
        if(cmbn[m]==n) fprintf(_ulog, " %d", m);
      }
      fprintf(_ulog, "\n");
    }
  }
  /* reduce AF array, areas, names */
  for(i=0,n=1; n<=nSrf; n++) {
    if(cmbn[n]) {
      continue;
    }
    area[++i] = area[n];
    for(j=0, m=1; m<=n; m++) {
      if(cmbn[m] == 0) {
        AF[i][++j] = AF[n][m];
      }
    }
    strcpy(name[i], name[n]);
  }
  fprintf(_ulog, "Number of surfaces reduced to %d.\n", i);

  return i;

}  /* end of Combine */

/***  Separate.c  ************************************************************/

/*  Separate subsurfaces from base surfaces.  */

void Separate(const IX nSrf, const IX *base, R4 *area, R8 **AF)
/*  nSrf; number of surfaces
 *  base; base surface numbers
 *  area; surface areas
 *  AF;   radiation interchange factors (area*F)
 */
{
  IX k, m, n;

  for(n=1; n<=nSrf; n++) { /* Adjust AF values */
    for(k=n+1; k<=nSrf; k++) { /* Look ahead for subsurfaces of N */
      if(base[k] != n) {
        continue;
      }
      for(m=n+1; m<k; m++) {
        AF[m][n] -= AF[k][m];
      }
      for(m=k; m<=nSrf; m++) {
        AF[m][n] -= AF[m][k];
      }
    }

    for(m=n+1; m<=nSrf; m++) { /* Look ahead for subsurfaces of M */
      for(k=m+1; k<=nSrf; k++) {
        if(base[k] == m) {
          AF[m][n] -= AF[k][n];   /* k > m > n */
        }
      }
    }
  }  /* end outer loop */

  for(n=1; n<=nSrf; n++) { /* Adjust surface values */
    if(base[n]) {
      area[base[n]] -= area[n];   /* requires subsurface after base */
    }
  }

#ifdef DEBUG
  for(n=1; n<=nSrf; n++) {
    fprintf(_ulog, "Area: %d %f\n", n, area[n]);
  }
#endif

}  /* end of Separate */

/***  NormAF.c  **************************************************************/

/*    Normalize the view factors for an enclosure so that for each row i
 *    SUM(F[i,j]) = EMIT[i] and also AF(i,j) = AF(j,i) for all i, j.  */

void NormAF(const nSrf, const R4 *emit, const R4 *area, R8 **AF,
            const R8 eMax, const IX itMax)
/*  nSrf; number of surfaces
 *  emit; surface emittances
 *  area; surface areas
 *  AF;   radiation interchange factors / lower triangle
 *  eMax; maximum error permitted in sumF
 *  itMax; maximum number of iterations
 */
{
  IX n;    /* row */
  IX m;    /* column */
  IX iter; /* iterations count */
  R8 err;  /* row error value */
  R8 maxError=1.0;   /* max error value */
  R8 sumAF, sumF;

  for(iter=0; iter<itMax && maxError>eMax; iter++) {
    for(maxError=0.0,m=1; m<=nSrf; m++) {
      for(sumAF=0.0,n=1; n<=m; n++) {
        sumAF += AF[m][n];
      }
      for(n=m+1; n<=nSrf; n++) {
        sumAF += AF[n][m];
      }
      sumF = sumAF / area[m];
      err = fabs(sumF - emit[m]);
      if(err > maxError) {
        maxError = err;
      }
      sumF = emit[m] / sumF;
      for(n=1; n<=m; n++) {
        AF[m][n] *= sumF;
      }
      for(n=m+1; n<=nSrf; n++) {
        AF[n][m] *= sumF;
      }
    }
    if(_list>1) {
      fprintf(_ulog, "NormAF: %d  maxError: %.2e\n", iter+1, maxError);
    }
  }

  if(iter>=itMax) {
    error(2, __FILE__, __LINE__, "Too many iterations for normalization");
  }
  fprintf(_ulog, "%d normalization iterations.\n", iter);

}  /* end of NormAF */

/*  IntFac.c  ****************************************************************/

/*  Compute the total radiation interchange factors; i.e., include the
 *     effect of surface diffuse reflectance.
 *  Ref: H.C. Hottel & A.F. Sarofim, "Radiative Transfer", McGraw-Hill, 1967,
 *  pp 84-86. Requires solution of [A]*{W} = {B}.
 *  [A] is symmetric, negative-definite.  */

void IntFac(const IX nSrf, const R4 *emit, const R4 *area, R8 **AF)
/*  nSrf; number of surfaces
 *  emit; surface emittances, 0 < emit < 1
 *  area; surface areas
 *  AF;   radiation interchange factors [triangular]
 */
{
  IX  m, n;
  R8 *a, *w, *W;

  a = Alc_V(1, nSrf, sizeof(R8), __FILE__, __LINE__);
  W = Alc_V(0, nSrf*nSrf, sizeof(R8), __FILE__, __LINE__);

  /* subtract AREA/RHO from diagonal elements */
  for(n=1; n<=nSrf; n++) {
    a[n] = area[n] / (1.0 - emit[n]);
    AF[n][n] -= a[n];
    a[n] *= emit[n];
  }

  LUFactorSymm(nSrf, AF);  /* factor AF (symmetric) */
  
  w = W-1; /* This will get the indexing to work */
  for(n=1; n<=nSrf; n++) { /* determine response vectors */
    w[n] = -a[n];
    LUSolveSymm(nSrf, AF, w);
    w += nSrf;
  }
  
  w = W-1; /* This will get the indexing to work */
  /* Compute total interchange areas */
  for(n=1; n<=nSrf; n++) {
    for(m=1; m<n; m++) {
      AF[n][m] = a[m] * w[m];
    }
    AF[n][n] = a[n] * (w[n] - emit[n]);
    for(m=n+1; m<=nSrf; m++) {
      AF[m][n] = 0.5 * (AF[m][n] + a[m] * w[m]);
    }
    w += nSrf;
  }

  Fre_V(a, 1, nSrf, sizeof(R8), __FILE__, __LINE__);
  Fre_V(W, 0, nSrf*nSrf, sizeof(R8), __FILE__, __LINE__);

}  /* end of IntFac */

/*  LUFactorSymm.c  **********************************************************/

/*  L-U factorization of symmetric matrix [A] which is used for
 *  solution of simultaneous equations [A] * X = B.
 *  Only the lower triangle of [A] (including diagonal) is used.
 *  Decomposition of this form of matrix is supposed to be very stable
 *  without pivoting: _Numerical Recipes in C_, 2nd ed., p 97.  */

void LUFactorSymm(const IX neq, R8 **a)
{
  IX i, j;
  R8 dot, tmp;

  a[1][1] = 1.0 / a[1][1];
  for(i=2; i<=neq; i++) { /* process column i */
    for(j=2; j<i; j++) {
      a[i][j] -= DotProd(j-1, a[i], a[j]);
    }
    /* process diagonal i */
    for(dot=0.0, j=1; j<i; j++) {
      tmp = a[i][j] * a[j][j];
      dot += a[i][j] * tmp;
      a[i][j] = tmp;
    }
    a[i][i] -= dot;
    if(a[i][i] == 0.0) {
      error(3, __FILE__, __LINE__, "Zero on the diagonal, row %d", i);
    }
    a[i][i] = 1.0 / a[i][i];
  }  /*  end of i loop  */

}   /*  end of LUFactorSymm  */

/***  DotProd.c  *************************************************************/

/*  Double precision dot product.  dot = SUM(x[1]*y[1] ... x[n]*y[n]).
 *  Passing pointers to x[0] and y[0].  */

R8 DotProd(const IX n, const R8 *x, const R8 *y)
{
  IX j;
  R8 dot=0.0;

  for(j=n; j; j--) {
    dot += x[j] * y[j];
  }

  return dot;

}  /* end DotProd */

/*  LUSolveSymm.c  ***********************************************************/

/*  Solution of simultaneous equations [A] * {X} = {B}, where [A] (which is
 *  stored by rows) has already been reduced to L-U form in LUFactorSymm().
 *  The solution vector {X} over-writes {B}.  */
/* const removal: const R8 **a */
void LUSolveSymm(const IX neq, R8 **a, R8 *b)
{
  IX i;

  for(i=2; i<=neq; i++) {    /*  forward substitution  */
    b[i] -= DotProd(i-1, a[i], b);
  }

  for(i=neq; i; i--) {
    b[i] *= a[i][i];
  }

  for(i=neq; i>1; i--) {     /*  back substitution  */
    DAXpY(i-1, -b[i], a[i], b);
  }

}  /*  end of LUSolveSymm  */

/***  DAXpY.c  ***************************************************************/

/*  Double precision product:  Yi = Yi + A * Xi  for i = 1 to N.
 *  Passing pointers to x[0] and y[0].  */

void DAXpY(const IX n, const R8 a, const R8 *x, R8 *y)
{
  IX j;

  for(j=n; j; j--) {
    y[j] += a * x[j];
  }

}  /* end DAXpY */

