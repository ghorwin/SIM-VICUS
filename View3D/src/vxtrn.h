/*subfile:  vxtrn.h  **********************************************************/
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

/*  global variables for VIEWCHK  */

extern FILE *_ulog; /* log file */
extern FILE *_unxt; /* input file */
//extern IX _echo;    /* true = echo input file */
extern I1 _vdrive[_MAX_DRIVE];   /* drive letter for program ViewGrX.exe */
extern I1 _vdir[_MAX_DIR];       /* directory path for program ViewGrX.exe */
extern I1 _file_name[_MAX_PATH]; /* temporary file name */
extern I1 _string[LINELEN];  /* buffer for a character string */

extern IX _ntot;    /* total number of surfaces */
extern IX _nsrf;    /* number of radiating surfaces */
extern IX _nobs;    /* number of obstruction surfaces */
extern IX _nvrt;    /* number of vertices */

extern IX _encl;    /* true = surfaces form an enclosure */
extern IX _list;    /* output control, higher value = more output */

extern I1 *_pb;     /* memory block for polygon descriptions */
extern POLY *_nfpd; /* pointer to next free polygon descripton */
extern POLY *_mrdp; /* most recently defined polygon */
extern HCVE *_nfve; /* pointer to next free vertex */
extern R4 _epsarea; /* minimum surface area */

extern R4 _xmin, _xmax, _ymin, _ymax;  /* limits of transformed vertices */

extern IX _nAllSrf;       /* total number of surfaces */
extern IX _nRadSrf;       /* number of radiating surfaces; 
                             initially includes mask & null surfaces */
extern IX _nMaskSrf;      /* number of mask & null surfaces */
extern IX _nObstrSrf;     /* number of obstruction surfaces */
extern IX _nVertices;     /* number of vertices */
extern IX _format;        /* geometry format: 3 or 4 */
extern IX _outFormat;     /* output file format */
extern IX _doRow;         /* row to solve; 0 = all rows */
extern IX _doCol;         /* column to solve; 0 = all columns */
extern IX _enclosure;     /* 1 = surfaces form an enclosure */
extern IX _emittances;    /* 1 = process emittances */
extern IX _nPossObstr;    /* number of possible view obstructing surfaces */
extern IX _nProbObstr;    /* number of probable view obstructing surfaces */
extern IX _prjReverse;    /* projection control; 0 = normal, 1 = reverse */
extern R8 _epsAdap;       /* convergence for adaptive integration */
extern R8 _rcRatio;       /* rRatio of surface radii */
extern R8 _relSep;        /* surface separation / sum of radii */
extern IX _method;        /* 0 = 2AI, 1 = 1AI, 2 = 2LI, 3 = 1LI, 4 = ALI */
extern IX _nEdgeDiv;      /* number of edge divisions */
extern IX _maxRecursALI;  /* max number of ALI recursion levels */
extern U4 _usedV1LIadapt; /* number of V1LIadapt() calculations used */
extern IX _failViewALI;   /* 1 = unobstructed view factor did not converge */
extern IX _maxRecursion;  /* maximum number of recursion levels */
extern IX _minRecursion;  /* minimum number of recursion levels */
extern IX _failRecursion; /* 1 = obstructed view factor did not converge */
extern R8 _epsAF;         /* convergence for current AF calculation */
extern U4 _wastedVObs;    /* number of ViewObstructed() calculations wasted */
extern U4 _usedVObs;      /* number of ViewObstructed() calculations used */
extern U4 _totPoly;       /* total number of polygon view factors */
extern U4 _totVpt;        /* total number of view points */
extern IX _failConverge;  /* 1 if any calculation failed to converge */
extern SRFDAT3X _srf1T;   /* participating surface; transformed coordinates */
extern SRFDAT3X _srf2T;   /* participating surface; transformed coordinates;
                             view from srf1T toward srf2T. */ 
extern SRFDAT3X *_srfOT;  /* pointer to array of view obstrucing surfaces;
                             dimensioned from 0 to maxSrfT in View3d();
                             coordinates transformed relative to srf2T. */
