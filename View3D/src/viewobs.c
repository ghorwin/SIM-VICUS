/*subfile:  viewobs.c  ********************************************************/
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
/*  Compute obstructed view factors  */

#include <stdio.h>
#include <string.h> /* prototype: memcpy */
#include <math.h>   /* prototype: fabs */
#include "types.h"
#include "view3d.h"
#include "prtyp.h"

/* local functions */
void SubsrfRS(IX n, VERTEX3D v[], VERTEX3D s[]);
void SubsrfTS(IX n, VERTEX3D v[], VERTEX3D s[]);

extern FILE *_ulog; /* written output file */

#define PId2     1.570796326794896619   /* pi / 2 */
#define PIt2inv  0.159154943091895346   /* 1 / (2 * pi) */

/***  ViewObstructed.c  ******************************************************/

/*  Compute view factor (AF), with view obstructions
 *  by computing views to unshaded polygons.  */

R8 ViewObstructed(VFCTRL *vfCtrl, IX nv1, VERTEX3D v1[], R8 area, IX nDiv)
/* nv1  - number of vertices of surface 1.
 * v1   - vertices of surface 1.
 * area - area of surface 1.
 * nDiv - division factor, 3 or 4. */
{
  POLY *pp;     /* pointer to a polygon */
  POLY *shade;  /* pointer to the obstruction shadow polygon */
  POLY *stack;  /* pointer to stack of unobstructed polygons */
  POLY *next;   /* pointer to next unobstructed polygons */
  R8 dF;    /* F from a view point to an unshaded area */
  R8 dFv;   /* F from a view point to all unshaded areas */
  R8 AFu;   /* AF from all view points to all unshaded areas */
  VERTEX3D v2[MAXNV2]; /* 3D vertices: obstruction */
  VERTEX3D *pv2; /* clipped obstruction */
  VERTEX2D vs[MAXNV2], vb[MAXNV1]; /* 2D vertices: shadow and base surface (2) */
  SRFDAT3X *srfT;  /* pointer to surface */
  DIRCOS *dc1;  /* pointer to direction cosines of surface 1 */
  R8 xmin, xmax, ymin, ymax; /* clipping limits */
  IX clip; /* if true, clip to prevent upward projection */
  R8 epsDist, epsArea;
  R8 hc, zc[MAXNV1];  /* surface clipping test values */
  VERTEX3D vpt[16];  /* vertices of view points */
  R8 weight[16];     /* integration weighting factors */
  IX nvpt; /* number of view points */
  IX np;   /* view point number */
  IX nv2, nvs, nvb;
  IX j, k, n;

#ifdef DEBUG
  fprintf(_ulog, "ViewObstructed:\n");
#endif
  //#if( DEBUG > 2 )
  //  for( j=0; j<nv1; j++)
  //    if( v1[j].z <=0) ;
  //#endif

  dc1 = &vfCtrl->srf1T.dc;
  srfT = &vfCtrl->srf2T;
  nvb = srfT->nv;
  for(n=0; n<nvb; n++) {       /* reverse polygon 2 to clockwise */
    vb[n].x = srfT->v[nvb-1-n].x;
    vb[n].y = srfT->v[nvb-1-n].y;
  }
  xmin = xmax = vb[0].x;
  ymin = ymax = vb[0].y;
  for(n=1; n<nvb; n++) {       /* determine polygon 2 to limits */
    if(vb[n].x < xmin) {
      xmin = vb[n].x;
    }
    if(vb[n].x > xmax) {
      xmax = vb[n].x;
    }
    if(vb[n].y < ymin) {
      ymin = vb[n].y;
    }
    if(vb[n].y > ymax) {
      ymax = vb[n].y;
    }
  }
#ifdef DEBUG
  DumpP2D("Base Surface:", nvb, vb);
  fprintf(_ulog, "limits:  %f %f   %f %f\n", xmin, xmax, ymin, ymax);
#endif
  epsDist = 1.0e-6 * sqrt((xmax-xmin)*(xmax-xmin) + (ymax-ymin)*(ymax-ymin));
  epsArea = 1.0e-6 * srfT->area;

  /* determine Gaussian weights and view points of polygon 1 */
  nvpt = SubSrf(nDiv, nv1, v1, area, vpt, weight);

  /* compute obstructed view from each view point of polygon 1 */
  for(AFu=0.0,np=0; np<nvpt; np++) {       /* begin view points loop */
    hc = 0.9999f * vpt[np].z;
#ifdef DEBUG
    fprintf(_ulog, "view point: %f %f %f\n", vpt[np].x, vpt[np].y, vpt[np].z);
    fprintf(_ulog, "Hclip %g\n", hc);
    fflush(_ulog);
#endif
    /* begin with cleared small structures area - memBlock */
    InitPolygonMem(epsDist, epsArea);
    stack = SetPolygonHC(nvb, vb, 1.0);  /* convert surface 2 to HC */
#ifdef DEBUG
    DumpHC("BASE SURFACE:", stack, NULL);
#endif

    /* project shadow of each view obstructing surface */
    srfT = vfCtrl->srfOT;
    for(dFv=0.0,j=0; j<vfCtrl->nProbObstr; j++,srfT++) {
      /* CTD must be behind surface */
      R8 dot = VDOTW ((vpt+np), (&srfT->dc));
#ifdef DEBUG
      fprintf(_ulog, "Surface %d;  dot %f\n", srfT->nr, dot);
      fflush(_ulog);
#endif
      if(dot >= 0.0) {
        continue;      /* no shadow polygon created */
      }
      nvs = srfT->nv;
      for(clip=n=0; n<nvs; n++) {
        zc[n] = srfT->v[n].z - hc;
        if(zc[n] > 0.0) clip = 1;
      }
      if(clip) {       /* clip to prevent upward projection */
#ifdef DEBUG
        fprintf(_ulog, "Clip M;  zc: %g %g %g %g\n",
                zc[0], zc[1], zc[2], zc[3]);
#endif
        nvs = ClipPolygon(-1, nvs, (VERTEX3D *)&srfT->v, zc, v2);
#ifdef DEBUG
        if(nvs >= MAXNV2 || nvs < 0) {
          error(3, __FILE__, __LINE__,
                "Invalid number of vertices (%d) in ViewObstructed",
                nvs);
        }
#endif
        if(nvs < 3) {
          continue; /* no shadow polygon created */
        }
        pv2 = v2;
#ifdef DEBUG
        DumpP3D("Clipped surface:", nvs, pv2);
#endif
      } else {
        pv2 = (void *)&srfT->v;
      }

      /* project obstruction from centroid to z=0 plane */
      for(n=0; n<nvs; n++,pv2++) {
        R8 temp = vpt[np].z / (vpt[np].z - pv2->z);  /* projection factor */
        vs[n].x = vpt[np].x - temp * (vpt[np].x - pv2->x);
        vs[n].y = vpt[np].y - temp * (vpt[np].y - pv2->y);
      }
      /* limit projected surface; avoid some HC problems */
#ifdef DEBUG
      if(nvs >= MAXNV2 || nvs < 0) {
        error(3, __FILE__, __LINE__,
              "Invalid number of vertices (%d) in ViewObstructed",
              nvs);
      }
#endif
      nvs = LimitPolygon(nvs, vs, xmax, xmin, ymax, ymin);
      if(nvs < 3) {
        continue; /* no shadow polygon created */
      }
#ifdef DEBUG
      /* bounds check on projected surface */
      {
        R8 temp = 0.0;
        for(n=0; n<nvs; n++) {
          if(fabs(vs[n].x) > temp) {
            temp = fabs(vs[n].x);
          }
          if(fabs(vs[n].y) > temp) {
            temp = fabs(vs[n].y);
          }
        }
        if(temp > 1.01) {
          error(1, __FILE__, __LINE__,
                "Projected surface too large in ViewObstructed");
        }
      }
#endif
      NewPolygonStack();
      shade = SetPolygonHC(nvs, vs, 0.0);
      if(shade) {
#ifdef DEBUG
        DumpHC("SHADOW:", shade, NULL);
#endif

        /* compute unshaded portion of surface 2 polygon */
        NewPolygonStack();
        /* Determine portions of old polygons outside the shadow polygon, */
        /* must save next to pop old stack */
        for(pp=stack; pp; pp=next) {
          next = pp->next;
          k = PolygonOverlap(shade, pp, 3, 1); /* 1 = popping old stack */
          if(k == -999) {
            break;
          }
        }
        if(k == -999) {
          stack = NULL;
        } else {
          stack = TopOfPolygonStack();
        }
        if(stack==NULL) {            /* no new unshaded polygons; so */
          break;                     /* polygon 2 is totally obstructed. */
        }
#ifdef DEBUG
        DumpHC("UNSHADED:", stack, NULL);
#endif
        FreePolygons(shade, NULL); /* free the shadow polygon */
      }  /* end shade */
    }  /* end of obstruction surfaces (J) loop */
    if(stack == NULL) {
      continue;
    }

    /* compute interchange area to each unshaded polygon */
    vfCtrl->totVpt += 1;
    for(pp=stack; pp; pp=pp->next) {
      vfCtrl->totPoly += 1;
      nv2 = GetPolygonVrt3D(pp, v2);
#ifdef DEBUG
      DumpP3D("Unshaded surface:", nv2, v2);
#endif
      dF = V1AIpart(nv2, v2, vpt+np, dc1);
#ifdef DEBUG
      fprintf(_ulog, " Partial view factor: %g\n", dF);
#endif
#ifdef DEBUG
      if(dF < 0.0) {
        if(dF < -1.0e-16) {
          error(1, __FILE__, __LINE__,
                "Negative F (%4g) set to 0 in ViewObstructed", dF);
          //# if( DEBUG > 0 )     /* normally 1 */
          DumpHC(" Polygon", pp, pp);
          fprintf(_ulog, " View point: (%g, %g, %g)\n", vpt[np].x, vpt[np].y, vpt[np].z);
          fprintf(_ulog, " Direction: (%g, %g, %g)\n", dc1->x, dc1->y, dc1->z);
          V1AIpart(nv2, v2, vpt+np, dc1);
          fflush(_ulog);
          //# endif
        }
        dF = 0.0;
      }
#endif
      dFv += dF;
    }

#ifdef DEBUG
    fprintf(_ulog, " SS: x %f, y %f, z %f, dFv %g\n",
            vpt[np].x, vpt[np].y, vpt[np].z, dFv);
#endif
    AFu += dFv * weight[np];
  }  /* end of view points (np) loop */

#ifdef DEBUG
  if(AFu < 0.0) { /* due to negative weight; enly np=0 */
    if(weight[0] > 0) {
      //if( AFu < -1.0e-11 )
      error(1, __FILE__, __LINE__,
            "Negative AFu (%4g) set to 0 in ViewObstructed", AFu);
    }
    AFu = 0.0;
  }
#endif

#ifdef DEBUG
  fprintf(_ulog, "v_obst_u AF:  %g\n", AFu);
  fflush(_ulog);
#endif

  return AFu;

}  /*  end of ViewObstructed  */

