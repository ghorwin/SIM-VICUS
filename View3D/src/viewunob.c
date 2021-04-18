/*subfile:  viewunob.c  *******************************************************/
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
/*  Compute unobstructed view factors  */

#include <stdio.h>
#include <math.h>  /* prototypes: atan, cos, fabs, log, sqrt */
#include "types.h"
#include "view3d.h"
#include "prtyp.h"

extern FILE *_ulog; /* log file */
extern I1 *methods[]; /* method abbreviations */
extern IX _maxNVT;  /* maximum number of temporary vertices */

#define PId2     1.570796326794896619   /* pi / 2 */
#define PIinv    0.318309886183790672   /* 1 / pi */
#define PIt4inv  0.079577471545947673   /* 1 / (4 * pi) */

/* The following variables are "global" to this file.  
 * They are allocated and freed in ViewsInit(). */
EDGEDCS *_rc1; /* edge DirCos of surface 1 */
EDGEDCS *_rc2; /* edge DirCos of surface 2 */
EDGEDIV **_dv1;  /* edge divisions of surface 1 */
EDGEDIV **_dv2;  /* edge divisions of surface 2 */
I4 _usedV1LIpart=0L;  /* number of calls to V1LIpart() */

/***  ViewUnobstructed.c  ****************************************************/

/*  Compute view factor (AF) -- no view obstructions.  */

R8 ViewUnobstructed(VFCTRL *vfCtrl, IX row, IX col)
{
  VERTEX3D pt1[16], pt2[16];
  R8 area1[16], area2[16];
  SRFDAT3X *srf1;  /* pointer to surface 1 */
  SRFDAT3X *srf2;  /* pointer to surface 2 */
  R8 AF0;  /* estimate of AF */
  R8 AF1;  /* improved estimate; one more edge division */
  IX nmax, mmax;
  IX nDiv;

#ifdef DEBUG
  fprintf(_ulog, " VU %.2e", vfCtrl->epsAF);
#endif

  srf1 = &vfCtrl->srf1T;
  srf2 = &vfCtrl->srf2T;
  if(vfCtrl->method < ALI) {
    AF1 = 2.0 * srf1->area;
  }
  if(vfCtrl->method == DAI) { /* double area integration */
#ifdef DEBUG
    fprintf(_ulog, " 2AI");
#endif
    for(nDiv=1; nDiv<5; nDiv++) {
      AF0 = AF1;
      nmax = SubSrf(nDiv, srf1->nv, srf1->v, srf1->area, pt1, area1);
      mmax = SubSrf(nDiv, srf2->nv, srf2->v, srf2->area, pt2, area2);
      AF1 = View2AI(nmax, &srf1->dc, pt1, area1, mmax, &srf2->dc, pt2, area2);
#ifdef DEBUG
      fprintf(_ulog, " %g", AF1);
#endif
      if(fabs(AF1 - AF0) < vfCtrl->epsAF) {
        goto done;
      }
    }
  }
  else if(vfCtrl->method == SAI) { /* single area integration */
#ifdef DEBUG
    fprintf(_ulog, " 1AI");
#endif
    for(nDiv=1; nDiv<5; nDiv++) {
      AF0 = AF1;
      nmax = SubSrf(nDiv, srf1->nv, srf1->v, srf1->area, pt1, area1);
      AF1 = -View1AI(nmax, pt1, area1, &srf1->dc, srf2);
#ifdef DEBUG
      fprintf(_ulog, " %g", AF1);
#endif
      if(fabs(AF1 - AF0) < vfCtrl->epsAF) {
        goto done;
      }
    }
  } else if(vfCtrl->method == SLI) { /* single line integration */
#ifdef DEBUG
    fprintf(_ulog, " 1LI");
#endif
    for(nDiv=1; nDiv<5; nDiv++) {
      AF0 = AF1;
      DivideEdges(nDiv, srf1->nv, srf1->v, _rc1, _dv1);
      AF1 = View1LI(nDiv, srf1->nv, _rc1, _dv1, srf1->v, srf2->nv, srf2->v);
#ifdef DEBUG
      fprintf(_ulog, " %g", AF1);
#endif
      if(fabs(AF1 - AF0) < vfCtrl->epsAF) {
        goto done;
      }
    }
  } else if(vfCtrl->method == DLI) { /* double line integration */
#ifdef DEBUG
    fprintf(_ulog, " 2LI");
#endif
    for(nDiv=1; nDiv<5; nDiv++) {
      AF0 = AF1;
      DivideEdges(nDiv, srf1->nv, srf1->v, _rc1, _dv1);
      DivideEdges(nDiv, srf2->nv, srf2->v, _rc2, _dv2);
      AF1 = View2LI(nDiv, srf1->nv, _rc1, _dv1, nDiv, srf2->nv, _rc2, _dv2);
#ifdef DEBUG
      fprintf(_ulog, " %g", AF1);
#endif
      if(fabs(AF1 - AF0) < vfCtrl->epsAF) {
        goto done;
      }
    }
  }

  /* adaptive single line integration - method==ALI or simpler methods fail */
#ifdef DEBUG
  if(vfCtrl->method < ALI) {
    fprintf(_ulog, " Fixing view factor\n");
  }
  fprintf(_ulog, " ALI");
  //#endif
  //#if( DEBUG == 1 )
  if(vfCtrl->method < ALI) {
    fprintf(_ulog, " row %d, col %d,  Fix %s (r %.2f, s %.2f) AF0 %g Af1 %g\n",
            row, col, methods[vfCtrl->method],vfCtrl->rcRatio, vfCtrl->relSep,
        AF0, AF1);
  }
#endif

  AF1 = ViewALI(srf1->nv, srf1->v, srf2->nv, srf2->v, vfCtrl);

#ifdef DEBUG
  if(vfCtrl->method < ALI) {
    fprintf(_ulog, "AF %g\n", AF1);
  }
#endif
  if(vfCtrl->method == ALI) { /* adaptive line integration */
    nDiv = 2;                 /* for bins[][] report */
  }
#ifdef DEBUG
  fprintf(_ulog, " %g", AF1);
#endif

done:
#ifdef DEBUG
  fprintf(_ulog, "\n");
#endif
  vfCtrl->nEdgeDiv = nDiv;

  return AF1;

}  /* end ViewUnobstructed */

