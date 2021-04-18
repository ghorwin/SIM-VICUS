/*subfile:  vglob.h  *********************************************************/
/*  VGLOB:  global variables for VIEWCHK  */

FILE *_ulog=NULL; /* log file */
FILE *_unxt=NULL; /* input file */
IX _echo=0;       /* true = echo input file */
I1 _vdrive[_MAX_DRIVE];   /* drive letter for program ViewGrX.exe */
I1 _vdir[_MAX_DIR];       /* directory path for program ViewGrX.exe */
I1 _file_name[_MAX_PATH]; /* temporary file name */
I1 _string[LINELEN];  /* buffer for a character string */

IX _encl;    /* true = surfaces form an enclosure */
IX _list;    /* output control, higher value = more output */

IX _nAllSrf;       /* total number of surfaces */
IX _nRadSrf;       /* number of radiating surfaces; 
                      initially includes mask & null surfaces */
IX _nMaskSrf;      /* number of mask & null surfaces */
IX _nObstrSrf;     /* number of obstruction surfaces */
IX _nVertices;     /* number of vertices */
IX _format;        /* geometry format: 3 or 4 */
IX _outFormat;     /* output file format */
IX _doRow;           /* row to solve; 0 = all rows */
IX _doCol;           /* column to solve; 0 = all columns */
IX _enclosure;     /* 1 = surfaces form an enclosure */
IX _emittances;    /* 1 = process emittances */
IX _nPossObstr;    /* number of possible view obstructing surfaces */
IX _nProbObstr;    /* number of probable view obstructing surfaces */
IX _prjReverse;    /* projection control; 0 = normal, 1 = reverse */
R8 _epsAdap;       /* convergence for adaptive integration */
R8 _rcRatio;       /* rRatio of surface radii */
R8 _relSep;        /* surface separation / sum of radii */
IX _method;        /* 0 = 2AI, 1 = 1AI, 2 = 2LI, 3 = 1LI, 4 = ALI */
IX _nEdgeDiv;      /* number of edge divisions */
IX _maxRecursALI;  /* max number of ALI recursion levels */
U4 _usedV1LIadapt; /* number of V1LIadapt() calculations used */
IX _failViewALI;   /* 1 = unobstructed view factor did not converge */
IX _maxRecursion;  /* maximum number of recursion levels */
IX _minRecursion;  /* minimum number of recursion levels */
IX _failRecursion; /* 1 = obstructed view factor did not converge */
R8 _epsAF;         /* convergence for current AF calculation */
U4 _wastedVObs;    /* number of ViewObstructed() calculations wasted */
U4 _usedVObs;      /* number of ViewObstructed() calculations used */
U4 _totPoly;       /* total number of polygon view factors */
U4 _totVpt;        /* total number of view points */
IX _failConverge;  /* 1 if any calculation failed to converge */
SRFDAT3X _srf1T;   /* participating surface; transformed coordinates */
SRFDAT3X _srf2T;   /* participating surface; transformed coordinates;
                      view from srf1T toward srf2T. */ 
SRFDAT3X *_srfOT;  /* pointer to array of view obstrucing surfaces;
                       dimensioned from 0 to maxSrfT in View3d();
                       coordinates transformed relative to srf2T. */