/***  V1AIpart.c  ************************************************************/

/*  Compute the radiation shape factor between infinitesimal surface
 *  P1 and polygon-shaped surface P2 by the contour integral method.
 *  Ref:  Hottel & Sarofim, "Radiative Transfer", 1967, p48.
 *  Result is positive if vertices of P2 are defined in clockwise order;
 *  otherwise, the result is negative.
 *  This algorithm uses only vector operations plus one SQRT and one
 *  ATAN function call per edge of surface P2.  Degenerate cases
 *  are not handled; good input geometry is assumed.
 */

R8 V1AIpart(const IX nv, const VERTEX3D p2[],
            const VERTEX3D *p1, const DIRCOS *u1)
/*  nv   number of vertices/edges of surface (polygon) P2
 *  p2   coordinates of vertices of surface (polygon) P2
 *  p1   coordinates of surface (point) P1
 *  u1   components of unit vector normal to surface P1 */
{
  IX n;  /* edge number */
  VECTOR3D A,  /* A = vector from P1 to P2[n-1]; |A| > 0 */
      B,  /* B = vector from P1 to P2[n]; |B| > 0 */
      C;  /* C = vector cross product of A and B */
  R8 UdotC; /* dot product of U and C; always >= 0 */
  R8 sum=0; /* sum of line integrals */

  /* Initialization */
  n = nv - 1;
  VECTOR(p1, (p2+n), (&B));           /* vector B */
  /* For all edges of polygon p2: */
  for(n=0; n<nv; n++) {
    VCOPY((&B), (&A));                /* A = old B */
    VECTOR(p1, (p2+n), (&B));         /* vector B */
    VCROSS((&A), (&B), (&C));         /* C = A cross B */
    UdotC = VDOT(u1, (&C));           /* U dot C */
    if(fabs(UdotC) > EPS2) {
      R8 Clen = VLEN((&C));           /* | C | */
      if(Clen > EPS2) {
        /* gamma = angle between A and B; 0 < gamma < 180 */
        R8 gamma = PId2 - atan(VDOT((&A), (&B)) / Clen);
        sum += UdotC * gamma / Clen;
      } else {
        /* Retained for historical reasons, DO NOT REMOVE. */
        error(3, __FILE__, __LINE__, "View1AI failed, call George");
      }
    }
  }  /* end edge loop */

  sum *= PIt2inv;                 /* Divide by 2*pi */

  return sum;

}  /* end of V1AIpart

/***  View1AI.c  *************************************************************/

