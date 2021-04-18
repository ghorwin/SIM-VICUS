/*subfile:  ctrans.c  *********************************************************/
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
#include <math.h>   /* prototypes: fabs, sqrt */
#include <float.h>  /* define: FLT_EPSILON */
#include "types.h" 
#include "view3d.h"
#include "prtyp.h" 

extern FILE *_ulog; /* log file */

/***  CTIdent.c  *************************************************************/

/*  Set up 4 X 4 identity matrix [T].  Contiguous storage.  */

void CTIdent(R8 t[4][4])
{
  IX j;

  memset((void *)t, 0, 16*sizeof(R8));
  for(j=0; j<4; j++) {
    t[j][j] = 1.0;
  }
}  /* end CTIdent */

/***  CTScale.c  *************************************************************/

/*  Add scaling S to transformation matrix [T].
 *  The scaling transformation is:
 *         |  s  0  0  0  |   | t00 t01 t02 t03 |
 *  [T'] = |  0  s  0  0  | * | t10 t11 t12 t13 |
 *         |  0  0  s  0  |   | t20 t21 t22 t23 |
 *         |  0  0  0  1  |   | t30 t31 t32 t33 |
 */

void CTScale(const R8 s, R8 t[4][4])
{
  IX i,j;

  for(i=0; i<3; i++) {
    for(j=0; j<4; j++) {
      t[i][j] *= s;
    }
  }

}  /* end of CTScale */

/***  CTShift.c  *************************************************************/

/*  Add shift {S} to transformation matrix [T]. That is, point {S} in the 
 *  original coordinate system becomes the origin in the new coordinate system.
 *  The shift transformation matrix is:
 *         |  1  0  0 -sx  |   | t00 t01 t02 t03 |
 *  [T'] = |  0  1  0 -sy  | * | t10 t11 t12 t13 |
 *         |  0  0  1 -sz  |   | t20 t21 t22 t23 |
 *         |  0  0  0  1   |   | t30 t31 t32 t33 |
 */

void CTShift(const VERTEX3D *s, R8 t[4][4])
{
  IX j;

  for(j=0; j<4; j++) {
    t[0][j] -= s->x * t[3][j];
  }
  for(j=0; j<4; j++) {
    t[1][j] -= s->y * t[3][j];
  }
  for(j=0; j<4; j++) {
    t[2][j] -= s->z * t[3][j];
  }

}  /* end of CTShift */

/***  CTRotateX.c  ***********************************************************/

/*  Add rotation around X-axis to transformation matrix [T].
 *  The X-rotation transformation is:
 *         |  1   0  0  0  |   | t00 t01 t02 t03 |
 *  [T'] = |  0  ca -sa 0  | * | t10 t11 t12 t13 |
 *         |  0  sa  ca 0  |   | t20 t21 t22 t23 |
 *         |  0   0  0  1  |   | t30 t31 t32 t33 |
 */

void CTRotateX(const R8 cosAngle, const R8 sinAngle, R8 t[4][4])
{
  IX j;

#ifdef DEBUG
  if(cosAngle*cosAngle + sinAngle*sinAngle - 1.0 > 2.0*FLT_EPSILON) {
    error(2, __FILE__, __LINE__, "Invalid angle in CTRotateX");
  }
#endif

  for(j=0; j<4; j++) {
    R8 tmp = t[1][j];
    t[1][j] = cosAngle * tmp - sinAngle * t[2][j];
    t[2][j] = sinAngle * tmp + cosAngle * t[2][j];
  }

}  /* end of CTRotateX */

/***  CTRotateZ.c  ***********************************************************/

/*  Add rotation around Z-axis to transformation matrix [T].
 *  The Z-rotation transformation is:
 *         | ca -sa 0  0  |   | t00 t01 t02 t03 |
 *  [T'] = | sa  ca 0  0  | * | t10 t11 t12 t13 |
 *         |  0  0  1  0  |   | t20 t21 t22 t23 |
 *         |  0  0  0  1  |   | t30 t31 t32 t33 |
 */

void CTRotateZ(const R8 cosAngle, const R8 sinAngle, R8 t[4][4])
{
  IX j;

#ifdef DEBUG
  if(cosAngle*cosAngle + sinAngle*sinAngle-1.0 > 2.0*FLT_EPSILON) {
    error(2, __FILE__, __LINE__, "Invalid angle in CTRotateZ");
  }
#endif

  for(j=0; j<4; j++)
  {
    R8 tmp = t[0][j];
    t[0][j] = cosAngle * tmp - sinAngle * t[1][j];
    t[1][j] = sinAngle * tmp + cosAngle * t[1][j];
  }

}  /* end of CTRotateZ */

/***  CTRotateU.c  ***********************************************************/
 
/*  Add double rotation to transformation matrix [T] so as to view the origin 
 *  from direction U.  This is done by a Z-rotation followed by an X-rotation.
 */