/***  View2AI.c  *************************************************************/

/*  Compute direct interchange area by double area integration.
 *  Surfaces described by their direction cosines and NSS1|2 vertices
 *  and associated areas for numberical integration.  */

R8 View2AI(const IX nss1, const DIRCOS *dc1, const VERTEX3D *pt1, const R8 *area1,
           const IX nss2, const DIRCOS *dc2, const VERTEX3D *pt2, const R8 *area2)
{
  VECTOR3D V;
  R8 r2, sumt, sum=0.0;
  IX i, j;

  for(i=0; i<nss1; i++) {
    for(j=0; j<nss2; j++) {
      VECTOR((pt1+i), (pt2+j), (&V));
      sumt = VDOT((&V), dc1) * VDOT((&V), dc2);
      r2 = VDOT((&V), (&V));
      sumt *= area1[i] * area2[j] / (r2 * r2);
      sum -= sumt;
    }
  }

  sum *= PIinv;             /* divide by pi */

  return sum;

}  /* end View2AI */

/*  View1AI() is in viewobs.c  */

/***  View2LI.c  *************************************************************/

/*  Compute direct interchange area by double line integration. 
 *  Both surfaces described by directions cosines of edges and
 *  subdivisions of those edges for numerical integration.  */
/* const removal: const EDGEDIV **dv1, const EDGEDIV **dv2 */
R8 View2LI(const IX nd1, const IX nv1, const EDGEDCS *rc1, EDGEDIV **dv1,
           const IX nd2, const IX nv2, const EDGEDCS *rc2, EDGEDIV **dv2)
/* nd1 - number of edge divisions of polygon 1.
 * nv1 - number of vertices/edges of polygon 1.
 * rc1 - vector of direction cosines of edges of polygon 1.
 * dv1 - array of edge divisions of polygon 1.
 * nd2 - number of edge divisions of polygon 2.
 * nv2 - number of vertices/edges of polygon 2.
 * rc2 - vector of direction cosines of edges of polygon 2.
 * dv2 - array of edge divisions of polygon 2.
 */
{
  VECTOR3D R;    /* vector between edge elements */
  R8 r2;         /* square of distance between edge elements */
  R8 dot;        /* dot product of edge direction cosines */
  R8 sum, sumt;  /* R8 because of large +/- operations */
  IX i;   /* surface 1 edge index */
  IX j;   /* surface 2 edge index */
  IX n;   /* surface 1 edge element index */
  IX m;   /* surface 2 edge element index */

  for(sum=0.0,i=0; i<nv1; i++) {
    for(j=0; j<nv2; j++) {
      dot = VDOT((rc1+i), (rc2+j));
      if(fabs(dot) < EPS2) {
        continue;
      }
      for(sumt=0.0,n=0; n<nd1; n++) {
        for(m=0; m<nd2; m++) {
          VECTOR((dv1[i]+n), (dv2[j]+m), (&R));
          r2 = VDOT((&R), (&R));
#ifdef DEBUG
          if(r2 < EPS) {
            error(2, __FILE__, __LINE__, "log(r2)=%6g in View2LI", r2);
          }
#endif
          sumt += dv1[i][n].s * dv2[j][m].s * log(r2);
        }  /* end m & n loops */
      }
      sum += dot * sumt;
    }  /* end j loop */
  }  /* end i loop */

  sum *= PIt4inv;          /* divide by 4*pi */

  return sum;

}  /* end of View2LI */

