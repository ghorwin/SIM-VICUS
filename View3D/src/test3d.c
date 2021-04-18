/*subfile:  test3d.c  *********************************************************/
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

#include <stdio.h>
#include <string.h> /* prototype: memcpy */
#include <math.h>   /* prototype: fabs */
#include "types.h" 
#include "view3d.h"
#include "prtyp.h" 

/*  VSHIFT:  vector C = vector B minus scalar D times vector A.  */
#define VSHIFT(b,d,a,c)  \
  c->x = b->x - d * a->x; \
  c->y = b->y - d * a->y; \
  c->z = b->z - d * a->z;

extern IX _list;    /* output control, higher value = more output */
extern I1 _string[LINELEN];  /* buffer for a character string */
extern FILE *_ulog; /* log file */

/***  AddMaskSrf.c  **********************************************************/

/*  Add mask/null surfaces to list of possible obstructions.
 *  Allow for a mask on a subsurface which also masks the base surface. */

IX AddMaskSrf(SRFDAT3D *srf, const SRFDATNM *srfN, const SRFDATNM *srfM,
              const IX *maskSrf, const IX *baseSrf, VFCTRL *vfCtrl, IX *possibleObstr,
              IX nPoss)
/* srfN - data for surface N.
 * srfM - data for surface M.
 * maskSrf - list of mask and null surfaces.
 * baseSrf - list of base surfaces.
 * possibleObstr  - list of possible obstructing surfaces.
 * vfCtrl - computation controls.
 * nPoss - number of possible obstructions.
 */
{
  IX j, k, n;
  SRFDAT3D *ps;
  IX Ns=srfN->nr,
      Ms=srfM->nr;  /* surface numbers */
  R8 eps;  /* dot product test value */
  IX nv;   /* number of vertices */

  if(srfN->rc < srfM->rc) {
    eps = 1.0e-5f * srfN->rc;
  } else {
    eps = 1.0e-5f * srfM->rc;
  }
  nv = srfM->nv;
  for(j=vfCtrl->nMaskSrf; j; j--) { /* look for masks of srfN */
    for(k=baseSrf[maskSrf[j]]; k; k=baseSrf[k]) {
      if(k == Ns) {
        ps = srf + maskSrf[j];
        for(n=0; n<nv; n++) {       /* mask may not be behind srfM */
          if(VDOTW((ps->v[n]), (&srfM->dc)) > eps) {
            break;
          }
        }
        if(n==nv) {
          continue;
        }
        possibleObstr[++nPoss] = maskSrf[j];
        ps->NrelS = 1;
        ps->MrelS = -1;
      }
    }
  }

  nv = srfN->nv;
  for(j=vfCtrl->nMaskSrf; j; j--) { /* look for masks of srfM */
    for(k=baseSrf[maskSrf[j]]; k; k=baseSrf[k]) {
      if(k == Ms) {
        ps = srf + maskSrf[j];
        for(n=0; n<nv; n++) {       /* mask may not be behind srfN */
          if(VDOTW((ps->v[n]), (&srfN->dc)) > eps) {
            break;
          }
        }
        if(n==nv) {
          continue;
        }
        possibleObstr[++nPoss] = maskSrf[j];
        ps->MrelS = 1;
        ps->NrelS = -1;
      }
    }
  }

  if(vfCtrl->col && nPoss && _list>3) {
    DumpOS("AddMaskSrf LOS:", nPoss, possibleObstr);
  }

  return nPoss;

}  /* end AddMaskSrf */

/***  BoxTest.c  *************************************************************/

/*  Box test to reduce the list of possible obstructing surfaces:
 *  obstruction may not lie outside box containing surfaces N and M.  */

IX BoxTest(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM,
           VFCTRL *vfCtrl, IX *possibleObstr, IX nPossObstr)
