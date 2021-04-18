/*subfile:  polygn.c  *********************************************************/
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
/* The functions in this file maintain:
 *   a stack of free vertex/edge structures,
 *   a stack of free polygon structures, and
 *   one or more stacks of defined polygon structures.
 * Only one defined polygon stack may be created at a time.
 * However, multiple stacks may be saved by using external pointers. */

#include <stdio.h>
#include <string.h> /* prototype: memset, memcpy */
#include <math.h>   /* prototype: fabs */
#include <limits.h> /* define UINT_MAX */
#include "types.h" 
#include "view3d.h"
#include "prtyp.h"
#include "tmpstore.h"

extern FILE *_ulog; /* log file */
extern IX _maxNVT;  /* maximum number of temporary vertices */

IX TransferVrt(VERTEX2D *toVrt, const VERTEX2D *fromVrt, IX nFromVrt);

/* Both of the the block storage pointers should start as NULL */
TMPSTORE *_memPoly = NULL; /* memory block for polygon descriptions */
TMPSTORE *_memVertEdge = NULL; /* memory block for vertex/edge structures */
HCVE *_nextFreeVE; /* pointer to next free vertex/edge */
POLY *_nextFreePD; /* pointer to next free polygon descripton */
POLY *_nextUsedPD; /* pointer to top-of-stack used polygon */
IX _vertedgeCount = 0; /* count of total number vertex/edge structs requested */
IX _polyCount = 0; /* count of total number polygons requested */
R8 _epsDist;   /* minimum distance between vertices */
R8 _epsArea;   /* minimum surface area */
VERTEX2D *_leftVrt;  /* coordinates of vertices to left of edge */
VERTEX2D *_rightVrt; /* coordinates of vertices to right of edge */
VERTEX2D *_tempVrt;  /* coordinates of temporary polygon */
IX *_u=NULL;  /* +1 = vertex left of edge; -1 = vertex right of edge */

/*  Extensive use is made of 'homogeneous coordinates' (HC) which are not 
 *  familiar to most engineers.  The important properties of HC are 
 *  summarized below:
 *  A 2-D point (X,Y) is represented by a 3-element vector (w*X,w*Y,w), 
 *  where w may be any real number except 0.  A line is also represented 
 *  by a 3-element vector (a,b,c).  The directed line from point 
 *  (w*X1,w*Y1,w) to point (v*X2,v*Y2,v) is given by 
 *    (a,b,c) = (w*X1,w*Y1,w) cross (v*X2,v*Y2,v). 
 *  The sequence of the cross product is a convention to determine sign. 
 *  A point lies on a line if (a,b,c) dot (w*X,w*Y,w) = 0. 
 *  'Normalize' the representation of a point by setting w to 1.  Then 
 *  if (a,b,c) dot (X,Y,1) > 0.0, the point is to the left of the line. 
 *  If it is less than zero, the point is to the right of the line. 
 *  The point where two lines intersect is given by
 *    (w*X,w*Y,w) = (a1,b1,c1) cross (a2,b2,c2).
 *  Reference:  W.M. Newman and R.F. Sproull, "Principles of Interactive
 *  Computer Graphics", Appendix II, McGraw-Hill, 1973.  */

/***  PolygonOverlap.c  ******************************************************/

/*  Process two convex polygons.  Vertices are in clockwise sequence!!
 *
 *  There are three processing options -- savePD:
 *  (1)  Determine the portion of polygon P2 which is within polygon P1; 
 *       this may create one convex polygon.
 *  (3)  Determine the portion of polygon P2 which is outside polygon P1; 
 *       this may create one or more convex polygons.
 *  (2)  Do both (1) and (3).
 *
 *  If freeP2 is true, free P2 from its stack before exiting.
 *  Return 0 if P2 outside P1, 1 if P2 inside P1, 2 for partial overlap.
 */