void CTRotateU(const DIRCOS *u, R8 t[4][4])
{
  R8 v;

#ifdef DEBUG
  if(fabs(sqrt(u->x*u->x + u->y*u->y + u->z*u->z)-1.0) > 2.0*FLT_EPSILON) {
    error(2, __FILE__, __LINE__, "Invalid angle in CTRotateU");
  }
#endif

  v = sqrt(u->x * u->x + u->y * u->y);
  if(v > FLT_EPSILON) {
    CTRotateZ(-u->y / v, -u->x / v, t);
  }
  CTRotateX(+u->z, -v, t);

}  /* end of CTRotateU */

/***  CT3D.c  ****************************************************************/

/*  Coordinate transformation of 3D vertices.
 *  NV vertices/coordinates are stored contiguously:
 *     [x0][y0][z0][x1][y1][z1][x2] ...
 *  Transform:  {q} = [T]{p}.
 *     | q0 |   | t00 t01 t02 t03 |   | p0 |  (x coordinate)
 *     | q1 | = | t10 t11 t12 t13 | * | p1 |  (y)
 *     | q2 |   | t20 t21 t22 t23 |   | p2 |  (z)
 *     | 1. |   | 0.0 0.0 0.0 1.0 |   | 1. | 
 *  (Constants in row 4 are assumed; i.e., no perspective transformation.)
 */
/* const removal: const R8 t[4][4] */
void CT3D(const IX nv, R8 t[4][4], const VECTOR3D *p, VECTOR3D *q)
{
  IX n;

  for(n=nv; n; n--,p++,q++) {
    q->x = t[0][0] * p->x + t[0][1] * p->y + t[0][2] * p->z + t[0][3];
    q->y = t[1][0] * p->x + t[1][1] * p->y + t[1][2] * p->z + t[1][3];
    q->z = t[2][0] * p->x + t[2][1] * p->y + t[2][2] * p->z + t[2][3];
  }

}  /* end of CT3D */

/***  CoordTrans3D.c  ********************************************************/

/*  Coordinate transformation so that surface 2 is centered 
 *  in the Z=0 plane and fits within the -1<x<1 and -1<y<1 square.  */

void CoordTrans3D(SRFDAT3D *srf, SRFDATNM *srf1, SRFDATNM *srf2,
                  IX *probableObstr, VFCTRL *vfCtrl)
/* srf  - data for all surfaces.
 * xyz  - coordinates of vertices.
 * srf1 - data for surface 1.
 * srf2 - data for surface 2.
 * probableObstr  - list of probable obstructing surfaces.
 * srfT - data for transformed surfaces.
 * scale - coordinate scaling factor.
 */
{
  VERTEX3D vs[MAXNV1]; /* temporary vertices */
  SRFDAT3X *srf1T;  /* pointer to surface 1 */
  SRFDAT3X *srf2T;  /* pointer to surface 2 */
  SRFDAT3X *srfOT;  /* pointer to obstrucing surface */
  SRFDAT3D *ps;     /* pointer to original surface */
  R8 z[MAXNV1];  /* Z coordinates */
  R8 a[4][4]; /* coordinates transformation matrix */
  R8 b[4][4]; /* coordinate rotation matrix */
  R8 scale; /* coordinate scaling factor */
  R8 zmax;  /* Z coordinate of highest vertex */
  R8 eps=-1.0e-6f;
  IX clip;  /* if true, clip portion of surface below Z=0 plane */
  IX nv;    /* number of vertices */
  IX j, n;

  scale = 1.0f / srf2->rc;   /* distance scaling factor */
#ifdef DEBUG
  fprintf(_ulog, "CoordTrans3D:  %f\n", scale);
  DumpVA(" dc ", 1, 3, &srf2->dc.x);
  DumpVA(" ctd", 1, 3, &srf2->ctd.x);
#endif
  /* set up coordinate transformations */
  CTIdent(b);
  CTRotateU((void *)&srf2->dc, b);
  CTIdent(a);
  CTShift((void *)&srf2->ctd, a);
  CTRotateU((void *)&srf2->dc, a);
  CTScale(scale, a);
#ifdef DEBUG
  DumpVA(" A", 3, 4, a[0]);
  DumpVA(" B", 3, 4, b[0]);
#endif

  /* transform surface 1 */
  srf1T = &vfCtrl->srf1T;
  CT3D(srf1->nv, a, srf1->v, srf1T->v); /* vertex */
  CT3D(1, b, (void *)&srf1->dc, (void *)&srf1T->dc);  /* direction cosines */
  CT3D(1, a, &srf1->ctd, &srf1T->ctd);  /* centroid */
  srf1T->area = srf1->area * scale * scale;
  srf1T->shape = srf1->shape;
  srf1T->nr = srf1->nr;
  srf1T->nv = srf1->nv;

  /* transform surface 2 */
  srf2T = &vfCtrl->srf2T;
  CT3D(srf2->nv, a, srf2->v, srf2T->v);
  CT3D(1, b, (void *)&srf2->dc, (void *)&srf2T->dc);
  CT3D(1, a, &srf2->ctd, &srf2T->ctd);
  srf2T->area = srf2->area * scale * scale;
  srf2T->shape = srf2->shape;
  srf2T->nr = srf2->nr;
  srf2T->nv = srf2->nv;
#ifdef DEBUGXXX
  srf1T->dc.w = srf2T->dc.w = 0.0;
  srf1T->ztmax = srf2T->ztmax = 0.0;
#endif

  /* transform obstruction surfaces */
  srfOT = vfCtrl->srfOT;
  for(j=1; j<=vfCtrl->nProbObstr; j++,srfOT++) {
    ps = srf + probableObstr[j];
    srfOT->nr = ps->nr;
    nv = srfOT->nv = ps->nv;
    for(n=0; n<nv; n++) {
      CT3D(1, a, ps->v[n], srfOT->v+n);  /* vertices */
    }
    CT3D(1, b, (void *)&ps->dc, (void *)&srfOT->dc);  /* dir. cosines */
    CT3D(1, a, &ps->ctd, &srfOT->ctd);  /* centroid */
    srfOT->dc.w = -VDOT((&srfOT->dc), (&srfOT->ctd));
    zmax = 0.0;   /* zmax and clipping calculations */
    clip = 0;
    for(n=0; n<nv; n++) {
      z[n] = srfOT->v[n].z;
      if(z[n] > zmax) {
        zmax = z[n];
      }
      if(z[n] < 0.0)
      {
        if(z[n] < eps) {    /* round-off errors for co-planar srfs */
          clip = 1;
        } else {
          z[n]=0.0;
        }
      }
    }
    srfOT->ztmax = zmax;
    /* This may never be needed for plane polygons */
    if(clip) {
#ifdef DEBUG
      fprintf(_ulog, " Clipping obstruction surface %d\n", srfOT->nr);
#endif
      memcpy(vs, srfOT->v, nv*sizeof(VERTEX3D));
      srfOT->nv = ClipPolygon(1, nv, vs, z, srfOT->v);
    }
  }

#ifdef DEBUG
  Dump3X("Surface 1", srf1T);
  Dump3X("Surface 2", srf2T);
  srfOT = vfCtrl->srfOT;
  for(j=0; j<vfCtrl->nProbObstr; j++,srfOT++) {
    Dump3X("Obstruction", srfOT);
  }
  fflush(_ulog);
#endif

}  /*  end of CoordTrans3D  */