/* srf  - data for all surfaces.
 * srfN - data for surface N.
 * srfM - data for surface M.
 * possibleObstr  - list of possible obstructing surfaces (output).
 * nPossObsrt  - number of possible obstructing surfaces
 */
{
  R8 xmax, xmin, ymax, ymin, zmax, zmin;  /* limits of box enclosing N & M */
  IX nv;     /* number of vertices */
  IX n;      /* vertex number */
  IX i, k;   /* surface number */
  IX nPoss;  /* number of possible obstructing surfaces */

#ifdef DEBUG
  fprintf(_ulog, "BoxTest: %d\n", nPossObstr);
#endif
  xmax = xmin = srfN->v[0].x;
  ymax = ymin = srfN->v[0].y;
  zmax = zmin = srfN->v[0].z;
  for(n=1; n<srfN->nv; n++) {
    if(srfN->v[n].x > xmax) {
      xmax = srfN->v[n].x;
    }
    if(srfN->v[n].x < xmin) {
      xmin = srfN->v[n].x;
    }
    if(srfN->v[n].y > ymax) {
      ymax = srfN->v[n].y;
    }
    if(srfN->v[n].y < ymin) {
      ymin = srfN->v[n].y;
    }
    if(srfN->v[n].z > zmax) {
      zmax = srfN->v[n].z;
    }
    if(srfN->v[n].z < zmin) {
      zmin = srfN->v[n].z;
    }
  }
  for(n=0; n<srfM->nv; n++) {
    if(srfM->v[n].x > xmax) {
      xmax = srfM->v[n].x;
    }
    if(srfM->v[n].x < xmin) {
      xmin = srfM->v[n].x;
    }
    if(srfM->v[n].y > ymax) {
      ymax = srfM->v[n].y;
    }
    if(srfM->v[n].y < ymin) {
      ymin = srfM->v[n].y;
    }
    if(srfM->v[n].z > zmax) {
      zmax = srfM->v[n].z;
    }
    if(srfM->v[n].z < zmin) {
      zmin = srfM->v[n].z;
    }
  }
  /* process possible view obstructing surfaces */
  for(nPoss=0,i=1; i<=nPossObstr; i++) {
    k = possibleObstr[i];
    nv = srf[k].nv;

    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->x < xmax) {
        break;
      }
    }
    if(n==nv) {
      continue;    /* all vertices > xmax; no obstruction */
    }
    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->x > xmin) {
        break;
      }
    }
    if(n==nv) {
      continue;    /* all vertices < xmin; no obstruction */
    }

    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->y < ymax) {
        break;
      }
    }
    if(n==nv) {
      continue;
    }
    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->y > ymin) {
        break;
      }
    }
    if(n==nv) {
      continue;
    }

    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->z < zmax) {
        break;
      }
    }
    if(n==nv) {
      continue;
    }
    for(n=0; n<nv; n++) {
      if(srf[k].v[n]->z > zmin) {
        break;
      }
    }
    if(n==nv) {
      continue;
    }
    /* K may be an obstruction */
    possibleObstr[++nPoss] = k;
  }

  if(vfCtrl->col && nPoss && _list>3) {
    DumpOS("BoxTest LOS:", nPoss, possibleObstr);
  }

  return nPoss;

}  /*  end of BoxTest  */

/***  ConeRadiusTest.c  ******************************************************/

/*  Cone (or cylinder) radius test to reduce the list
 *  of possible obstructing surfacess  */

IX ConeRadiusTest(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM,
                  VFCTRL *vfCtrl, IX *possibleObstr, IX nPossObstr, R8 distNM)