/***  View1LI.c  *************************************************************/

/*  Compute direct interchange area by single line integral method.
 *  Surface 1 described by directions cosines of edges and
 *  subdivisions of those edges for numerical integration.
 *  Surface 2 described by its vertices.  */
/* const removal: const EDGEDIV **dv1 */
R8 View1LI(const IX nd1, const IX nv1, const EDGEDCS *rc1, EDGEDIV **dv1,
           const VERTEX3D *v1, const IX nv2, const VERTEX3D *v2)
/* nd1 - number of edge divisions of polygon 1.
 * nv1 - number of vertices/edges of polygon 1.
 * rc1 - vector of direction cosines of edges of polygon 1.
 * dv1 - array of edge divisions of polygon 1.
 * v1  - vector of vertices of polygon 1.
 * nv2 - number of vertices/edges of polygon 2.
 * v2  - vector of vertices of polygon 2.
 */
{
  R8 sum, sumt;  /* double because of large +/- operations */
  IX i, im1;  /* surface 1 edge index */
  IX j, jm1;  /* surface 2 edge index */
  IX n;       /* surface 1 edge element index */

  /* for all edges of polygon 2 */
  jm1 = nv2 - 1;
  for(sum=0.0,j=0; j<nv2; jm1=j++) {
    VECTOR3D B;   /* edge of polygon 2 */
    R8 b2;        /* length of edge squared */
    R8 b, binv;   /* length of edge and its inverse */

    VECTOR((v2+jm1), (v2+j), (&B));
    b2 = VDOT((&B), (&B));
#ifdef DEBUG
    if(b2 < EPS2) {
      error(2, __FILE__, __LINE__, "small b2=%6g in View1LI", b2);
    }
#endif
    b = sqrt(b2);
    binv = 1.0 / b;    /* b > 0.0 */
    /* for all edges of polygon 1 */
    im1 = nv1 - 1;
    for(i=0; i<nv1; im1=i++) {
      IX parallel=1;  /* true if edges i and j are parallel */
      R8 dot = VDOT((&B), (rc1+i)) * binv;
      if(fabs(dot) <= EPS * b) continue;
      if(fabs(dot) < 1.0-EPS) {
        parallel = 0;
      }
      for(sumt=0.0,n=0; n<nd1; n++) {  /* numeric integration */
        IX close;
        sumt += V1LIpart((void*)(dv1[i]+n), v2+jm1, v2+j, &B, b2, &close)
            * dv1[i][n].s;
        if(parallel && close) {
          break;   /* colinear edges */
        }
      }  /* end numeric integration */

      if(n==nd1) {
        sum += dot * sumt * binv;
      } else {             /* colinear edges ==> analytic solution */
        sumt = V1LIxact(v1+im1, v1+i, rc1[i].s, v2+jm1, v2+j, b);
        sum += dot * sumt;
      }
    }  /* end i loop */
  }  /* end j loop */

  sum *= PIt4inv;          /* divide by 4*pi */

  return sum;

}  /* end of View1LI */

/***  V1LIpart.c  ************************************************************/

/*  Compute Mitalas & Stephenson dF value for a single point, PP,
 *  to an edge from B0 to B1.  Part of single line integral method.  */

R8 V1LIpart(const VERTEX3D *pp, const VERTEX3D *b0, const VERTEX3D *b1,
            const VECTOR3D *B, const R8 b2, IX *flag)