/***  DumpSrf3D.c  ***********************************************************/

/*  Dump SRFDAT3D structure.  */

void DumpSrf3D(I1 *title, SRFDAT3D *srf)
{
  IX n;
  fprintf(_ulog, "%s:  %d  area %.3e\n", title, srf->nr, srf->area);
  DumpVA(" ctd", 1, 3, &srf->ctd.x);
  DumpVA("  dc", 1, 4, &srf->dc.x);
  for(n=0; n<srf->nv; n++) {
    DumpVA("  vt", 1, 3, &srf->v[n]->x);
  }

}  /* end DumpSrf3D */

/***  DumpSrfNM.c  ***********************************************************/

/*  Dump SRFDATNM structure.  */

void DumpSrfNM(I1 *title, SRFDATNM *srf)
{
  IX n;
  fprintf(_ulog, "%s:  %d  area %.3e\n", title, srf->nr, srf->area);
  DumpVA(" ctd", 1, 3, &srf->ctd.x);
  DumpVA("  dc", 1, 4, &srf->dc.x);
  for(n=0; n<srf->nv; n++) {
    DumpVA("  vt", 1, 3, &srf->v[n].x);
  }

}  /* end DumpSrfNM */

/***  Dump3X.c  **************************************************************/

/*  Dump SRFDAT3X structure.  */

void Dump3X(I1 *title, SRFDAT3X *srfT)
{
  IX n;
  fprintf(_ulog, "%s:  %d  %.3e  %f\n", title, srfT->nr, srfT->area,
          srfT->ztmax);
  DumpVA(" dct", 1, 4, &srfT->dc.x);
  DumpVA(" ctd", 1, 3, &srfT->ctd.x);
  for(n=0; n<srfT->nv; n++) {
    DumpVA("  vt", 1, 3, &srfT->v[n].x);
  }

}  /* end Dump3X */

/***  DumpVA.c  **************************************************************/

/*  Dump a vector {A} or array [A] (R8 values) to file _ulog.
 *  A vector has only one row.
 */

void DumpVA(I1 *title, const IX rows, const IX cols, R8 *a)
{
  IX i, j, n;

  fprintf(_ulog, "%s:", title);
  if(rows>1) {
    fprintf(_ulog, "\n");
  }
  for(j=0; j<rows; j++)
  {
    if(rows>1) {
      fprintf(_ulog, "%5d", j);
    }
    for(n=0,i=cols; i; i--) {
      fprintf(_ulog, " %15.8f", *a++);
      if(++n == 5) {
        fprintf(_ulog, "\n     ");
        n = 0;
      }
    }
    if(n) {
      fprintf(_ulog, "\n");
    }
  }
  fflush(_ulog);

}  /* end of DumpVA */