/* srf  - data for all surfaces.
 * srfN - data for surface N.
 * srfM - data for surface M.
 * possibleObstr  - list of possible obstructing surfaces (output).
 * distNM  - distance between centroids of N and M.
 * nPossObstr  - number of possible obstructing surfaces
 */
{
  IX mode=0; /* test mode; 0 = cylinder; -1 = cone from srfM;
                             +1 = cone from srfN  */
  R8 radCylndr;  /* radius of cylinder */
  R8 radSmall;   /* radius of smaller surface */
  R8 radLarge;   /* radius of larger surface */
  R8 distSmall;  /* distance from apex of cone to smaller surface */
  R8 distLarge;  /* distance from apex of cone to larger surface */
  R8 distK;      /* distance from apex to surface K */
  R8 d, e, f;
  DIRCOS dcNM;    /* direction cosines of line between srfN and srfM */
  VERTEX3D apex;  /* coordinates of apex of cone */
  VECTOR3D a, b;  /* vectors */
  IX i, k;   /* surface number */
  IX nPoss=0;  /* number of possible obstructing surfaces */

#ifdef DEBUG
  fprintf(_ulog, "ConeRadiusTest: %d\n", nPossObstr);
#endif

  if(srfN->rc < 0.7071*srfM->rc) mode = +1;
  if(srfM->rc < 0.7071*srfN->rc) mode = -1;

  if(mode<0) {
    VECTOR((&srfM->ctd), (&srfN->ctd), (&a));
    radSmall = srfM->rc;
    radLarge = srfN->rc;
  } else {
    VECTOR((&srfN->ctd), (&srfM->ctd), (&a));
    radSmall = srfN->rc;
    radLarge = srfM->rc;
  }
  d = 1.0f / distNM;
  VSCALE(d, (&a), (&dcNM));
  if(mode) {
    if(distNM<radLarge) mode = 0;
  }
#ifdef DEBUG
  fprintf(_ulog, "mode %d;  distNM %f;  dcNM %f %f %f\n",
          mode, distNM, dcNM.x, dcNM.y, dcNM.z);
#endif

  if(mode) {
    e = radSmall / (radLarge - radSmall);
    distSmall = e * distNM;
    distLarge = distSmall + distNM;
    f = 1.0f / (distSmall*distSmall - radSmall*radSmall);
    if(mode < 0) {
      VSHIFT((&srfM->ctd), e, (&a), (&apex));
    } else {
      VSHIFT((&srfN->ctd), e, (&a), (&apex));
    }
#ifdef DEBUG
    fprintf(_ulog, "Cone: %f %f %f %f  %f %f %f\n",
            radSmall, radLarge, distSmall, distLarge, apex.x, apex.y, apex.z);
#endif
  } else {
    radCylndr = MAX(srfN->rc, srfM->rc);
#ifdef DEBUG
    fprintf(_ulog, "Cylinder: %f\n", radCylndr);
#endif
  }
  /* process possible view obstructing surfaces */
  for(i=1; i<=nPossObstr; i++) {
    k = possibleObstr[i];
#ifdef DEBUG
    fprintf(_ulog, "K: %d\n", k);
#endif
    if(mode) {                    /* cone radius test */
      VECTOR((&apex), (&srf[k].ctd), (&a));
      distK = VDOT((&a), (&dcNM));  /* distance from apex tests */
      if((distK+srf[k].rc) <= (distSmall-radSmall)) continue;
      if((distK-srf[k].rc) >= (distLarge+radLarge)) continue;
      VCROSS((&dcNM), (&a), (&b));
      d = radSmall * distK + distSmall * srf[k].rc;
      if(VDOT((&b),(&b)) > (f*d*d)) continue;
    } else {                    /* cylinder radius test */
      VECTOR((&srfN->ctd), (&srf[k].ctd), (&a));
      VCROSS((&dcNM), (&a), (&b));
      d = radCylndr + srf[k].rc;
      if(VDOT((&b),(&b)) > (d*d)) {
        continue;
      }
    }
#ifdef DEBUG
    fprintf(_ulog, "Passed radius test\n");
#endif
    /* K may be an obstruction */
    possibleObstr[++nPoss] = k;
  }

  if(vfCtrl->col && nPoss && _list>3) {
    DumpOS("ConeRadiusTest LOS:", nPoss, possibleObstr);
  }

  return nPoss;

}  /*  end of ConeRadiusTest  */

/***  OrientationTest.c  *****************************************************/

/*  Orientation tests to reduce the list of possible obstructs  */

IX OrientationTest(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM,
                   VFCTRL *vfCtrl, IX *possibleObstr, IX nPossObstr)
