/*subfile:  view3d.h **********************************************************/
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
/*  include file for the VIEW3D program */

#include <string.h> /* prototype: memcpy */

#define MAXNV 4     /* max number of vertices for an initial surface */
#define MAXNV1 5    /* max number of vertices after 1 clip */
#define MAXNV2 12   /* max number of vertices clipped projection */
//#define NAMELEN 12  /* length of a name */
#ifdef XXX
#define PI       3.141592653589793238
#define PIt2     6.283185307179586477   /* 2 * pi */
#define PIt4    12.566370614359172954   /* 4 * pi */
#define PId2     1.570796326794896619   /* pi / 2 */
#define PId4     0.785398163397448310   /* pi / 4 */
#define PIinv    0.318309886183790672   /* 1 / pi */
#define PIt2inv  0.159154943091895346   /* 1 / (2 * pi) */
#define PIt4inv  0.079577471545947673   /* 1 / (4 * pi) */
#endif
#define RTD 57.2957795  /* convert radians to degrees */
#define EPS 1.0e-6
#define EPS2 1.0e-12

typedef struct v2d {     /* structure for 2D vertex */
  R8  x;  /* X-coordinate */
  R8  y;  /* Y-coordinate */
} VERTEX2D;

typedef struct v3d {     /* structure for a 3D vertex or vector */

  R8  x;  /* X-coordinate */
  R8  y;  /* Y-coordinate */
  R8  z;  /* Z-coordinate */
} V3D;

#define VERTEX3D V3D
#define VECTOR3D V3D

/* vector macros using pointer to 3-element structures */

/*  VECTOR:  define vector C from vertex A to vextex B.  */
#define VECTOR(a,b,c)   \
    c->x = b->x - a->x; \
    c->y = b->y - a->y; \
    c->z = b->z - a->z; 

/*  VCOPY:  copy elements from vertex/vector A to B.  */
#define VCOPY(a,b)  { b->x = a->x; b->y = a->y; b->z = a->z; }

/*  VMID:  define vertex C midway between vertices A and B.  */
#define VMID(a,b,c)   \
    c->x = 0.5 * (a->x + b->x); \
    c->y = 0.5 * (a->y + b->y); \
    c->z = 0.5 * (a->z + b->z); 

/*  VDOT:  compute dot product of vectors A and B.  */
#define VDOT(a,b)   (a->x * b->x + a->y * b->y + a->z * b->z)

/*  VDOTW:  dot product of a vertex, V, and direction cosines, C.  */
#define VDOTW(v,c)  (c->w + VDOT(v,c))

/*  VLEN:  compute length of vector A.  */
#define VLEN(a)     sqrt( VDOT(a,a) )

/*  VCROSS:  compute vector C as cross product of A and B.  */
#define VCROSS(a,b,c)   \
    c->x = a->y * b->z - a->z * b->y; \
    c->y = a->z * b->x - a->x * b->z; \
    c->z = a->x * b->y - a->y * b->x; 

/*  VSCALE:  vector B = scalar C times vector A.  */
#define VSCALE(c,a,b)  \
    b->x = c * a->x; \
    b->y = c * a->y; \
    b->z = c * a->z; 

typedef struct dircos {       /* structure for direction cosines */
  R8  x;  /* X-direction cosine */
  R8  y;  /* Y-direction cosine */
  R8  z;  /* Z-direction cosine */
  R8  w;  /* distance from surface to origin (signed) */
} DIRCOS;

typedef struct srfdat3d {     /* structure for 3D surface data */
  IX nr;              /* surface number */
  IX nv;              /* number of vertices */
  IX shape;           /* 3 = triangle; 4 = parallelogram; 0 = other */
  IX type;            /* surface type data - defined below */
  R8 area;            /* area of surface */
  R8 rc;              /* radius enclosing the surface */
  DIRCOS dc;          /* direction cosines of surface normal */
  VERTEX3D ctd;       /* coordinates of centroid */
  VERTEX3D *v[MAXNV]; /* pointers to coordinates of up to MAXNV vertices */
  IX NrelS;           /* orientation of srf N relative to S:
                         -1: N behind S; +1: N in front of S;
                          0: part of N behind S, part in front */
  IX MrelS;           /* orientation of srf M relative to S */
} SRFDAT3D;

#define RSRF 0  /* normal surface */
#define SUBS 1  /* subsurface */
#define MASK 2  /* mask surface */
#define NULS 3  /* null surface */
#define OBSO 4  /* obstruction only surface */