/* pp     pointer to vertex on polygon 1;
 * b0     pointer to start of edge on polygon 2;
 * b1     pointer to end of edge on polygon 2;
 * B      vector from p0 to p1 (edge of polygon 2);
 * b2     length^2 of vector B;
 * *flag; return 1 if g=0; else return 0. */
{
  VECTOR3D S, T, SxB;
  R8 s2, t2, sxb2;
  R8 sum=0.0;

  _usedV1LIpart += 1;  /* number of calls to V1LIpart() */
  VECTOR(b0, pp, (&S));
  s2 = VDOT((&S), (&S));
  if(s2 > EPS2) {
    sum += VDOT((&S), B) * log(s2);
  }

  VECTOR(pp, b1, (&T));
  t2 = VDOT((&T), (&T));
  if(t2 > EPS2) {
    sum += VDOT((&T), B) * log(t2);
  }

  VCROSS((&S), B, (&SxB));
  sxb2 = VDOT((&SxB), (&SxB));
  if(sxb2 > EPS2*b2) {
    R8 h = s2 + t2 - b2;
    R8 g = sqrt(sxb2);
    if(g > EPS2) {
      R8 omega = PId2 - atan(0.5 * h / g);
      sum += 2.0 * (g * omega - b2);
    } else {
      /* Retained for historical reasons, DO NOT REMOVE. */
      error(3, __FILE__, __LINE__, "View1LI failed, call George");
    }
    *flag = 0;
  } else {
    sum -= 2.0 * b2;
    *flag = 1;
  }

  return sum;

}  /* end V1LIpart */

/***  V1LIadapt.c  ***********************************************************/

/*  Compute line integral by adaptive Simpson integration.
 *  This is a recursive calculation!  */

R8 V1LIadapt(VERTEX3D Pold[3], R8 dFold[3], R8 h, const VERTEX3D *b0,
const VERTEX3D *b1, const VECTOR3D *B, const R8 b2, IX level, VFCTRL *vfCtrl)
/* Pold   3 vertices on edge of polygon 1;
 * dFold  corresponding dF values;
 * h      |Pold[2] - Pold[0]| / 6.0;
 * b0     pointer to start of edge on polygon 2;
 * b1     pointer to end of edge on polygon 2;
 * B      vector from p0 to p1 (edge of polygon 2);
 * b2     length^2 of vector B;
 * *flag; return 1 if g=0; else return 0. */
{
  VERTEX3D P[5];
  R8 dF[5];
  R8 F3,  /* F using 3-point Simpson integration */
      F5;  /* F using 5-point Simpson integration */
  IX flag, j;

  for(j=0; j<3; j++) {
    VCOPY((Pold+j), (P+j+j));
    dF[j+j] = dFold[j];
  }
  F3 = h * (dF[0] + 4.0*dF[2] + dF[4]);

  vfCtrl->usedV1LIadapt += 2;
  VMID((P+0), (P+2), (P+1));
  dF[1] = V1LIpart(P+1, b0, b1, B, b2, &flag);

  VMID((P+2), (P+4), (P+3));
  dF[3] = V1LIpart(P+3, b0, b1, B, b2, &flag);
  h *= 0.5;
  F5 = h * (dF[0] + 4.0*dF[1] + 2.0*dF[2] + 4.0*dF[3] + dF[4]);

  if(fabs(F5 - F3) > vfCtrl->epsAF) {     /* test convergence */
    if(++level > vfCtrl->maxRecursALI) {  /* limit maximum recursions */
      vfCtrl->failViewALI = 1;
    } else {             /* one more level of adaptive integration */
      F5 = V1LIadapt(P+0, dF+0, h, b0, b1, B, b2, level, vfCtrl)
          + V1LIadapt(P+2, dF+2, h, b0, b1, B, b2, level, vfCtrl);
    }
  }

  return F5;

}  /* end V1LIadapt */

/***  V1LIxact.c  ************************************************************/

/*  Analytic integration of colinear edges. */

R8 V1LIxact(const VERTEX3D *a0, const VERTEX3D *a1, const R8 a,
            const VERTEX3D *b0, const VERTEX3D *b1, const R8 b)
/* a0 - point for start of edge of polygon 1.
 * a1 - point for end of edge of polygon 1.
 * a  - length of edge of polygon 1.
 * b0 - point for start of edge of polygon 2.
 * b1 - point for end of edge of polygon 2.
 * b -  length of edge of polygon 2.
 */
{
  VECTOR3D V;   /* temporary vector */
  R8 e2, d2;
  R8 sum=0.0;

  VECTOR(b0, a1, (&V));
  e2 = VDOT((&V), (&V));

  VECTOR(b1, a0, (&V));
  d2 = VDOT((&V), (&V));

  if(e2 < EPS2 && d2 < EPS2) { /* identical edges */
    sum = b * b * (log(b * b) - 3.0);
  } else {                     /* non-identical edges */
    R8 c2, f2;
    if(e2 > EPS2) {
      sum += e2 - e2 * log(e2);
    }
    if(d2 > EPS2) {
      sum += d2 - d2 * log(d2);
    }
    
    VECTOR(b0, a0, (&V));
    c2 = VDOT((&V), (&V));
    if(c2 > EPS2) {
      sum += c2 * log(c2) - c2;
    }
    
    VECTOR(b1, a1, (&V));
    f2 = VDOT((&V), (&V));
    if(f2 > EPS2) {
      sum += f2 * log(f2) - f2;
    }
    sum = 0.5 * sum - 2.0 * a * b;
  }

  return sum;

}  /* end V1LIxact */