/* srf  - data for all surfaces.
 * srfN - data for surface N.
 * srfM - data for surface M.
 * possibleObstr  - list of possible obstructing surfaces (output).
 * nPossObsrt  - number of possible obstructing surfaces
 */
{
  R8 dot, eps; /* dot product and test value */
  IX infront;  /* true if a vertex of surface #2 is in front of surface 1 */
  IX behind;   /* true if a vertex of surface #2 is behind surface 1 */
  IX nv;      /* number of vertices */
  IX n;       /* vertex number */
  IX i, k;    /* surface number */
  IX nPoss;   /* number of possible obstructing surfaces */

#ifdef DEBUG
  fprintf(_ulog, "OrientationTest: %d\n", nPossObstr);
#endif
  if(srfN->rc < srfM->rc) {
    eps = 1.0e-5f * srfN->rc;
  }
  else {
    eps = 1.0e-5f * srfM->rc;
  }

  /* process possible view obstructing surfaces */
  for(nPoss=0,i=1; i<=nPossObstr; i++) {
    k = possibleObstr[i];
#ifdef DEBUG
    fprintf(_ulog, "K: %d\n", k);
#endif
    nv = srf[k].nv;
    /* no obstruction if K totally behind N - ==> OrientationTestN() */
    /* no obstruction if K totally behind M */
    for(n=0; n<nv; n++) {
      if(VDOTW((srf[k].v[n]), (&srfM->dc)) > eps) {
        break;
      }
    }
    if(n==nv) {
      continue;
    }
#ifdef DEBUG
    fprintf(_ulog, "K in front of srfM\n");
#endif

    infront = behind = 0;  /* check vertices of N relative to K */
    for(n=srfN->nv; n; n--) {
      dot = VDOTW((srfN->v+n-1), (&srf[k].dc));
      if(dot >  eps) infront = 1;
      if(dot < -eps) behind = 1;
    }
    srf[k].NrelS = infront - behind;
    if(infront + behind == 0) {
      continue;   /* coplanar surfaces */
    }

    infront = behind = 0;  /* check vertices of M relative to K */
    for(n=srfM->nv; n; n--) {
      dot = VDOTW((srfM->v+n-1), (&srf[k].dc));
      if(dot >  eps) infront = 1;
      if(dot < -eps) behind = 1;
    }
    srf[k].MrelS = infront - behind;
    if(infront + behind == 0) {
      continue;   /* coplanar surfaces */
    }
#ifdef DEBUG
    fprintf(_ulog, "NrelS %d, MrelS %d\n", srf[k].NrelS, srf[k].MrelS);
#endif
    /* no obstruction if N & M in front of K */
    /* no obstruction if N & M behind K */
    if(srf[k].NrelS * srf[k].MrelS > 0) {
      continue;
    }

    /* K may be an obstruction */
    possibleObstr[++nPoss] = k;
  }  /* end i loop */

  if(vfCtrl->col && nPoss && _list>3) {
    DumpOS("OrientationTest LOS:", nPoss, possibleObstr);
  }

  return nPoss;

}  /*  end of OrientationTest  */

/***  OrientationTestN.c  *********************************************************/

/*  Remove possible obstructions behind N from list.  */

IX OrientationTestN(SRFDAT3D *srf, IX N, VFCTRL *vfCtrl,
                    IX *possibleObstr, IX nPossObstr)
/* srf  - data for all surfaces.
 * N - number of surface N.
 * possibleObstr  - list of possible obstructing surfaces (input/output).
 * nPossObsrt  - number of possible obstructing surfaces; return new value.
 */
{
  IX j;       /* vertex number */
  IX i, k;    /* surface number */
  IX nPoss;   /* number of possible obstructing surfaces */
  R8 eps = 1.0e-5 * srf[N].rc;

#ifdef DEBUG
  fprintf(_ulog, "OrientationTestN: %d\n", nPossObstr);
#endif

  for(nPoss=0,i=1; i<=nPossObstr; i++) {
    k = possibleObstr[i];
    /* no obstruction if K totally behind N */
    for(j=srf[k].nv; j; j--) {
      if(VDOTW((srf[k].v[j-1]), (&srf[N].dc)) > eps) {
        break;
      }
    }
    if(j) {                /* K may be an obstruction */
      possibleObstr[++nPoss] = k;
    }
  }  /* end i loop */

  if(vfCtrl->col && nPoss && _list>3) {
    DumpOS("OrientationTestN LOS:", nPoss, possibleObstr);
  }

  return nPoss;

}  /*  end of OrientationTestN  */

/***  ClipPolygon.c  *********************************************************/