IX PolygonOverlap(const POLY *p1, POLY *p2, const IX savePD, IX freeP2)
{
  POLY *pp;     /* pointer to polygon */
  POLY *initUsedPD;  /* initial top-of-stack pointer */
  HCVE *pv1;    /* pointer to edge of P1 */
  IX nLeftVrt;  /* number of vertices to left of edge */
  IX nRightVrt; /* number of vertices to right of edge */
  IX nTempVrt;  /* number of vertices of temporary polygon */
  //  VERTEX2D leftVrt[MAXNVT];  /* coordinates of vertices to left of edge */
  //  VERTEX2D rightVrt[MAXNVT]; /* coordinates of vertices to right of edge */
  //  VERTEX2D tempVrt[MAXNVT];  /* coordinates of temporary polygon */
  IX overlap=0; /* 0: P2 outside P1; 1: P2 inside P1; 2: part overlap */
  IX j, jm1;    /* vertex indices;  jm1 = j - 1 */

#ifdef DEBUG
  fprintf(_ulog, "PolygonOverlap:  P1 [%p]  P2 [%p]  flag %d\n",
          p1, p2, savePD);
#endif

  initUsedPD = _nextUsedPD;
  nTempVrt = GetPolygonVrt2D(p2, _tempVrt);

#ifdef DEBUG
  DumpP2D("P2:", nTempVrt, _tempVrt);
#endif

  pv1 = p1->firstVE;
  do {  /*  process tempVrt against each edge of P1 (long loop) */
    /*  transfer tempVrt into leftVrt and/or rightVrt  */
    R8 a1, b1, c1; /* HC for current edge of P1 */
    //    IX u[MAXNVT];  /* +1 = vertex left of edge; -1 = vertex right of edge */
    IX left=1;     /* true if all vertices left of edge */
    IX right=1;    /* true if all vertices right of edge */
#ifdef DEBUG
    fprintf(_ulog, "Test against edge of P1\nU:");
#endif

    /* compute and save u[j] - relations of vertices to edge */
    a1 = pv1->a;
    b1 = pv1->b;
    c1 = pv1->c;
    pv1 = pv1->next;
    for(j=0; j<nTempVrt; j++) {
      R8 dot = _tempVrt[j].x * a1 + _tempVrt[j].y * b1 + c1;
      if(dot > _epsArea) {
        _u[j] = 1;
        right = 0;
      } else if(dot < -_epsArea) {
        _u[j] = -1;
        left = 0;
      } else {
        _u[j] = 0;
      }
#ifdef DEBUG
      fprintf(_ulog, " %d", _u[j]);
#endif
    }
#ifdef DEBUG
    fprintf(_ulog, "\nQuick test:  right %d; left %d;\n", right, left);
#endif

    /* use quick tests to skip unnecessary calculations */
    if(right) {
      continue;
    }
    if(left) {
      goto p2_outside_p1;
    }

    /* check each vertex of tempVrt against current edge of P1 */
    jm1 = nTempVrt - 1;
    for(nLeftVrt=nRightVrt=j=0; j<nTempVrt; jm1=j++) {   /* short loop */
      if(_u[jm1]*_u[j] < 0) { /* vertices j-1 & j on opposite sides of edge */
                              /* compute intercept of edges */
        R8 a, b, c, w; /* HC intersection components */
        a = _tempVrt[jm1].y - _tempVrt[j].y;
        b = _tempVrt[j].x - _tempVrt[jm1].x;
        c = _tempVrt[j].y * _tempVrt[jm1].x - _tempVrt[jm1].y * _tempVrt[j].x;
        w = b * a1 - a * b1;
#ifdef DEBUG
        if(fabs(w) < _epsArea*(a+b+c)) {
          error(1, __FILE__, __LINE__, "small W in PolygonOverlap");
          DumpHC("P1:", p1, p1);
          DumpHC("P2:", p2, p2);
          fprintf(_ulog, "a, b, c, w: %g %g %g %g\n", a, b, c, w);
          fflush(_ulog);
          fprintf(_ulog, "x, y: %g %g\n", (c*b1-b*c1)/w, (a*c1-c*a1)/w);
        }
#endif
#ifdef DEBUG
        if(w == 0.0) {
          error(3, __FILE__, __LINE__,
                "Division by zero (w=0) in PolygonOverlap");
        }
#endif
        _rightVrt[nRightVrt].x = _leftVrt[nLeftVrt].x = (c*b1 - b*c1) / w;
        _rightVrt[nRightVrt++].y = _leftVrt[nLeftVrt++].y = (a*c1 - c*a1) / w;
      }
      if(_u[j] >= 0) {       /* vertex j is on or left of edge */
        _leftVrt[nLeftVrt].x = _tempVrt[j].x;
        _leftVrt[nLeftVrt++].y = _tempVrt[j].y;
      }
      if(_u[j] <= 0) {        /* vertex j is on or right of edge */
        _rightVrt[nRightVrt].x = _tempVrt[j].x;
        _rightVrt[nRightVrt++].y = _tempVrt[j].y;
      }
    }  /* end of short loop */

#ifdef DEBUG
    DumpP2D("Left polygon:", nLeftVrt, _leftVrt);
    DumpP2D("Right polygon:", nRightVrt, _rightVrt);
#endif
    //    if(nLeftVrt >= _maxNVT || nRightVrt >= _maxNVT)
    //      errorf(3, __FILE__, __LINE__, "Parameter _maxNVT too small");
    if(nLeftVrt >= _maxNVT) {
      error(2, __FILE__, __LINE__,
            "Parameter maxV (%d) too small in PolygonOverlap",_maxNVT);
      DumpP2D("Offending Polygon:", nLeftVrt, _leftVrt);
      overlap = -999;
      goto finish;
    }
    if(nRightVrt >= _maxNVT) {
      error(2, __FILE__, __LINE__,
            "Parameter maxV (%d) too small in PolygonOverlap",_maxNVT);
      DumpP2D("Offending Polygon:", nRightVrt, _rightVrt);
      overlap = -999;
      goto finish;
    }

    if(savePD > 1) { /* transfer left vertices to outside polygon */
      nTempVrt = TransferVrt(_tempVrt, _leftVrt, nLeftVrt);
#ifdef DEBUG
      DumpP2D("Outside polygon:", nTempVrt, _tempVrt);
#endif
      if(nTempVrt > 2) {
        SetPolygonHC(nTempVrt, _tempVrt, p2->trns);
        overlap = 1;
      }
    }

    /* transfer right side vertices to tempVrt */
    nTempVrt = TransferVrt(_tempVrt, _rightVrt, nRightVrt);
#ifdef DEBUG
    DumpP2D("Inside polygon:", nTempVrt, _tempVrt);
#endif
    if(nTempVrt < 2) { /* 2 instead of 3 allows degenerate P2; espArea = 0 */
      goto p2_outside_p1;
    }

  } while(pv1 != p1->firstVE);    /* end of P1 long loop */

  /* At this point tempVrt contains the overlap of P1 and P2. */

  if(savePD < 3) {   /* save the overlap polygon */
#ifdef DEBUG
    DumpP2D("Overlap polygon:", nTempVrt, _tempVrt);
#endif
    pp = SetPolygonHC(nTempVrt, _tempVrt, p2->trns * p1->trns);
    if(pp==NULL && savePD==2) {  /* overlap area too small */
      goto p2_outside_p1;
    }
  }
  overlap += 1;
  goto finish;

p2_outside_p1:     /* no overlap between P1 and P2 */
  overlap = 0;
#ifdef DEBUG
  fprintf(_ulog, "P2 outside P1\n");
#endif
  if(savePD > 1) {   /* save outside polygon - P2 */
    if(initUsedPD != _nextUsedPD) { /* remove previous outside polygons */
      FreePolygons(_nextUsedPD, initUsedPD);
    }

    if(freeP2) {        /* transfer P2 to new stack */
      pp = p2;
      freeP2 = 0;               /* P2 already freed */
    } else {                /* copy P2 to new stack */
      HCVE *pv, *pv2;
      pp = GetPolygonHC();      /* get cleared polygon data area */
      pp->area = p2->area;      /* copy P2 data */
      pp->trns = p2->trns;
      pv2 = p2->firstVE;
      do {
        if(pp->firstVE)
          pv = pv->next = GetVrtEdgeHC();
        else
          pv = pp->firstVE = GetVrtEdgeHC();
        memcpy(pv, pv2, sizeof(HCVE));   /* copy vertex/edge data */
        pv2 = pv2->next;
      } while(pv2 != p2->firstVE);
      pv->next = pp->firstVE;   /* complete circular list */
#ifdef DEBUG
      DumpHC("COPIED SURFACE:", pp, pp);
#endif
    }
    pp->next = initUsedPD;   /* link PP to stack */
    _nextUsedPD = pp;
  }

finish:
  if(freeP2) {  /* transfer P2 to free space */
    FreePolygons(p2, p2->next);
  }
  return overlap;

}  /* end of PolygonOverlap */