typedef struct srfdatnm {     /* structure for 3D surface data */
  IX nr;              /* surface number */
  IX nv;              /* number of vertices */
  IX shape;           /* 3 = triangle; 4 = rectangle; 0 = other */
  IX buffer;          /* for structure alignment */
  R8 area;            /* area of surface */
  R8 rc;              /* radius enclosing the surface */
  DIRCOS dc;          /* direction cosines of surface normal */
  VERTEX3D ctd;       /* coordinates of centroid */
  VERTEX3D v[MAXNV1]; /* coordinates of vertices */
  R8 dist[MAXNV1];    /* distances of vertices above plane of other surface */
} SRFDATNM;

typedef struct srfdat3x {    /* structure for 3D surface data */
  IX nr;              /* surface number */
  IX nv;              /* number of vertices */
  IX shape;           /* 3 = triangle; 4 = rectangle; 0 = other */
  IX buffer;          /* for structure alignment */
  R8 area;            /* surface area  */
  R8 ztmax;           /* maximum Z-coordinate of surface */
  DIRCOS dc;          /* direction cosines of surface normal */
  VERTEX3D ctd;       /* coordinates of centroid */
  VERTEX3D v[MAXNV1]; /* coordinates of vertices */
} SRFDAT3X;

typedef struct edgedcs {   /* structure for direction cosines of polygon edge */
  R8  x;  /* X-direction cosine */
  R8  y;  /* Y-direction cosine */
  R8  z;  /* Z-direction cosine */
  R8  s;  /* length of edge */
} EDGEDCS;

typedef struct edgediv {   /* structure for Gaussian division of polygon edge */
  R8  x;  /* X-coordinate of element */
  R8  y;  /* Y-coordinate of element */
  R8  z;  /* Z-coordinate of element */
  R8  s;  /* length of element */
} EDGEDIV;

typedef struct {         /* view factor calculation control values */
  IX nAllSrf;       /* total number of surfaces */
  IX nRadSrf;       /* number of radiating surfaces;
                         initially includes mask & null surfaces */
  IX nMaskSrf;      /* number of mask & null surfaces */
  IX nObstrSrf;     /* number of obstruction surfaces */
  IX nVertices;     /* number of vertices */
  IX format;        /* geometry format: 3 or 4 */
  IX outFormat;     /* output file format */
  IX row;           /* row to solve; 0 = all rows */
  IX col;           /* column to solve; 0 = all columns */
  IX enclosure;     /* 1 = surfaces form an enclosure */
  IX emittances;    /* 1 = process emittances */
  IX nPossObstr;    /* number of possible view obstructing surfaces */
  IX nProbObstr;    /* number of probable view obstructing surfaces */
  IX prjReverse;    /* projection control; 0 = normal, 1 = reverse */
  R8 epsAdap;       /* convergence for adaptive integration */
  R8 rcRatio;       /* rRatio of surface radii */
  R8 relSep;        /* surface separation / sum of radii */
  IX method;        /* 0 = 2AI, 1 = 1AI, 2 = 2LI, 3 = 1LI, 4 = ALI */
  IX nEdgeDiv;      /* number of edge divisions */
  IX maxRecursALI;  /* max number of ALI recursion levels */
  U4 usedV1LIadapt; /* number of V1LIadapt() calculations used */
  IX failViewALI;   /* 1 = unobstructed view factor did not converge */
  IX maxRecursion;  /* maximum number of recursion levels */
  IX minRecursion;  /* minimum number of recursion levels */
  IX failRecursion; /* 1 = obstructed view factor did not converge */
  R8 epsAF;         /* convergence for current AF calculation */
  U4 wastedVObs;    /* number of ViewObstructed() calculations wasted */
  U4 usedVObs;      /* number of ViewObstructed() calculations used */
  U4 totPoly;       /* total number of polygon view factors */
  U4 totVpt;        /* total number of view points */
  IX failConverge;  /* 1 if any calculation failed to converge */
  SRFDAT3X srf1T;   /* participating surface; transformed coordinates */
  SRFDAT3X srf2T;   /* participating surface; transformed coordinates;
                       view from srf1T toward srf2T. */
  SRFDAT3X *srfOT;  /* pointer to array of view obstrucing surfaces;
                       dimensioned from 0 to maxSrfT in View3d();
                       coordinates transformed relative to srf2T. */
} VFCTRL;

#define UNK -1  /* unknown integration method */
#define DAI 0   /* double area integration */
#define SAI 1   /* single area integration */
#define DLI 2   /* double line integration */
#define SLI 3   /* single line integration */
#define ALI 4   /* adaptive line integration */

typedef struct hcve { /* homogeneous coordinate description of vertex/edge */
  struct hcve *next;  /* pointer to next vertex/edge */
  IX buffer;          /* for structure alignment */
  R8 x, y;    /* X and Y coordinates of the vertex */
  R8 a, b;    /* A, B */
  R8 c;       /*  & C homogeneous coordinates of the edge */
} HCVE;