/*  Estimate direct interchange area by single area integration.
 *  Surface 1 described by its direction cosines and NSS vertices
 *  and associated areas for numerical (Gaussian) integration.  */

R8 View1AI(IX nss, VERTEX3D *p1, R8 *area1, DIRCOS *dc1, SRFDAT3X *srf2)
{
  IX j;
  R8 sum=0.0;

  for(j=0; j<nss; j++) {
    sum += area1[j] * V1AIpart(srf2->nv, srf2->v, p1+j, dc1);
  }

  return sum;

}  /* end View1AI */

/***  Subsurface.c  **********************************************************/

/*  Divide convex polygon SRF into elemental subsurfaces SUB.  */
/*  See program TEST99B for verification tests. GNW  */

IX Subsurface(SRFDAT3X *srf, SRFDAT3X sub[])
{
  IX nSubSrf;       /* number of subsurfaces */
  VERTEX3D tmpVrt;  /* temporary vertex */
  VECTOR3D edge[4]; /* quadrilateral edge vectors */
  R8 edgeLength[4]; /* lengths of edges */
  R8 cosAngle[4];   /* cosines of angles */
  IX obtuse=0;      /* identifies obtuse angles */
  IX i, j, k;

  if(srf->shape > 0) {    /* triangle or parallelogram */
    memcpy(sub+0, srf, sizeof(SRFDAT3X));    /* no subdivision */
    sub[0].nr = 0;
    nSubSrf = 1;
  } else if(srf->nv==4) { /* convex quadrilateral */
    for(j=0,i=1; j<4; j++,i++) { /* compute edge vectors and lengths */
      i &= 3;   /* equivalent to: if(i==4) i = 0; in this context */
      VECTOR((srf->v+i), (srf->v+j), (edge+j));
      edgeLength[j] = VLEN((edge+j));
    }
    for(k=1,j=0,i=3; j<4; j++,i++,k*=2) { /* compute corner angles */
      i &= 3;                /* A dot B = |A|*|B|*cos(angle) */
      cosAngle[j] = -VDOT((edge+j), (edge+i)) / (edgeLength[j] * edgeLength[i]);
      if(cosAngle[j] < 0.0) { /* angle > 90 if cos(angle) < 0 */
        obtuse += k;
      }
    }
#ifdef DEBUG
    for(j=0; j<4; j++) {
      fprintf(_ulog, " edge %d: (%f %f %f) L %f, C %f (%.3f deg)\n", j+1,
              edge[j].x, edge[j].y, edge[j].z,
              edgeLength[j], cosAngle[j], acos(cosAngle[j])*RTD);
    }
    fprintf(_ulog, " quadrilateral, case %d\n", obtuse);
    fflush(_ulog);
#endif
    switch (obtuse) { /* divide based on number and positions of obtuse angles */
    case 0:       /* rectangle */
      memcpy(sub+0, srf, sizeof(SRFDAT3X));    /* no subdivision */
      sub[0].nr = 0;
      nSubSrf = 1;
      break;
    case 1:       /* only angle 0 is obtuse */
    case 4:       /* only angle 2 is obtuse */
    case 5:       /* angles 0 and 2 are obtuse */
    case 7:       /* angles 0, 1, and 2 are obtuse */
    case 13:      /* angles 0, 2, and 3 are obtuse */
      VCOPY((srf->v+0), (sub[0].v+0));
      VCOPY((srf->v+1), (sub[0].v+1));
      VCOPY((srf->v+2), (sub[0].v+2));
      VCOPY((srf->v+2), (sub[1].v+0));
      VCOPY((srf->v+3), (sub[1].v+1));
      VCOPY((srf->v+0), (sub[1].v+2));
      nSubSrf = 2;
      break;
    case 2:       /* only angle 1 is obtuse */
    case 8:       /* only angle 3 is obtuse */
    case 10:      /* angles 1 and 3 are obtuse */
    case 11:      /* angles 0, 1, and 3 are obtuse */
    case 14:      /* angles 1, 2, and 3 are obtuse */
      VCOPY((srf->v+1), (sub[0].v+0));
      VCOPY((srf->v+2), (sub[0].v+1));
      VCOPY((srf->v+3), (sub[0].v+2));
      VCOPY((srf->v+3), (sub[1].v+0));
      VCOPY((srf->v+0), (sub[1].v+1));
      VCOPY((srf->v+1), (sub[1].v+2));
      nSubSrf = 2;
      break;
    case 3:       /* angles 0 and 1 are obtuse */
      tmpVrt.x = 0.5f * (srf->v[2].x + srf->v[3].x);
      tmpVrt.y = 0.5f * (srf->v[2].y + srf->v[3].y);
      tmpVrt.z = 0.5f * (srf->v[2].z + srf->v[3].z);
      VCOPY((srf->v+3), (sub[0].v+0));
      VCOPY((srf->v+0), (sub[0].v+1));
      VCOPY(( &tmpVrt), (sub[0].v+2));
      VCOPY((srf->v+0), (sub[1].v+0));
      VCOPY((srf->v+1), (sub[1].v+1));
      VCOPY(( &tmpVrt), (sub[1].v+2));
      VCOPY((srf->v+1), (sub[2].v+0));
      VCOPY((srf->v+2), (sub[2].v+1));
      VCOPY(( &tmpVrt), (sub[2].v+2));
      nSubSrf = 3;
      break;
    case 6:       /* angles 1 and 2 are obtuse */
      tmpVrt.x = 0.5f * (srf->v[0].x + srf->v[3].x);
      tmpVrt.y = 0.5f * (srf->v[0].y + srf->v[3].y);
      tmpVrt.z = 0.5f * (srf->v[0].z + srf->v[3].z);
      VCOPY((srf->v+0), (sub[0].v+0));
      VCOPY((srf->v+1), (sub[0].v+1));
      VCOPY(( &tmpVrt), (sub[0].v+2));
      VCOPY((srf->v+1), (sub[1].v+0));
      VCOPY((srf->v+2), (sub[1].v+1));
      VCOPY(( &tmpVrt), (sub[1].v+2));
      VCOPY((srf->v+2), (sub[2].v+0));
      VCOPY((srf->v+3), (sub[2].v+1));
      VCOPY( (&tmpVrt), (sub[2].v+2));
      nSubSrf = 3;
      break;
    case 9:       /* angles 0 and 3 are obtuse */
      tmpVrt.x = 0.5f * (srf->v[1].x + srf->v[2].x);
      tmpVrt.y = 0.5f * (srf->v[1].y + srf->v[2].y);
      tmpVrt.z = 0.5f * (srf->v[1].z + srf->v[2].z);
      VCOPY((srf->v+2), (sub[0].v+0));
      VCOPY((srf->v+3), (sub[0].v+1));
      VCOPY( (&tmpVrt), (sub[0].v+2));
      VCOPY((srf->v+3), (sub[1].v+0));
      VCOPY((srf->v+0), (sub[1].v+1));
      VCOPY( (&tmpVrt), (sub[1].v+2));
      VCOPY((srf->v+0), (sub[2].v+0));
      VCOPY((srf->v+1), (sub[2].v+1));
      VCOPY( (&tmpVrt), (sub[2].v+2));
      nSubSrf = 3;
      break;
    case 12:      /* angles 2 and 3 are obtuse */
      tmpVrt.x = 0.5f * (srf->v[0].x + srf->v[1].x);
      tmpVrt.y = 0.5f * (srf->v[0].y + srf->v[1].y);
      tmpVrt.z = 0.5f * (srf->v[0].z + srf->v[1].z);
      VCOPY((srf->v+1), (sub[0].v+0));
      VCOPY((srf->v+2), (sub[0].v+1));
      VCOPY( (&tmpVrt), (sub[0].v+2));
      VCOPY((srf->v+2), (sub[1].v+0));
      VCOPY((srf->v+3), (sub[1].v+1));
      VCOPY( (&tmpVrt), (sub[1].v+2));
      VCOPY((srf->v+3), (sub[2].v+0));
      VCOPY((srf->v+0), (sub[2].v+1));
      VCOPY( (&tmpVrt), (sub[2].v+2));
      nSubSrf = 3;
      break;
    case 15:      /* invalid configuration */
      error(2, __FILE__, __LINE__, "Invalid configuration in Subsurface");
      break;
    default:
      error(2, __FILE__, __LINE__, "Invalid switch %d in Subsurface",obtuse);
    } /* end switch */
    if(obtuse > 0) {   /* complete subdivision data */
      for(j=0; j<nSubSrf; j++) {
        sub[j].nr = j;
        sub[j].nv = 3;
        SetCentroid(3, sub[j].v, &sub[j].ctd);
        sub[j].area = Triangle(sub[j].v+0, sub[j].v+1, sub[j].v+2, &tmpVrt, 0);
        memcpy(&sub[j].dc, &srf->dc, sizeof(DIRCOS));
      }
    }
  } else if(srf->nv==5) {
    for(j=0,i=1; j<5; j++,i++) {
      if(i==5) {
        i = 0;
      }
      VCOPY((&srf->ctd), (sub[j].v+0));
      VCOPY( (srf->v+j), (sub[j].v+1));
      VCOPY( (srf->v+i), (sub[j].v+2));
      sub[j].nr = j;
      sub[j].nv = 3;
      SetCentroid(3, sub[j].v, &sub[j].ctd);
      sub[j].area = Triangle(sub[j].v+0, sub[j].v+1, sub[j].v+2, &tmpVrt, 0);
      memcpy(&sub[j].dc, &srf->dc, sizeof(DIRCOS));
    }
    nSubSrf = 5;
  } else {
    error(3, __FILE__, __LINE__,
          "Invalid number of vertices (%d) in Subsurface",srf->nv);
  }

  return nSubSrf;

}  /* end Subsurface */