/*  Clip polygon according to FLAG and DIST vector.
 *  Ordering of vertices is retained.
 *  Return number of vertices in clipped polygon.  */

IX ClipPolygon(const IX flag, const IX nVu, VERTEX3D *vPoly, R8 *dist,
               VERTEX3D *vClipPoly)
/* flag   - save vertices above or below clipping plane
 *          for flag equal to 1 or -1, respectively.
 * nVu    - number of vertices of the unclipped polygon.
 * vPoly  - vertices of unclipped polygon.
 * dist   - distance of each vertex above clipping plane; found by dot
 *          product of vertex with clipping plane normal form coefficients.
 * vClipPoly - vertices of the clipped polygon. */
{
  IX j, jm1;   /* vertex indices;  jm1 = j - 1 */
  IX nVc=0;    /* number of vertices of clipped polygon */

  jm1 = nVu - 1;
  for(j=0; j<nVu; jm1=j++) {
    if(dist[j] * dist[jm1] < 0.0) {  /* save intercept with plane */
                   /* Sign test means vertices must be on opposite sides */
      R8 h;          /* of clipping plane with neither vertex on the plane. */
      h = dist[j] / (dist[j] - dist[jm1]);   /* no division by zero */
      vClipPoly[nVc].x = vPoly[j].x + h * (vPoly[jm1].x - vPoly[j].x);
      vClipPoly[nVc].y = vPoly[j].y + h * (vPoly[jm1].y - vPoly[j].y);
      vClipPoly[nVc++].z = vPoly[j].z + h * (vPoly[jm1].z - vPoly[j].z);
    }
    if(flag * dist[j] >= 0.0) { /* flag >= 0: save point on or above plane */
                                /* flag <= 0: save point on or below plane */
      vClipPoly[nVc].x = vPoly[j].x;
      vClipPoly[nVc].y = vPoly[j].y;
      vClipPoly[nVc++].z = vPoly[j].z;
    }
  }

  return nVc;

}  /* end of ClipPolygon */

/***  SelfObstructionClip.c  *************************************************/

/*  Clip surface N.  This may increase the number of vertices by one.  */

void SelfObstructionClip(SRFDATNM *srfN)
{
  VERTEX3D v[MAXNV1];   /* vertices after clipping; DIST from SOTEST3 */

  srfN->nv = ClipPolygon(1, srfN->nv, srfN->v, srfN->dist, v);

  memcpy((void *)&srfN->v, (void *)v, srfN->nv * sizeof(VERTEX3D));

  srfN->rc = SetCentroid(srfN->nv, v, &srfN->ctd);  /* centroid and radius */

  srfN->shape = SetShape(srfN->nv, v, &srfN->area);  /* shape and area */

}  /*  end of SelfObstructionClip  */

/***  SetShape.c  ************************************************************/

/*  Set shape number and area of convex polygon.  */

IX SetShape(const IX nv, VERTEX3D *v, R8 *area)
{
  VECTOR3D c;
  const static R8 epsA=1.0e-5;
  IX j, shape=0;

  if(nv == 3) {
    *area = Triangle(v+0, v+1, v+2, &c, 0);
    shape = 3;
  } else if(nv == 4) {
    VECTOR3D vv[4];   /* vectors defining polygon edges */
    R8 edge[4];       /* length^2 of edge vectors */
    R8 area0, area2;
    VECTOR((v+0), (v+1), (vv+0));  /* vector from v[0] to v[1] */
    VECTOR((v+2), (v+1), (vv+1));  /* vector from v[2] to v[1] */
    VECTOR((v+2), (v+3), (vv+2));  /* vector from v[2] to v[3] */
    VECTOR((v+0), (v+3), (vv+3));  /* vector from v[0] to v[3] */
    VCROSS((vv+0), (vv+3), (&c));
    area0 = VLEN((&c));
    VCROSS((vv+1), (vv+2), (&c));
    area2 = VLEN((&c));
    *area = 0.5f * (area0 + area2);
    for(j=0; j<nv; j++)
      edge[j] = VDOT((vv+j), (vv+j));
    if(fabs(edge[2] - edge[0]) < epsA * (edge[0] + edge[2]))
      if(fabs(edge[3] - edge[1]) < epsA * (edge[1] + edge[3]))
        shape = 4;      /* parallelogram */
    //      if( fabs(area0 - area2) < epsA * (area0 + area2) )
    //        shape = 4;      /* rectangle */
    //      else
    //        errorf( 1, __FILE__, __LINE__, "Parallelogram", "" );
  } else {
    *area = 0.0;
    for(j=2; j<nv; j++)
      *area += Triangle(v+0, v+j-1, v+j, &c, 0);
  }

  return shape;

}  /* end SetShape */