typedef struct poly { /* description of a polygon */
  struct poly *next;  /* pointer to next polygon */
  HCVE *firstVE;      /* pointer to first vertex of polygon */
  R8 trns;            /* (0.0 <= transparency <= 1.0) */
  R8 area;            /* area of the polygon */
} POLY;

/* macros for simple mathematical operations */
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))   /* max of 2 values */
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))   /* min of 2 values */

/*  function prototypes.  */

/* input / output */
void CountVS3D(I1 *title, VFCTRL *vfCtrl);
void GetVS3D(I1 **name, R4 *emit, IX *base, IX *cmbn,SRFDAT3D *srf,
             VERTEX3D *xyz, VFCTRL *vfCtrl);
void GetVS3Da(I1 **name, R4 *emit, IX *base, IX *cmbn,SRFDAT3D *srf,
              VERTEX3D *xyz, VFCTRL *vfCtrl);
R8 VolPrism(VERTEX3D *a, VERTEX3D *b, VERTEX3D *c);
void SetPlane(SRFDAT3D *srf);
void ReportAF(const IX nSrf, const IX encl, const I1 *title, I1 ** name, 
              const R4 *area, const R4 *emit, const IX *base, R8 ** AF,
              IX flag);

/* 3-D view factor functions */
void View3D(SRFDAT3D *srf, const IX *base, IX *possibleObstr,R8 **AF,
            VFCTRL *vfCtrl);
IX ProjectionDirection(SRFDAT3D *srf, SRFDATNM *srfn, SRFDATNM *srfm,
                       IX *los, VFCTRL *vfCtrl);

R8 ViewUnobstructed(VFCTRL *vfCtrl, IX row, IX col);
R8 View2AI(const IX nss1, const DIRCOS *dc1, const VERTEX3D *pt1, 
           const R8 *area1, const IX nss2, const DIRCOS *dc2,
           const VERTEX3D *pt2, const R8 *area2);
R8 View2LI(const IX nd1, const IX nv1, const EDGEDCS *rc1, EDGEDIV **dv1,
           const IX nd2, const IX nv2, const EDGEDCS *rc2, EDGEDIV **dv2);
R8 View1LI(const IX nd1, const IX nv1, const EDGEDCS *rc1, EDGEDIV **dv1,
           const VERTEX3D *v1, const IX nv2, const VERTEX3D *v2);
R8 V1LIpart(const VERTEX3D *pp, const VERTEX3D *b0, const VERTEX3D *b1,
            const VECTOR3D *B, const R8 b2, IX *flag);
R8 V1LIxact(const VERTEX3D *a0, const VERTEX3D *a1, const R8 a, 
            const VERTEX3D *b0, const VERTEX3D *b1, const R8 b);
R8 V1LIadapt(VERTEX3D Pold[3], R8 dFold[3], R8 h, const VERTEX3D *b0,
const VERTEX3D *b1, const VECTOR3D *B, const R8 b2, IX level,
VFCTRL *vfCtrl);
R8 ViewALI(const IX nv1, const VERTEX3D *v1, const IX nv2, const VERTEX3D *v2,
           VFCTRL *vfCtrl);
void ViewsInit(IX maxDiv, IX init);
IX DivideEdges(IX nd, IX nv, VERTEX3D *vs, EDGEDCS *rc, EDGEDIV **dv);
IX GQParallelogram(const IX nDiv, const VERTEX3D *vp, VERTEX3D *p, R8 *w);
IX GQTriangle(const IX nDiv, const VERTEX3D *vt, VERTEX3D *p, R8 *w);
IX SubSrf(const IX nDiv, const IX nv, const VERTEX3D *v, const R8 area,
          VERTEX3D *pt, R8 *wt);

R8 ViewObstructed(VFCTRL *vfCtrl, IX nv1, VERTEX3D v1[], R8 area, IX nDiv);
R8 View1AI(IX nss, VERTEX3D *p1, R8 *area1, DIRCOS *dc1, SRFDAT3X *srf2);
R8 V1AIpart(const IX nv, const VERTEX3D p2[],const VERTEX3D *p1,
            const DIRCOS *u1);
IX Subsurface(SRFDAT3X *srf, SRFDAT3X sub[]);
R8 SetCentroid(const IX nv, VERTEX3D *vs, VERTEX3D *ctd);
R8 Triangle(VERTEX3D *p1, VERTEX3D *p2, VERTEX3D *p3, void *dc, IX dcflag);
void substs(IX n, VERTEX3D v[], VERTEX3D s[]);
R8 ViewTP(VERTEX3D v1[], R8 area, IX level, VFCTRL *vfCtrl);
R8 ViewRP(VERTEX3D v1[], R8 area, IX level, VFCTRL *vfCtrl);