/***  SetCentroid.c  *********************************************************/

/*  Compute approximate centroid and enclosing radius.  */

R8 SetCentroid(const IX nv, VERTEX3D *vs, VERTEX3D *ctd)
/* nv  - number of vertices of surface.
 * vs  - coordinates of vertices.
 * ctd - coordinates of centroid.
 */
{
  VECTOR3D v;
  R8 r2, rr=0.0;
  IX j;

  ctd->x = ctd->y = ctd->z = 0.0;
  for(j=0; j<nv; j++) {
    ctd->x += vs[j].x;
    ctd->y += vs[j].y;
    ctd->z += vs[j].z;
  }
  ctd->x /= (R8)nv;
  ctd->y /= (R8)nv;
  ctd->z /= (R8)nv;

  for(j=0; j<nv; j++) {
    VECTOR(ctd, (vs+j), (&v));
    r2 = VDOT((&v), (&v));
    if(r2 > rr) {
      rr = r2;
    }
  }

  return sqrt(rr);

}  /* end of SetCentroid */

/***  Triangle.c  ************************************************************/

/*  Compute directions cosines and area of a triangle from the
 *  vector cross product:  C = A X B
 *  where   A = vector from point P2 to point P3,
 *    and   B = vector from P2 to P1.  */

R8 Triangle(VERTEX3D *p1, VERTEX3D *p2, VERTEX3D *p3, void *dc, IX dcflag)
/* p1  - X, Y, & Z coordinates of point P1.
 * p2  - X, Y, & Z coordinates of point P2.
 * p3  - X, Y, & Z coordinates of point P3.
 *  c  - X, Y, & Z components of direction cosines vector "C".
 */
{
  VECTOR3D a, b, *c=(void *)dc;
  R8 r;  /* length of "C" (= twice the area of triangle P1-P2-P3) */

  VECTOR(p2, p3, (&a));
  VECTOR(p2, p1, (&b));
  VCROSS((&a), (&b), c);
  r = VLEN(c);
  if(dcflag) { /* compute direction cosines */
    if(r <= 1.e-12) {
      fprintf(_ulog, "Vertices:\n");
      fprintf(_ulog, "  %f  %f  %f\n", p1->x, p1->y, p1->z);
      fprintf(_ulog, "  %f  %f  %f\n", p2->x, p2->y, p2->z);
      fprintf(_ulog, "  %f  %f  %f\n", p3->x, p3->y, p3->z);
      error(3, __FILE__, __LINE__, "Vertices give invalid area in Triangle");
    }
    c->x /= r;   /* reduce C to unit length */
    c->y /= r;
    c->z /= r;
  }

#ifdef XXX
  fprintf(_ulog, " DC: %f %f %f; A: %f\n", c->x, c->y, c->z, 0.5*r);
#endif

  return (0.5f * r);

}  /* end of Triangle */