/***  SelfObstructionTest3D.c  ***********************************************/

/*  Self-obstruction test -- determine the portion of SRF2 that is 
 *  in front of SRF1; transfer SRF2 data to srfN.
 *  Return 0 if the SRF2 is entirely behind SRF1; return 1 if SRF2 is fully
 *  or partially in front.  If partially in front, srfN->area set to 0.  */

IX SelfObstructionTest3D(SRFDAT3D *srf1, SRFDAT3D *srf2, SRFDATNM *srfN)
{
  IX nv;     /* number of vertices of surface #2 */
  IX n;      /* vertex number */
  IX infront=0; /* true if a vertex of surface #2 is in front of surface 1 */
  IX behind=0;  /* true if a vertex of surface #2 is behind surface 1 */
  R8 dist;   /* distance in front of plane; save for clipping calc. */

  nv = srf2->nv;
  /* check centroid of srf2 against srf1;*/
  dist = VDOTW((&srf2->ctd), (&srf1->dc));
  if(dist >= srf2->rc) infront = 1;
  if(dist <= -srf2->rc) behind = 1;

  if(!infront && !behind) { /* more detailed test: */
    R8 eps=1.0e-5*srf2->rc; /* test value for "close to plane of SRF1" */
    for(n=0; n<nv; n++) {   /* check all vertices of srf2 against srf1 */
      dist = srfN->dist[n] = VDOTW((srf2->v[n]), (&srf1->dc));
      if(dist > eps) {
        infront = 1;
      } else if(dist < -eps) {
        behind = 1;
      } else {
        srfN->dist[n] = 0.0;
      }
    }
  }

  if(infront) {    /* size assumes contiguous variables in the structs */
    static const IX size=(4*sizeof(IX)+2*sizeof(R8)+sizeof(DIRCOS)+sizeof(VERTEX3D));
    memcpy(srfN, srf2, size);
    for(n=0; n<nv; n++) {
      memcpy(srfN->v+n, srf2->v[n], sizeof(VERTEX3D));
    }
    if(behind) {
      srfN->area = 0.0;   /* flag for clipping calculation */
    }
    return 1;
  } else {                   /* all vertices of #2 on or behind #1 plane */
    return 0;
  }
}  /*  end of SelfObstructionTest3D  */

/***  IntersectionTest.c  ****************************************************/

/*  Test for intersecting surfaces; vertices from SelfObstructionClip().
 *  Probably not used often, so efficiency is not important.  */