/* 3-D view test functions */
IX AddMaskSrf(SRFDAT3D *srf, const SRFDATNM *srfN, const SRFDATNM *srfM,
              const IX *maskSrf, const IX *baseSrf, VFCTRL *vfCtrl, IX *los,
              IX nPoss);
IX BoxTest(SRFDAT3D *srf, SRFDATNM *srfn, SRFDATNM *srfm, VFCTRL *vfCtrl,
           IX *los, IX nProb);
IX ClipPolygon(const IX flag, const IX nv, VERTEX3D *v, R8 *dot, VERTEX3D *vc);
IX ConeRadiusTest(SRFDAT3D *srf, SRFDATNM *srfn, SRFDATNM *srfm,
                  VFCTRL *vfCtrl, IX *los, IX nProb, R8 distNM);
IX CylinderRadiusTest(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM, IX *los,
                      R8 distNM, IX nProb);
IX OrientationTest(SRFDAT3D *srf, SRFDATNM *srfn, SRFDATNM *srfm,
                   VFCTRL *vfCtrl, IX *los, IX nProb);
IX OrientationTestN(SRFDAT3D *srf, IX N, VFCTRL *vfCtrl, IX *possibleObstr,
                    IX nPossObstr);
void SelfObstructionClip(SRFDATNM *srfn);
IX SetShape(const IX nv, VERTEX3D *v, R8 *area);
IX SelfObstructionTest3D(SRFDAT3D *srf1, SRFDAT3D *srf2, SRFDATNM *srfn);
void IntersectionTest(SRFDATNM *srfn, SRFDATNM *srfm);
void DumpOS(I1 *title, const IX nos, IX *los);
IX SetPosObstr3D(IX nSrf, SRFDAT3D *srf, IX *lpos);

/* polygon processing */
IX PolygonOverlap(const POLY *p1, POLY *p2, const IX flagOP, IX freeP2);
void FreePolygons(POLY *first, POLY *last);
POLY *SetPolygonHC(const IX nVrt, const VERTEX2D *polyVrt, const R8 trns);
IX GetPolygonVrt2D(const POLY *pp, VERTEX2D *polyVrt);
IX GetPolygonVrt3D(const POLY *pp, VERTEX3D *srfVrt);
POLY *GetPolygonHC(void);
HCVE *GetVrtEdgeHC(void);
void NewPolygonStack(void);
POLY *TopOfPolygonStack(void);
void InitTmpVertMem(void);
void FreeTmpVertMem(void);
void InitPolygonMem(const R8 epsDist, const R8 epsArea);
void FreePolygonMem(void);
IX LimitPolygon(IX nVrt, VERTEX2D polyVrt[], const R8 maxX, const R8 minX, 
                const R8 maxY, const R8 minY);
void DumpHC(I1 *title, const POLY *pfp, const POLY *plp);
void DumpFreePolygons(void);
void DumpFreeVertices(void);
void DumpP2D(I1 *title, const IX nvs, VERTEX2D *vs);
void DumpP3D(I1 *title, const IX nvs, VERTEX3D *vs);

/* vector functions */
void CoordTrans3D(SRFDAT3D *srfAll, SRFDATNM *srf1, SRFDATNM *srf2,
                  IX *probableObstr, VFCTRL *vfCtrl);
void DumpSrf3D(I1 *title, SRFDAT3D *srf);
void DumpSrfNM(I1 *title, SRFDATNM *srf);
void Dump3X(I1 *tittle, SRFDAT3X *srfT);
void DumpVA(I1 *title, const IX rows, const IX cols, R8 *a);

/* post processing */
IX DelNull(const IX nSrf, SRFDAT3D *srf, IX *base, IX *cmbn, R4 *emit,
           R4 *area, I1 **name, R8 **AF);
void NormAF(const IX nSrf, const R4 *emit, const R4 *area, R8 **AF,
            const R8 eMax, const IX itMax);
IX Combine(const IX nSrf, const IX *cmbn, R4 *area, I1 **name, R8 **AF);
void Separate(const IX nSrf, const IX *base, R4 *area, R8 **AF);
void IntFac(const IX nSrf, const R4 *emit, const R4 *area, R8 **AF);
void LUFactorSymm(const IX neq, R8 **a);
void LUSolveSymm(const IX neq, R8 **a, R8 *b);
void DAXpY(const IX n, const R8 a, const R8 *x, R8 *y);
R8 DotProd(const IX n, const R8 *x, const R8 *y);