/***  TransferVrt.c  *********************************************************/

/*  Transfer vertices from polygon fromVrt to polygon toVrt eliminating nearly
 *  duplicate vertices.  Closeness of vertices determined by _epsDist.  
 *  Return number of vertices in polygon toVrt.  */

IX TransferVrt(VERTEX2D *toVrt, const VERTEX2D *fromVrt, IX nFromVrt)
{
  IX j;   /* index to vertex in polygon fromVrt */
  IX jm1; /* = j - 1 */
  IX n;   /* index to vertex in polygon toVrt */

  jm1 = nFromVrt - 1;
  for(n=j=0; j<nFromVrt; jm1=j++) {
    if(fabs(fromVrt[j].x - fromVrt[jm1].x) > _epsDist
       || fabs(fromVrt[j].y - fromVrt[jm1].y) > _epsDist) { /* transfer to toVrt */
      toVrt[n].x = fromVrt[j].x;
      toVrt[n++].y = fromVrt[j].y;
    } else if(n>0) {  /* close: average with prior toVrt vertex */
      toVrt[n-1].x = 0.5f * (toVrt[n-1].x + fromVrt[j].x);
      toVrt[n-1].y = 0.5f * (toVrt[n-1].y + fromVrt[j].y);
    } else {          /* (n=0) average with prior fromVrt vertex */
      toVrt[n].x = 0.5f * (fromVrt[jm1].x + fromVrt[j].x);
      toVrt[n++].y = 0.5f * (fromVrt[jm1].y + fromVrt[j].y);
      nFromVrt -= 1;  /* do not examine last vertex again */
    }
  }
  return n;

}  /* end TransferVrt */