/***  SubsrfTS.c  ************************************************************/

/*  Compute vertices S of subsurface N of triangular surface V. 
 *  The triangle is divided into 4 congruent subsurfaces.  */

void SubsrfTS(IX n, VERTEX3D v[], VERTEX3D s[])
{
  IX j;

  if(n==3) {
    s[0].x = 0.5f * (v[1].x + v[2].x);
    s[0].y = 0.5f * (v[1].y + v[2].y);
    s[0].z = 0.5f * (v[1].z + v[2].z);
    s[1].x = 0.5f * (v[0].x + v[2].x);
    s[1].y = 0.5f * (v[0].y + v[2].y);
    s[1].z = 0.5f * (v[0].z + v[2].z);
    s[2].x = 0.5f * (v[1].x + v[0].x);
    s[2].y = 0.5f * (v[1].y + v[0].y);
    s[2].z = 0.5f * (v[1].z + v[0].z);
  } else {
    for(j=0; j<3; j++)
    {
      s[j].x = 0.5f * (v[n].x + v[j].x);
      s[j].y = 0.5f * (v[n].y + v[j].y);
      s[j].z = 0.5f * (v[n].z + v[j].z);
    }
  }

}  /* end SubsrfTS */

/***  ViewTP.c  **************************************************************/

/*  Compute view from triangle to polygon.  Recursive calculation!  */
/*  See program TEST99A for verification tests.  */