/***  ViewALI.c  *************************************************************/

/*  Compute direct interchange area by adaptive single line integral 
 *  (Mitalas-Stephensen) method */

R8 ViewALI(const IX nv1, const VERTEX3D *v1,
           const IX nv2, const VERTEX3D *v2, VFCTRL *vfCtrl)
/* nv1 - number of vertices/edges of polygon 1.
 * v1  - vector of vertices of polygon 1.
 * nv2 - number of vertices/edges of polygon 2.
 * v2  - vector of vertices of polygon 2.
 */
{
  VECTOR3D A[MAXNV1]; /* edges of polygon 1 */
  R8 a[MAXNV1]; /* lengths of polygon 1 edges */
  R8 sum, sumt; /* double because of large +/- operations */
  IX i, im1;    /* surface 1 edge index */
  IX j, jm1;    /* surface 2 edge index */

#ifdef DEBUG
  if(nv1>MAXNV1) {
    error(2, __FILE__, __LINE__, "MAXNV1 too small in ViewALI");
  }
  //#endif
  //#if( DEBUG > 1 )
  fprintf(_ulog, "Begin ViewALI():\n");
#endif
  im1 = nv1 - 1;
  for(i=0; i<nv1; im1=i++) {   /* for all edges of polygon 1 */
    VECTOR((v1+im1), (v1+i), (&A[i]));
    a[i] = VLEN((&A[i]));
#ifdef DEBUG
    if(a[i] < EPS) {
      error(2, __FILE__, __LINE__, "small edge (a=%6g) in ViewALI",a[i]);
    }
#endif
  }

  jm1 = nv2 - 1;
  for(sum=0.0,j=0; j<nv2; jm1=j++) { /* for all edges of polygon 2 */
    VECTOR3D B;  /* edge of polygon 2 */
    R8 b, b2;    /* length  and length^2 of edge */
    R8 dot;      /* dot product of edges i and j */

    VECTOR((v2+jm1), (v2+j), (&B));
    b2 = VDOT((&B), (&B));
    b = sqrt(b2);
#ifdef DEBUG
    if(b < EPS) {
      error(2, __FILE__, __LINE__, "small edge (b=%6g) in ViewALI", b);
    }
#endif

    im1 = nv1 - 1;
    for(i=0; i<nv1; im1=i++) {    /* for all edges of polygon 1 */
      VERTEX3D V[3];  /* vertices of edge i */
      R8 dF[3];
      IX flag1, flag2;
      //#ifdef DEBUG
      //      fprintf(_ulog, "Srf1 %d-%d (%f %f %f) to (%f %f %f)\n", i, ip1,
      //        v1[i].x, v1[i].y, v1[i].z, v1[ip1].x, v1[ip1].y, v1[ip1].z);
      //      fprintf(_ulog, "Srf2 %d-%d (%f %f %f) to (%f %f %f)\n", j, jp1,
      //        v2[j].x, v2[j].y, v2[j].z, v2[jp1].x, v2[jp1].y, v2[jp1].z);
      //#endif
      dot = VDOT((&B), (A+i)) / (b * a[i]);
      if(fabs(dot) <= EPS) {
        continue;
      }
#ifdef DEBUG
      fprintf(_ulog, " ViewALI: j=%d i=%d b %f a %f dot %f\n",
              j, i, b, a[i], dot);
#endif
      VCOPY((v1+im1), (V+0));
      dF[0] = V1LIpart(V+0, v2+jm1, v2+j, &B, b2, &flag1);

      VCOPY((v1+i), (V+2));
      dF[2] = V1LIpart(V+2, v2+jm1, v2+j, &B, b2, &flag2);
      vfCtrl->usedV1LIadapt += 2;

      if(flag1 + flag2 == 2) {   /* analytic integration */
        sumt = V1LIxact(v1+im1, v1+i, a[i], v2+jm1, v2+j, b);
      } else {                   /* adaptive integration */
        VMID((V+0), (V+2), (V+1));
        dF[1] = V1LIpart(V+1, v2+jm1, v2+j, &B, b2, &flag1);
        vfCtrl->usedV1LIadapt += 1;
        sumt = V1LIadapt(V, dF, a[i]/6.0, v2+jm1, v2+j,
                         &B, b2, 0, vfCtrl) / b;
      }
      sum += dot * sumt;
#ifdef DEBUG
      fprintf(_ulog, "ALI: i %d j %d dot %f t %f sum %f\n",
              j, i, dot, sumt, sum);
#endif
    }  /* end i loop */
  }  /* end j loop */

  sum *= PIt4inv;          /* divide by 4*pi */
#ifdef DEBUG
  fprintf(_ulog, "ViewALI AF: %g\n", sum);
#endif

  return sum;

}  /* end of ViewALI */