void IntersectionTest(SRFDATNM *srfN, SRFDATNM *srfM)
{
  VERTEX3D *p1, *p2, *p3;  /* vertices P1 and P2 on one polygon;
                              vertex P3 on the other. */
  VECTOR3D *pv12, v12;  /* vector from P1 to P2 */
  VECTOR3D *pv13, v13;  /* vector from P1 to P3 */
  VECTOR3D *pv23, v23;  /* vector from P2 to P3 */
  R8 lV12, lV13, lV23;  /* lengths of vectors */
  VERTEX3D intP[4];     /* intersection points */
  R8 eps, eps1;  /* rounding factors */
  IX nip=0; /* number of intersection points */
  IX nVrtN; /* number of vertices - surface N */
  IX nVrtM; /* number of vertices - surface M */
  IX n, m;  /* vertex indices on surfaces N and M */

  if(srfN->rc > srfM->rc) {
    eps = 1.0e-6f * srfN->rc;
  } else {
    eps = 1.0e-6f * srfM->rc;
  }
  eps1 = 1.0f + eps;
  pv12 = &v12; pv13 = &v13; pv23 = &v23;
  nVrtN = srfN->nv; nVrtM = srfM->nv;
#ifdef DEBUG
  fprintf(_ulog, "IntersectionTest: %d %d\n", srfN->nr, srfM->nr);
  DumpP3D("srfN", nVrtN, srfN->v);
  DumpP3D("srfM", nVrtM, srfM->v);
#endif

  p1 = srfN->v+nVrtN-1;
  for(n=0; n<nVrtN && nip<2; n++) { /* check vertices of surface M */
                                    /* against edges of surface N. */
    p2 = srfN->v+n;
    VECTOR(p1, p2, pv12);
    lV12 = VLEN(pv12);
    for(m=0; m<nVrtM; m++) {
      p3 = srfM->v+m;
      VECTOR(p1, p3, pv13);
      lV13 = VLEN(pv13);
      VECTOR(p2, p3, pv23);
      lV23 = VLEN(pv23);
      if(lV13 + lV23 < eps1 * lV12) {
        IX j;
        for(j=0; j<nip; j++) {
          if(fabs(intP[j].x - p3->x) < eps &&
             fabs(intP[j].y - p3->y) < eps &&
             fabs(intP[j].z - p3->z) < eps) {
            break;
          }
        }
        if(j==nip) {    /* P3 different from previous points */
          memcpy(intP+nip++, p3, sizeof(VERTEX3D));
        }
      }
    }
    p1 = p2;
  }

  p1 = srfM->v+nVrtM-1;
  for(m=0; m<nVrtM && nip<2; m++) { /* check vertices of surface N */
                                    /* against edges of surface M. */
    p2 = srfM->v+m;
    VECTOR(p1, p2, pv12);
    lV12 = VLEN(pv12);
    for(n=0; n<nVrtN; n++) {
      p3 = srfN->v+n;
      VECTOR(p1, p3, pv13);
      lV13 = VLEN(pv13);
      VECTOR(p2, p3, pv23);
      lV23 = VLEN(pv23);
      if(lV13 + lV23 < eps1 * lV12) {
        IX j;
        for(j=0; j<nip; j++) {
          if(fabs(intP[j].x - p3->x) < eps &&
             fabs(intP[j].y - p3->y) < eps &&
             fabs(intP[j].z - p3->z) < eps) {
            break;
          }
        }
        if(j==nip) {
          memcpy(intP+nip++, p3, sizeof(VERTEX3D));
        }
      }
    }
    p1 = p2;
  }

  if(nip>1) {
    error(1, __FILE__, __LINE__, "Surfaces may intersect in IntersectionTest");
#ifdef DEBUG
    sprintf(_string, "Surface %d:", srfN->nr);
    DumpP3D(_string, nVrtN, srfN->v);
    sprintf(_string, "Surface %d:", srfM->nr);
    DumpP3D(_string, nVrtM, srfM->v);
    fprintf(_ulog, "Intersection:\n");
    for(n=0; n<nip; n++) {
      fprintf(_ulog, "  %d %12.7f %12.7f %12.7f\n",
              n, intP[n].x, intP[n].y, intP[n].z);
    }
#endif
  }

}  /* end IntersectionTest */

/***  DumpOS.c  **************************************************************/

/*  Dump list of view obsrtucting surfaces  */

void DumpOS(I1 *title, const IX nos, IX *listObstr)
/* nos;  number of surfaces
 * listObstr;  list of obstruction surface numbers */
{
  IX  n;

  fprintf(_ulog, "%s", title);
  if(strlen(title) + 6*nos > 78) {
    fprintf(_ulog, "\n");
  }
  for(n=1; n<=nos; n++) {
    fprintf(_ulog, " %5d", listObstr[n]);
    if(n%10 == 0) {
      fprintf(_ulog, "\n");
    }
  }
  if(nos%10 != 0) {
    fprintf(_ulog, "\n");
  }
  fflush(_ulog);

}  /*  end of DumpOS  */

/***  SetPosObstr3D.c  *******************************************************/

/*  Set list of possible view obstructing surfaces.
 *  Return number of possible view obstructing surfaces.  */