/***  SetPolygonHC.c  ********************************************************/

/*  Set up polygon including homogeneous coordinates of edges.
 *  Return NULL if polygon area too small; otherwise return pointer to polygon.
 */

POLY *SetPolygonHC(const IX nVrt, const VERTEX2D *polyVrt, const R8 trns)
/* nVrt    - number of vertices (vertices in clockwise sequence);
 * polyVrt - X,Y coordinates of vertices (1st vertex not repeated at end),
             index from 0 to nVrt-1. */
{
  POLY *pp;    /* pointer to polygon */
  HCVE *pv;    /* pointer to HC vertices/edges */
  R8 area=0.0; /* polygon area */
  IX j, jm1;   /* vertex indices;  jm1 = j - 1 */

  pp = GetPolygonHC();      /* get cleared polygon data area */
#ifdef DEBUG
  fprintf(_ulog, " SetPolygonHC:  pp [%p]  nv %d\n", pp, nVrt);
#endif

  jm1 = nVrt - 1;
  for(j=0; j<nVrt; jm1=j++) { /* loop through vertices */
    if(pp->firstVE) {
      pv = pv->next = GetVrtEdgeHC();
    } else {
      pv = pp->firstVE = GetVrtEdgeHC();
    }
    pv->x = polyVrt[j].x;
    pv->y = polyVrt[j].y;
    pv->a = polyVrt[jm1].y - polyVrt[j].y; /* compute HC values */
    pv->b = polyVrt[j].x - polyVrt[jm1].x;
    pv->c = polyVrt[j].y * polyVrt[jm1].x - polyVrt[j].x * polyVrt[jm1].y;
    area -= pv->c;
  }
  pv->next = pp->firstVE;    /* complete circular list */

  pp->area = 0.5 * area;
  pp->trns = trns;
#ifdef DEBUG
  fprintf(_ulog, "  areas:  %f  %f,  trns:  %f\n",
          pp->area, _epsArea, pp->trns);
  fflush(_ulog);
#endif

  if(pp->area < _epsArea) { /* polygon too small to save */
    FreePolygons(pp, NULL);
    pp = NULL;
  } else {
    pp->next = _nextUsedPD;     /* link polygon to current list */
    _nextUsedPD = pp;           /* prepare for next linked polygon */
  }

  return pp;

}  /*  end of SetPolygonHC  */

/***  GetPolygonHC.c  ********************************************************/

/*  Return pointer to a cleared polygon structure.  
 *  This is taken from the list of unused structures if possible.  
 *  Otherwise, a new structure will be allocated.  */