/***  ViewsInit.c  ***********************************************************/

/*  Allocate / free arrays local to this file based on INIT.
 *  Initialize Gaussian integration coefficients.
 *  Store G coefficients in vectors emulating triangular arrays.  */

void ViewsInit(IX maxDiv, IX init)
{
  static IX maxRC1;    /* max number of values in RC1 */
  static IX maxRC2;    /* max number of values in RC2 */
  static IX maxDV1;    /* max number of values in DV1 */
  static IX maxDV2;    /* max number of values in DV2 */

  if(init) {
    maxRC1 = MAXNV1;
    _rc1 = Alc_V(0, maxRC1, sizeof(EDGEDCS), __FILE__, __LINE__);
    maxDV1 = maxDiv - 1;
    _dv1 = Alc_MC(0, maxRC1, 0, maxDV1, sizeof(EDGEDIV), __FILE__, __LINE__);
    maxRC2 = _maxNVT;  // MAXNVT @@@ needs work; 2005/11/02;
    _rc2 = Alc_V(0, maxRC2, sizeof(EDGEDCS), __FILE__, __LINE__);
    maxDV2 = maxDiv - 1;
    _dv2 = Alc_MC(0, maxRC2, 0, maxDV2, sizeof(EDGEDIV), __FILE__, __LINE__);
  } else {
    Fre_MC(_dv2, 0, maxRC2, 0, maxDV2, sizeof(EDGEDIV), __FILE__, __LINE__);
    Fre_V(_rc2, 0, maxRC2, sizeof(EDGEDCS), __FILE__, __LINE__);
    Fre_MC(_dv1, 0, maxRC1, 0, maxDV1, sizeof(EDGEDIV), __FILE__, __LINE__);
    Fre_V(_rc1, 0, maxRC1, sizeof(EDGEDCS), __FILE__, __LINE__);
    fprintf(_ulog, "Total line integral points evaluated:    %8lu\n",
            _usedV1LIpart);
  }

}  /* end ViewsInit */

const R8 _gqx[10] = {    /* Gaussian ordinates */
                         0.500000000, 0.211324865, 0.788675135, 0.112701665, 0.500000000,
                         0.887298335, 0.069431844, 0.330009478, 0.669990522, 0.930568156 };
const R8 _gqw[10] = {    /* Gaussian weights */
                         1.000000000, 0.500000000, 0.500000000, 0.277777778, 0.444444444,
                         0.277777778, 0.173927423, 0.326072577, 0.326072577, 0.173927423 };
const IX _offset[4] = { 0, 1, 3, 6 };

/***  DivideEdges.c  *********************************************************/

/*  Divide edges of a polygon for Gaussian quadrature, 1 <= nDiv <= 4.  */