R8 ViewTP(VERTEX3D v1[], R8 area, IX level, VFCTRL *vfCtrl)
/* v1   - vertices of surface 1;
 * area - area of surface 1;
 * vfCtrl->maxRecursion - recursion limit. */
{
  R8 AF;      /* AF values computed for triangle */
  R8 AF7,     /* AF values for 7-  */
      AF13;     /* and 13-point integration */
  R8 dF;      /* differential view factor */
  IX cnvg;    /* true if both AF.. sufficiently close */
  VERTEX3D vt[3]; /* vertices of subsurfaces */
  IX n;       /* subsurface number */

  if(level >= vfCtrl->minRecursion) {
    AF7 = ViewObstructed(vfCtrl, 3, v1, area, 3);
    AF13 = ViewObstructed(vfCtrl, 3, v1, area, 4);
  } else {
    AF7 = -vfCtrl->epsAF;
    AF13 = vfCtrl->epsAF;
  }

  if(fabs(AF13 - AF7) < vfCtrl->epsAF) {
    cnvg = 1;
  } else {
    cnvg = 0;
    if(level++ >= vfCtrl->maxRecursion)
      vfCtrl->failRecursion = cnvg = 1;     /* limit maximum recursions */
  }

  if(level >= vfCtrl->minRecursion) {
    vfCtrl->wastedVObs += 7;
    if(cnvg) {
      vfCtrl->usedVObs += 13;
    } else {
      vfCtrl->wastedVObs += 13;
    }
  }

  if(cnvg) {     /* AF7 and AF13 are similar; */
    AF = AF13;    /* therefore, assume AF13 is accurate. */
  } else {
    for(AF=0.0,n=0; n<4; n++) {  /* Otherwise, divide triangle into four subsurfaces */
      SubsrfTS(n, v1, vt);       /* and compute AF for each subsurface and sum. */
      dF = ViewTP(vt, 0.25f*area, level, vfCtrl);
      AF += dF;
#ifdef DEBUG
      fprintf(_ulog, "  ViewTP (%d) AF: %d (%f) %f\n", level, n, dF, AF);
#endif
    }
  }

  return AF;

}  /* end ViewTP */