POLY *GetPolygonHC(void)
{
  POLY *pp;  /* pointer to polygon structure */

  if(_nextFreePD) {
    pp = _nextFreePD;
    _nextFreePD = _nextFreePD->next;
    memset(pp, 0, sizeof(POLY));  /* clear pointers */
  } else {
    _polyCount++;
    pp = getMemory(_memPoly, __FILE__, __LINE__);
    // pp = Alc_EC(&_memPoly, sizeof(POLY), __FILE__, __LINE__);
  }
  return pp;

}  /* end GetPolygonHC */

/***  GetVrtEdgeHC.c  ********************************************************/

/*  Return pointer to an uncleared vertex/edge structure.  
 *  This is taken from the list of unused structures if possible.  
 *  Otherwise, a new structure will be allocated.  */

HCVE *GetVrtEdgeHC(void)
{
  HCVE *pv;  /* pointer to vertex/edge structure */

  if(_nextFreeVE) {
    pv = _nextFreeVE;
    _nextFreeVE = _nextFreeVE->next;
  } else {
    _vertedgeCount++;
    pv = getMemory(_memVertEdge, __FILE__, __LINE__);
    //pv = Alc_EC(&_memPoly, sizeof(HCVE), __FILE__, __LINE__);
  }

  return pv;

}  /* end GetVrtEdgeHC */

/***  FreePolygons.c  ********************************************************/

/*  Restore list of polygon descriptions to free space.  */

void FreePolygons(POLY *first, POLY *last)
/* first;  - pointer to first polygon in linked list (not NULL).
 * last;   - pointer to polygon AFTER last one freed (NULL = complete list). */
{
  POLY *pp; /* pointer to polygon */
  HCVE *pv; /* pointer to edge/vertex */

  for(pp=first; ; pp=pp->next) {
#ifdef DEBUG
    if(!pp) {
      error(3, __FILE__, __LINE__, "Polygon PP not defined in FreePolygons");
    }
    if(!pp->firstVE) {
      error(3, __FILE__, __LINE__, "FirstVE not defined in FreePolygons");
    }
#endif
    pv = pp->firstVE->next;           /* free vertices (circular list) */
    while(pv->next != pp->firstVE) {  /* find "end" of vertex list */
      pv = pv->next;
    }
    pv->next = _nextFreeVE;           /* reset vertex links */
    _nextFreeVE = pp->firstVE;
    if(pp->next == last) {
      break;
    }
  }
  pp->next = _nextFreePD;       /* reset polygon links */
  _nextFreePD = first;

}  /*  end of FreePolygons  */

/***  NewPolygonStack.c  *****************************************************/

/*  Start a new stack (linked list) of polygons.  */

void NewPolygonStack(void)
{
  _nextUsedPD = NULL;  /* define bottom of stack */
}  /* end NewPolygonStack */

/***  TopOfPolygonStack.c  ***************************************************/

/*  Return pointer to top of active polygon stack.  */

POLY *TopOfPolygonStack(void)
{
  return _nextUsedPD;
}  /* end TopOfPolygonStack */

/***  GetPolygonVrt2D.c  *****************************************************/

/*  Get polygon vertices.  Return number of vertices.
 *  Be sure polyVrt is sufficiently long.  */

IX GetPolygonVrt2D(const POLY *pp, VERTEX2D *polyVrt)
{
  HCVE *pv;    /* pointer to HC vertices/edges */
  IX j=0;      /* vertex counter */

  pv = pp->firstVE;
  do{
    polyVrt[j].x = pv->x;
    polyVrt[j++].y = pv->y;
    pv = pv->next;
  } while(pv != pp->firstVE);

  return j;

}  /*  end of GetPolygonVrt2D  */

/***  GetPolygonVrt3D.c  *****************************************************/

/*  Get polygon vertices.  Return number of vertices.
 *  Be sure polyVrt is sufficiently long.  */

IX GetPolygonVrt3D(const POLY *pp, VERTEX3D *polyVrt)
{
  HCVE *pv;    /* pointer to HC vertices/edges */
  IX j=0;      /* vertex counter */

  pv = pp->firstVE;
  do{
    polyVrt[j].x = pv->x;
    polyVrt[j].y = pv->y;
    polyVrt[j++].z = 0.0;
    pv = pv->next;
  } while(pv != pp->firstVE);

  return j;

}  /*  end of GetPolygonVrt3D  */