IX DivideEdges(IX nDiv, IX nVrt, VERTEX3D *Vrt, EDGEDCS *rc, EDGEDIV **dv)
/* nDiv  - number of divisions per edge.
 * nVrt  - number of vertices/edges.
 * Vrt  - coordinates of vertices.
 * rc  - direction cosines of each edge.
 * dv  - edge divisions for Gaussian quadrature.
 */
{
  VECTOR3D V; /* vector betweeen successive vertices */
  R8 s;    /* distance between vertices */
  IX i, im1, j, n;

#ifdef DEBUG
  fprintf(_ulog, "DIVEDGE:  nd=%d\n", nDiv);
  DumpP3D("Polygon:", nVrt, Vrt);
  //#endif
  //#if( DEBUG > 0 )
  if(nDiv > 4) {
    error(2, __FILE__, __LINE__, "nDiv > 4 in DivideEdges");
    nDiv = 4;
  }
#endif

  n = _offset[nDiv-1];

  im1 = nVrt - 1;
  for(i=0; i<nVrt; im1=i++) { /* for all edges */
    VECTOR((Vrt+im1), (Vrt+i), (&V));
    rc[i].s = s = VLEN((&V));
    rc[i].x = V.x / s;
    rc[i].y = V.y / s;
    rc[i].z = V.z / s;
    for(j=0; j<nDiv; j++) { /* divide each edge */
      dv[i][j].x = Vrt[im1].x + _gqx[n+j] * V.x;
      dv[i][j].y = Vrt[im1].y + _gqx[n+j] * V.y;
      dv[i][j].z = Vrt[im1].z + _gqx[n+j] * V.z;
      dv[i][j].s = _gqw[n+j] * s;
    }
  }

#ifdef DEBUG
  fprintf(_ulog, "rc:       x            y            z            s\n");
  for(i=0; i<nVrt; i++) {
    fprintf(_ulog, "%2d %12.5f %12.5f %12.5f %12.5f\n", i,
            rc[i].x, rc[i].y, rc[i].z, rc[i].s);
  }
  fprintf(_ulog, "dv:       x            y            z            s\n");
  for(i=0; i<nVrt; i++) {
    for(j=0; j<nDiv; j++) {
      fprintf(_ulog, "%2d %12.5f %12.5f %12.5f %12.5f\n", j,
              dv[i][j].x, dv[i][j].y, dv[i][j].z, dv[i][j].s);
    }
  }
  fflush(_ulog);
#endif
  return nDiv;

}  /* end of DivideEdges */

/***  GQParallelogram.c  *****************************************************/

/*  Compute Gaussian integration values for a parallelogram.  
 *
 *           v[3] *----------*----------*-----*----------* v[2]
 *               /   ...    /   ...    / ... /  p[nn-1] /
 *              *----------*----------*-----*----------*
 *             /   ...    /   ...    / ... /   ...    /
 *            *----------*----------*-----*----------*
 *           /   p[n]   /  p[n+1]  / ... /   ...    /
 *          *----------*----------*-----*----------*
 *         /   p[0]   /   p[1]   / ... /  p[n-1]  /
 *   v[0] *----------*----------*-----*----------* v[1]
 */

IX GQParallelogram(const IX nDiv, const VERTEX3D *vp, VERTEX3D *p, R8 *w)
/* nDiv - division factor
 * vp   - vertices of parallelogram
 * p    - coordinates of Gaussian points
 * w    - Gaussian weights */
{
  VECTOR3D v0;  /* vector from v[0] to v[3] */
  VECTOR3D v1;  /* vector from v[1] to v[2] */
  VECTOR3D v2;  /* vector from pt0 to pt1 */
  VERTEX3D pt0, pt1; /* Gaussian points on v0 and v1 */
  static const IX nss[4] = { 1, 4, 9, 16 };
  IX nSubSrf;   /* number of subsurfaces */
  IX i, j, n;

  n = _offset[nDiv-1];
  nSubSrf = nss[nDiv-1];
  VECTOR((vp+0), (vp+3), (&v0));
  VECTOR((vp+1), (vp+2), (&v1));

  for(i=0; i<nDiv; i++) {
    pt0.x = vp[0].x + v0.x * _gqx[n+i];
    pt0.y = vp[0].y + v0.y * _gqx[n+i];
    pt0.z = vp[0].z + v0.z * _gqx[n+i];
    pt1.x = vp[1].x + v1.x * _gqx[n+i];
    pt1.y = vp[1].y + v1.y * _gqx[n+i];
    pt1.z = vp[1].z + v1.z * _gqx[n+i];
    VECTOR((&pt0), (&pt1), (&v2));
    for(j=0; j<nDiv; j++,p++,w++) {
      p->x = pt0.x + v2.x * _gqx[n+j];
      p->y = pt0.y + v2.y * _gqx[n+j];
      p->z = pt0.z + v2.z * _gqx[n+j];
      *w = _gqw[n+i] * _gqw[n+j];     /* not correct for a */
    }                               /* general quadrilateral */
  }

  return nSubSrf;

}  /* end GQParallelogram */

/***  SubSrf.c  **************************************************************/

/*  Set Gaussian integration values for triangle or rectangle.  */

IX SubSrf(const IX nDiv, const IX nv, const VERTEX3D *Sv, const R8 area,
          VERTEX3D *Gpt, R8 *wt)