/***  SubsrfRS.c  ************************************************************/

/*  Compute vertices S of subsurface N of rectangular surface V.
 *  The rectangle is divided into 4 identical subsurfaces.  */

void SubsrfRS(IX n, VERTEX3D v[], VERTEX3D s[])
{
  IX j;

  for(j=0; j<4; j++) {
    s[j].x = 0.5f * (v[n].x + v[j].x);
    s[j].y = 0.5f * (v[n].y + v[j].y);
    s[j].z = 0.5f * (v[n].z + v[j].z);
  }

}  /* end SubsrfRS */

/***  ViewRP.c  **************************************************************/

/*  Compute view from rectangle to polygon.  Recursive calculation!  */
/*  See program TEST99A for verification tests.  */

R8 ViewRP(VERTEX3D v1[], R8 area, IX level, VFCTRL *vfCtrl)
{
  R8 AF;      /* AF values computed for rectangle */
  R8 AF9,     /* AF values for 9-  */
      AF16;     /* and 16-point integration */
  R8 dF;      /* differential view factor */
  IX cnvg;    /* true if both AF.. sufficiently close */
  VERTEX3D vt[4]; /* vertices of subsurfaces */
  IX n;       /* subsurface number */

  if(level >= vfCtrl->minRecursion) {
    AF9 = ViewObstructed(vfCtrl, 4, v1, area, 3);
    AF16 = ViewObstructed(vfCtrl, 4, v1, area, 4);
  } else {
    AF9 = -vfCtrl->epsAF;
    AF16 = vfCtrl->epsAF;
  }

  if(fabs(AF16 - AF9) < vfCtrl->epsAF) {
    cnvg = 1;
  } else {
    cnvg = 0;
    if(level++ >= vfCtrl->maxRecursion)
      vfCtrl->failRecursion = cnvg = 1;     /* limit maximum recursions */
  }

  if(level >= vfCtrl->minRecursion) {
    vfCtrl->wastedVObs += 9;
    if(cnvg) {
      vfCtrl->usedVObs += 16;
    } else {
      vfCtrl->wastedVObs += 16;
    }
  }

  if(cnvg) {      /* AF9 and AF16 are similar; */
    AF = AF16;    /* therefore, assume AF16 is accurate. */
  } else {
    for(AF=0.0,n=0; n<4; n++) {  /* Otherwise, divide rectangle into four subsurfaces*/
      SubsrfRS(n, v1, vt);       /* and compute AF for each subsurface and sum. */
      dF = ViewRP(vt, 0.25f*area, level, vfCtrl);
      AF += dF;
#ifdef DEBUG
      fprintf(_ulog, "  ViewRP (%d) AF: %d (%f) %f\n", level, n, dF, AF);
#endif
    }
  }

  return AF;

}  /* end ViewRP */