/***  FreeTmpVertMem.c  ******************************************************/

/*  Free vectors for temporary overlap vertices.  */

void FreeTmpVertMem(void)
{
  Fre_V(_u, 0, _maxNVT, sizeof(IX), __FILE__, __LINE__);
  Fre_V(_tempVrt, 0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
  Fre_V(_rightVrt, 0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
  Fre_V(_leftVrt, 0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
}  /*  end FreeTmpVertMem  */

/***  InitTmpVertMem.c  ******************************************************/

/*  Initialize vectors for temporary overlap vertices.  */

void InitTmpVertMem(void)
{
  if(_u) {
    error(3, __FILE__, __LINE__, "Temporary vertices already allocated");
  }
  _leftVrt = Alc_V(0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
  _rightVrt = Alc_V(0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
  _tempVrt = Alc_V(0, _maxNVT, sizeof(VERTEX2D), __FILE__, __LINE__);
  _u = Alc_V(0, _maxNVT, sizeof(IX), __FILE__, __LINE__);

}  /*  end InitTmpVertMem  */

/***  InitPolygonMem.c  ******************************************************/

/*  Initialize polygon processing memory and globals.  */

void InitPolygonMem(const R8 epsdist, const R8 epsarea)
{
  if(_memPoly) { /* clear existing polygon structures data */
    //_memPoly = Clr_EC(_memPoly); // This does not deallocate
    cleanStore(_memPoly, __FILE__, __LINE__); // This actually deallocates
  } else {  /* allocate polygon structures heap pointer */
    //_memPoly = Alc_ECI(8000, __FILE__, __LINE__);
    _memPoly = newStore(200, sizeof(POLY), __FILE__, __LINE__);
  }
  if(_memVertEdge) { /* clear existing vertex/edge structures data */
    cleanStore(_memVertEdge, __FILE__, __LINE__);
  } else { /* allocate vertex/edge structures heap pointer */
    _memVertEdge = newStore(200, sizeof(HCVE), __FILE__, __LINE__);
  }

  _epsDist = epsdist;
  _epsArea = epsarea;
  _nextFreeVE = NULL;
  _nextFreePD = NULL;
  _nextUsedPD = NULL;
#ifdef DEBUG
  fprintf(_ulog, "InitPolygonMem: epsDist %g epsArea %g\n",
          _epsDist, _epsArea);
#endif

}  /* end InitPolygonMem */

/***  FreePolygonMem.c  ******************************************************/

/*  Free polygon processing memory.  */

void FreePolygonMem(void)
{
  if(_memPoly){
    summarizeStore(_ulog, _memPoly, "Polygon Memory");
    deleteStore(_memPoly, __FILE__, __LINE__);
    _memPoly = NULL;
  }
  if(_memVertEdge) {
    summarizeStore(_ulog, _memVertEdge, "Vertex/Edge Memory");
    deleteStore(_memVertEdge, __FILE__, __LINE__);
    _memVertEdge = NULL;
  }
  //_memPoly = (I1 *)Fre_EC(_memPoly, __FILE__, __LINE__);
  _polyCount = 0;
  _vertedgeCount = 0;

}  /* end FreePolygonMem */

/***  LimitPolygon.c  ********************************************************/

/*  This function limits the polygon coordinates to a rectangle which encloses
 *  the base surface. Vertices may be in clockwise or counter-clockwise order.
 *  The polygon is checked against each edge of the window, with the portion
 *  inside the edge saved in the tempVrt or polyVrt array in turn.
 *  The code is long, but the loops are short.  
 *  Return the number of number of vertices in the clipped polygon
 *  and the clipped vertices in polyVrt.
 */

IX LimitPolygon(IX nVrt, VERTEX2D polyVrt[],
                const R8 maxX, const R8 minX, const R8 maxY, const R8 minY)
{
  //  VERTEX2D tempVrt[MAXNV2];  /* temporary vertices */
  IX n, m;  /* vertex index */

  /* test vertices against maxX */
  polyVrt[nVrt].x = polyVrt[0].x;
  polyVrt[nVrt].y = polyVrt[0].y;
  for(n=m=0; n<nVrt; n++) {
    if(polyVrt[n].x < maxX) {
      _tempVrt[m].x = polyVrt[n].x;
      _tempVrt[m++].y = polyVrt[n].y;
      if(polyVrt[n+1].x > maxX) {
        _tempVrt[m].x = maxX;
        _tempVrt[m++].y = polyVrt[n].y + (maxX - polyVrt[n].x)
            * (polyVrt[n+1].y - polyVrt[n].y) / (polyVrt[n+1].x - polyVrt[n].x);
      }
    } else if(polyVrt[n].x > maxX) {
      if (polyVrt[n+1].x < maxX) {
        _tempVrt[m].x = maxX;
        _tempVrt[m++].y = polyVrt[n].y + (maxX - polyVrt[n].x)
            * (polyVrt[n+1].y - polyVrt[n].y) / (polyVrt[n+1].x - polyVrt[n].x);
      }
    } else {
      _tempVrt[m].x = polyVrt[n].x;
      _tempVrt[m++].y = polyVrt[n].y;
    }
  }  /* end of maxX test */
  nVrt = m;
  if(nVrt < 3) {
    return 0;
  }
  /* test vertices against minX */
  _tempVrt[nVrt].x = _tempVrt[0].x;
  _tempVrt[nVrt].y = _tempVrt[0].y;
  for(n=m=0; n<nVrt; n++) {
    if(_tempVrt[n].x > minX) {
      polyVrt[m].x = _tempVrt[n].x;
      polyVrt[m++].y = _tempVrt[n].y;
      if(_tempVrt[n+1].x < minX) {
        polyVrt[m].x = minX;
        polyVrt[m++].y = _tempVrt[n].y + (minX - _tempVrt[n].x)
            * (_tempVrt[n+1].y - _tempVrt[n].y) / (_tempVrt[n+1].x - _tempVrt[n].x);
      }
    } else if(_tempVrt[n].x < minX) {
      if (_tempVrt[n+1].x > minX) {
        polyVrt[m].x = minX;
        polyVrt[m++].y = _tempVrt[n].y + (minX - _tempVrt[n].x)
            * (_tempVrt[n+1].y - _tempVrt[n].y) / (_tempVrt[n+1].x - _tempVrt[n].x);
      }
    } else {
      polyVrt[m].x = _tempVrt[n].x;
      polyVrt[m++].y = _tempVrt[n].y;
    }
  }  /* end of minX test */
  nVrt = m;
  if(nVrt < 3)
    return 0;
  /* test vertices against maxY */
  polyVrt[nVrt].y = polyVrt[0].y;
  polyVrt[nVrt].x = polyVrt[0].x;
  for(n=m=0; n<nVrt; n++) {
    if(polyVrt[n].y < maxY) {
      _tempVrt[m].y = polyVrt[n].y;
      _tempVrt[m++].x = polyVrt[n].x;
      if(polyVrt[n+1].y > maxY) {
        _tempVrt[m].y = maxY;
        _tempVrt[m++].x = polyVrt[n].x + (maxY - polyVrt[n].y)
            * (polyVrt[n+1].x - polyVrt[n].x) / (polyVrt[n+1].y - polyVrt[n].y);
      }
    }
    else if(polyVrt[n].y > maxY) {
      if (polyVrt[n+1].y < maxY) {
        _tempVrt[m].y = maxY;
        _tempVrt[m++].x = polyVrt[n].x + (maxY - polyVrt[n].y)
            * (polyVrt[n+1].x - polyVrt[n].x) / (polyVrt[n+1].y - polyVrt[n].y);
      }
    } else {
      _tempVrt[m].y = polyVrt[n].y;
      _tempVrt[m++].x = polyVrt[n].x;
    }
  }  /* end of maxY test */
  nVrt = m;
  if(nVrt < 3) {
    return 0;
  }
  /* test vertices against minY */
  _tempVrt[nVrt].y = _tempVrt[0].y;
  _tempVrt[nVrt].x = _tempVrt[0].x;
  for(n=m=0; n<nVrt; n++) {
    if(_tempVrt[n].y > minY) {
      polyVrt[m].y = _tempVrt[n].y;
      polyVrt[m++].x = _tempVrt[n].x;
      if(_tempVrt[n+1].y < minY) {
        polyVrt[m].y = minY;
        polyVrt[m++].x = _tempVrt[n].x + (minY - _tempVrt[n].y)
            * (_tempVrt[n+1].x - _tempVrt[n].x) / (_tempVrt[n+1].y - _tempVrt[n].y);
      }
    } else if(_tempVrt[n].y < minY) {
      if (_tempVrt[n+1].y > minY) {
        polyVrt[m].y = minY;
        polyVrt[m++].x = _tempVrt[n].x + (minY - _tempVrt[n].y)
            * (_tempVrt[n+1].x - _tempVrt[n].x) / (_tempVrt[n+1].y - _tempVrt[n].y);
      }
    } else {
      polyVrt[m].y = _tempVrt[n].y;
      polyVrt[m++].x = _tempVrt[n].x;
    }
  }  /* end of minY test */
  nVrt = m;
  if(nVrt < 3) {
    return 0;
  }
  return nVrt;    /* note: final results are in polyVrt */

}  /*  end of LimitPolygon  */

#ifdef DEBUG
/***  DumpHC.c  **************************************************************/

/*  Print the descriptions of sequential polygons.  */

void DumpHC(I1 *title, const POLY *pfp, const POLY *plp)
/*  pfp, plp; pointers to first and last (NULL acceptable) polygons  */
{
  const POLY *pp;
  HCVE *pv;
  IX i, j;

  fprintf(_ulog, "%s\n", title);
  for(i=0,pp=pfp; pp; pp=pp->next) {  /* polygon loop */
    fprintf(_ulog, " pd [%p]", pp);
    fprintf(_ulog, "  area %.4g", pp->area);
    fprintf(_ulog, "  trns %.3g", pp->trns);
    fprintf(_ulog, "  next [%p]", pp->next);
    fprintf(_ulog, "  fve [%p]\n", pp->firstVE);
    if(++i >= 100) {
      error(3, __FILE__, __LINE__, "Too many surfaces in DumpHC");
    }

    j = 0;
    pv = pp->firstVE;
    do{                  /* vertex/edge loop */
      fprintf(_ulog, "  ve [%p] %10.7f %10.7f %10.7f %10.7f %13.8f\n",
              pv, pv->x, pv->y, pv->a, pv->b, pv->c);
      pv = pv->next;
      if(++j >= _maxNVT) {
        error(3, __FILE__, __LINE__, "Too many vertices in DumpHC");
      }
    } while(pv != pp->firstVE);

    if(pp==plp) {
      break;
    }
  }
  fflush(_ulog);

}  /* end of DumpHC */

/*** DumpFreePolygons.c  *****************************************************/

void DumpFreePolygons(void)
{
  POLY *pp;

  fprintf(_ulog, "FREE POLYGONS:");
  for(pp=_nextFreePD; pp; pp=pp->next) {
    fprintf(_ulog, " [%p]", pp);
  }
  fprintf(_ulog, "\n");

}  /* end DumpFreePolygons */

/*** DumpFreeVertices.c  *****************************************************/

void DumpFreeVertices(void)
{
  HCVE *pv;

  fprintf(_ulog, "FREE VERTICES:");
  for(pv=_nextFreeVE; pv; pv=pv->next) {
    fprintf(_ulog, " [%p]", pv);
  }
  fprintf(_ulog, "\n");

}  /* end DumpFreeVertices */
#endif  /* end DEBUG */

/***  DumpP3D.c  *************************************************************/

/*  Dump 3-D polygon vertex data. */

void DumpP3D(I1 *title, const IX nvs, VERTEX3D *vs)
{
  IX n;

  fprintf(_ulog, "%s\n", title);
  fprintf(_ulog, " nvs: %d\tX\tY\tZ\n", nvs);
  for(n=0; n<nvs; n++) {
    fprintf(_ulog, "%4d\t%12.7f\t%12.7f\t%12.7f\n",
            n, vs[n].x, vs[n].y, vs[n].z);
  }
  fflush(_ulog);

}  /* end of DumpP3D */

/***  DumpP2D.c  *************************************************************/

/*  Dump 2-D polygon vertex data.  */

void DumpP2D(I1 *title, const IX nvs, VERTEX2D *vs)
{
  IX n;

  fprintf(_ulog, "%s\n", title);
  fprintf(_ulog, " nvs: %d\tX\tY\n", nvs);
  for(n=0; n<nvs; n++) {
    fprintf(_ulog, "%4d\t%12.7f\t%12.7f\n", n, vs[n].x, vs[n].y);
  }
  fflush(_ulog);

}  /* end of DumpP2D */