/* nDiv - division factor
 * nv   - number of vertices, 3 or 4
 * Sv   - coordinates of vertices
 * area - of triangle or rectangle
 * Gpt  - coordinates of Gaussian points
 * wt   - Gaussian weights */
{
  IX nSubSrf;       /* number of subsurfaces */
  IX n;

  if(nv == 3) {
    nSubSrf = GQTriangle(nDiv, Sv, Gpt, wt);
  } else {
    nSubSrf = GQParallelogram(nDiv, Sv, Gpt, wt);
  }

  for(n=0; n<nSubSrf; n++) {
    wt[n] *= area;
  }

#ifdef XXX
  fprintf(_ulog, "SubSrf: polygon vertices\n");
  fprintf(_ulog, "   #     x         y         z\n");
  for(n=0; n<nv; n++)
    fprintf(_ulog, "%3d, %9.6f %9.6f %9.6f\n",
            n, Sv[n].x, Sv[n].y, Sv[n].z);
  fprintf(_ulog, " integration points\n");
  fprintf(_ulog, "   #     x         y         z         w\n");
  for(n=0; n<nSubSrf; n++)
    fprintf(_ulog, "%3d, %9.6f %9.6f %9.6f %9.6f\n",
            n, Gpt[n].x, Gpt[n].y, Gpt[n].z, wt[n]);
  error(3, __FILE__, __LINE__, "end test");
#endif

  return nSubSrf;

}  /* end SubSrf */

/***  GQTriangle.c  **********************************************************/

/*  Gaussian integration values for a triangle, 1 <= nDiv <= 4.
 *  Return the coordinates of Point N and its associated weighting.  */

IX GQTriangle(const IX nDiv, const VERTEX3D *vt, VERTEX3D *p, R8 *w)
{
  /* nDiv - division factor
 * vt   - vertices of triangle
 * p    - coordinates of Gaussian points
 * w    - Gaussian weights */
  static const IX offset[4] = { 0, 1, 5, 12 };
  /* Gaussian ordinates & weights */
  static const R8 gx[25][4] = {
    {0.33333333, 0.33333333, 0.33333333, 1.00000000 },  /* 1-point */

    {0.33333333, 0.33333333, 0.33333333, -0.5625000 },  /* 4-point */
    {0.60000000, 0.20000000, 0.20000000, 0.52083333 },
    {0.20000000, 0.60000000, 0.20000000, 0.52083333 },
    {0.20000000, 0.20000000, 0.60000000, 0.52083333 },

    {0.33333333, 0.33333333, 0.33333333, 0.22500000 },  /* 7-point */
    {0.05971587, 0.47014206, 0.47014206, 0.13239415 },
    {0.47014206, 0.05971587, 0.47014206, 0.13239415 },
    {0.47014206, 0.47014206, 0.05971587, 0.13239415 },
    {0.79742699, 0.10128651, 0.10128651, 0.12593918 },
    {0.10128651, 0.79742699, 0.10128651, 0.12593918 },
    {0.10128651, 0.10128651, 0.79742699, 0.12593918 },

    {0.33333333, 0.33333333, 0.33333333, -0.14957004 }, /* 13-point */
    {0.47930807, 0.26034597, 0.26034597, 0.17561526 },
    {0.26034597, 0.47930807, 0.26034597, 0.17561526 },
    {0.26034597, 0.26034597, 0.47930807, 0.17561526 },
    {0.86973979, 0.06513010, 0.06513010, 0.05334724 },
    {0.06513010, 0.86973979, 0.06513010, 0.05334724 },
    {0.06513010, 0.06513010, 0.86973979, 0.05334724 },
    {0.63844419, 0.31286550, 0.04869031, 0.07711376 },
    {0.63844419, 0.04869031, 0.31286550, 0.07711376 },
    {0.31286550, 0.63844419, 0.04869031, 0.07711376 },
    {0.31286550, 0.04869031, 0.63844419, 0.07711376 },
    {0.04869031, 0.63844419, 0.31286550, 0.07711376 },
    {0.04869031, 0.31286550, 0.63844419, 0.07711376 } };

  static const IX nss[4] = { 1, 4, 7, 13 };  /* number of subsurfaces */
  IX nSubSrf;       /* number of subsurfaces */
  IX j, n;

  n = offset[nDiv-1];
  nSubSrf = nss[nDiv-1];
  for(j=0; j<nSubSrf; j++,p++,w++) {
    p->x = gx[j+n][0] * vt[0].x + gx[j+n][1] * vt[1].x + gx[j+n][2] * vt[2].x;
    p->y = gx[j+n][0] * vt[0].y + gx[j+n][1] * vt[1].y + gx[j+n][2] * vt[2].y;
    p->z = gx[j+n][0] * vt[0].z + gx[j+n][1] * vt[1].z + gx[j+n][2] * vt[2].z;
    *w = gx[j+n][3];
  }

  return nSubSrf;

}  /* end GQTriangle */