IX SetPosObstr3D(IX nSrf, SRFDAT3D *srf, IX *possibleObstr)
/* nSrf;  total number of surfaces (RSRF and OBSO)
 * srf;   vector of surface data [1:nSrf]
 * possibleObstr;  vector of possible view obtructions [1:nSrf]
 */
{
  IX ns;       /* surface number */
  IX n;        /* surface number */
  IX j;        /* vertex number */
  IX infront;  /* true if a vertex is in front of surface ns */
  IX behind;   /* true if a vertex is behind surface ns */
  R8 dot, eps; /* dot product and test value */
  IX npos=0;   /* number of possible view obstructing surfaces */

  for(ns=nSrf; ns; ns--) { /* reverse order for obstruction surfaces first */
    if(srf[ns].type != RSRF && srf[ns].type != OBSO) {
      continue;
    }
    eps = 1.0e-5f * srf[ns].rc;
    infront = behind = 0;
    for(n=nSrf; n; n--) {
      if(srf[ns].type != RSRF && srf[ns].type != OBSO) {
        continue;
      }
      for(j=0; j<srf[n].nv; j++) {
        dot = VDOTW((srf[n].v[j]), (&srf[ns].dc));
        if(dot > eps) {
          infront = 1;
        }
        if(dot < -eps) {
          behind = 1;
        }
      }
      if(infront && behind) {
        break;
      }
    }  /* end n loop */

    if(n) {                   /* some vertices in front and some behind */
                              /* surface ns ==> ns may obstruct a view. */
      npos += 1;
      for(n=npos; n>1; n--) { /* insertion sort: largest surfaces first */
        if(srf[possibleObstr[n-1]].area < srf[ns].area) {
          possibleObstr[n] = possibleObstr[n-1];
        } else {
          break;
        }
      }
      possibleObstr[n] = ns;
    }
  }

  return npos;

}  /*  end of SetPosObstr3D  */

#ifdef XXX
/***  CylinderRadiusTest.c  **************************************************/

/*  Cylinder radius test to reduce the list of possible obstructing surfaces */

IX CylinderRadiusTest(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM,
                      IX *possibleObstr, R8 distNM, IX nPossObstr)
/* srf  - data for all surfaces.
 * srfN - data for surface N.
 * srfM - data for surface M.
 * possibleObstr  - list of possible obstructing surfaces (output).
 * distNM  - distance between centroids of N and M.
 * nPossObsrt  - number of possible obstructing surfaces
 */
{
  DIRCOS dcNM; /* direction cosines of line between srfN and srfM */
  VECTOR3D a, b; /* vectors */
  R8 radCylndr; /* radius of cylinder */
  R8 d;
  IX i, k;     /* surface number */
  IX nPoss=0;  /* reduced number of possible obstructing surfaces */

#ifdef DEBUG
  fprintf(_ulog, "CylinderRadiusTest: %d\n", nPossObstr);
#endif

  d = 1.0 / distNM;
  VECTOR((&srfN->ctd), (&srfM->ctd), (&a));
  VSCALE(d, (&a), (&dcNM));

  radCylndr = MAX(srfN->rc, srfM->rc);
#ifdef DEBUG
  fprintf(_ulog, "Cylinder: %f\n", radCylndr);
#endif

  /* process possible view obstructing surfaces */
  for(i=1; i<=nPossObstr; i++) {
    k = possibleObstr[i];
#ifdef DEBUG
    fprintf(_ulog, "K: %d\n", k);
#endif
    VECTOR((&srfN->ctd), (&srf[k].ctd), (&a));
    VCROSS((&dcNM), (&a), (&b));
    d = radCylndr + srf[k].rc;
    if(VDOT((&b),(&b)) > (d*d)) {
      continue;
    }
#ifdef DEBUG
    fprintf(_ulog, "Passed radius test\n");
#endif
    /* K may be an obstruction */
    possibleObstr[++nPoss] = k;
  }

#ifdef DEBUG
  if(nPoss && _list>3) {
    DumpOS("CylinderRadiusTest LOS:", nPoss, possibleObstr);
  }
#endif

  return nPoss;

}  /*  end of CylinderRadiusTest  */

#endif /*XXX*/
