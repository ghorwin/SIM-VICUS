/*subfile:  view3d.c  *********************************************************/
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
#include <stdarg.h> /* variable argument list macro definitions */
#include <stdlib.h> /* prototype: exit */
#include <string.h> /* prototype: memcpy */
#include <math.h>   /* prototypes: fabs, sqrt */
#include <float.h>  /* define: FLT_EPSILON */
#include "types.h"
#include "view3d.h"
#include "prtyp.h"
void ViewMethod(SRFDATNM *srfN, SRFDATNM *srfM, R8 distNM, VFCTRL *vfCtrl);
void InitViewMethod(VFCTRL *vfCtrl);

extern IX _list;    /* output control, higher value = more output */
extern FILE *_ulog; /* log file */
extern I1 _string[]; /* buffer for a character string */
extern I1 *methods[]; /* method abbreviations */

IX _row=0;  /* row number; save for error() */
IX _col=0;  /* column number; " */
R8 _sli4;   /* use SLI if rcRatio > 4 and relSep > _sli4 */
R8 _sai4;   /* use SAI if rcRatio > 4 and relSep > _sai4 */
R8 _sai10;  /* use SAI if rcRatio > 10 and relSep > _sai10 */
R8 _dai1;   /* use DAI if relSep > _dai1 */
R8 _sli1;   /* use SLI if relSep > _sli1 */

/***  View3D.c  **************************************************************/

/*  Driver function to compute view factors for 3-dimensional surfaces.  */

/*  The AF array is stored in triangular form:
 *  +--------+--------+--------+--------+----
 *  | [1][1] |   -    |   -    |   -    |  -
 *  +--------+--------+--------+--------+----
 *  | [2][1] | [2][2] |   -    |   -    |  -
 *  +--------+--------+--------+--------+----
 *  | [3][1] | [3][2] | [3][3] |   -    |  -
 *  +--------+--------+--------+--------+----
 *  | [4][1] | [4][2] | [4][3] | [4][4] |  -
 *  +--------+--------+--------+--------+----
 *  |  ...   |  ...   |  ...   |  ...   | ...
 */

void View3D(SRFDAT3D *srf, const IX *base, IX *possibleObstr,
			R8 **AF, VFCTRL *vfCtrl)
/* srf    - surface / vertex data for all surfaces
 * base   - base surface numbers
 * possibleObstr - list of possible view obstructing surfaces
 * AF     - array of Area * F values
 * vfCtrl - control values consolitated into structure
 */
{
  IX n;  /* row */
  IX m;  /* column */
  IX n1=1, nn;     /* first and last rows */
  IX m1=1, mm;     /* first and last columns */
  IX *maskSrf;     /* list of mask and null surfaces */
  IX *possibleObstrN;  /* list of possible obstructions rel. to N */
  IX *probableObstr;   /* list of probable obstructions */
  IX nPossN;       /* number of possible obstructions rel. to N */
  IX nProb;        /* number of probable obstructions */
  IX mayView;      /* true if surfaces may view each other */
  SRFDATNM srfN;   /* row N surface */
  SRFDATNM srfM;   /* column M surface */
  SRFDATNM *srf1;  /* view from srf1 to srf2 -- */
  SRFDATNM *srf2;  /*   one is srfN, the other is srfM. */
  VECTOR3D vNM;    /* vector between centroids of srfN and srfM */
  R8 distNM;       /* distance between centroids of srfN and srfM */
  R8 minArea;      /* area of smaller surface */
  UX nAF0=0;       /* number of AF which must equal 0 */
  UX nAFnO=0;      /* number of AF without obstructing surfaces */
  UX nAFwO=0;      /* number of AF with obstructing surfaces */
  UX nObstr=0;     /* total number of obstructions considered */
  UX **bins;       /* for statistical summary */
  R8 nAFtot=1;     /* total number of view factors to compute */
  IX maxSrfT=4;    /* max number of participating (transformed) surfaces */

  maxSrfT = vfCtrl->nPossObstr + 1;
  nn = vfCtrl->nRadSrf;
  if(nn>1) {
	nAFtot = (R8)((nn-1)*nn);
  }
  if(vfCtrl->row > 0) {
	n1 = nn = vfCtrl->row;   /* can process a single row of view factors, */
	if(vfCtrl->col > 0) {
	  m1 = vfCtrl->col;      /* or a single view factor, */
	}
  }

  ViewsInit(4, 1);  /* initialize Gaussian integration coefficients */
  InitViewMethod(vfCtrl);

  possibleObstrN = Alc_V(1, vfCtrl->nAllSrf, sizeof(IX), __FILE__, __LINE__);
  probableObstr = Alc_V(1, vfCtrl->nAllSrf, sizeof(IX), __FILE__, __LINE__);

  vfCtrl->srfOT = Alc_V(0, maxSrfT, sizeof(SRFDAT3X), __FILE__, __LINE__);
  bins = Alc_MC(0, 4, 1, 5, sizeof(UX), __FILE__, __LINE__);
  vfCtrl->failConverge = 0;

  if(vfCtrl->nMaskSrf) { /* pre-process view masking surfaces */
	maskSrf = Alc_V(1, vfCtrl->nMaskSrf, sizeof(IX), __FILE__, __LINE__);
	for(m=1,n=vfCtrl->nRadSrf; n; n--) { /* set mask list */
	  if(srf[n].type == MASK || srf[n].type == NULS) {
		maskSrf[m++] = n;
	  }
	}
	DumpOS("Mask and Null surfaces:", vfCtrl->nMaskSrf, maskSrf);

	for(n=n1; n<=nn; n++) {
	  if(vfCtrl->col) {
		mm = m1 + 1;
	  } else {
		mm = n;
	  }
	  for(m=m1; m<mm; m++) { /* set all AF involving mask/null */
		if(srf[n].type == MASK || srf[n].type == NULS) {
		  if(base[n] == m) {
			AF[n][m] = srf[n].area;
		  } else {
			AF[n][m] = 0.0;
		  }
		} else if(srf[m].type == MASK || srf[m].type == NULS) {
		  AF[n][m] = 0.0;
		} else {
		  AF[n][m] = -1.0;     /* set AF flag values */
		}
	  }
	}
  }

  for(n=n1; n<=nn; n++) {  /* process AF values for row N */
	_row = n;
	if(vfCtrl->row == 0) { /* progress display - all surfaces */
	  R8 pctDone = 100 * (R8)((n-1)*n) / nAFtot;
	  fprintf(stderr, "\rSurface: %d; ~ %.1f %% complete", _row, pctDone);
	}
	AF[n][n] = 0.0;
	nPossN = vfCtrl->nPossObstr;  /* remove obstructions behind N */
	memcpy(possibleObstrN+1, possibleObstr+1, nPossN*sizeof(IX));
	nPossN = OrientationTestN(srf, n, vfCtrl, possibleObstrN, nPossN);
	if(vfCtrl->col) { /* set column limits */
	  mm = m1 + 1;
	} else if(vfCtrl->row > 0) {
	  mm = vfCtrl->nRadSrf + 1;
	} else {
	  mm = n;
	}

	for(m=m1; m<mm; m++) { /* compute view factor: row N, columns M */
	  if(vfCtrl->nMaskSrf && AF[n][m] >= 0.0) {
		continue;
	  }
	  if(m == n) {
		continue;
	  }
	  _col = m;
	  if(vfCtrl->row > 0 && vfCtrl->col == 0) { /* progress display - single surface */
		fprintf(stderr, "\rSurface %d to surface %d", _row, _col);
	  }
	  if(_list>0 && vfCtrl->row) {
		fprintf(_ulog, "*ROW %d, COL %d\n", _row, _col);
	  }
	  if(vfCtrl->col) {
		DumpSrf3D("  srf", srf+_row);
		DumpSrf3D("  srf", srf+_col);
		fflush(_ulog);
	  }

	  minArea = MIN(srf[n].area, srf[m].area);
	  mayView = SelfObstructionTest3D(srf+n, srf+m, &srfM);
	  if(mayView) {
		mayView = SelfObstructionTest3D(srf+m, srf+n, &srfN);
	  }
	  if(mayView) {
		if(srfN.area * srfM.area == 0.0) { /* must clip one or both surfces */
		  if(srfN.area + srfM.area == 0.0) {
			IntersectionTest(&srfN, &srfM);  /* check invalid geometry */
			SelfObstructionClip(&srfN);
			SelfObstructionClip(&srfM);
		  } else if(srfN.area == 0.0) {
			SelfObstructionClip(&srfN);
		  } else if(srfM.area == 0.0) {
			SelfObstructionClip(&srfM);
		  }
		}
		if(vfCtrl->col) {
		  DumpSrfNM("srfN", &srfN);
		  DumpSrfNM("srfM", &srfM);
		  fflush(_ulog);
		}
		VECTOR((&srfN.ctd), (&srfM.ctd), (&vNM));
		distNM = VLEN((&vNM));
		if(distNM < 1.0e-5 * (srfN.rc + srfM.rc)) {
		  error(3,__FILE__,__LINE__,"Surfaces have same centroids in View3D");
		}

		nProb = nPossN;
		memcpy(probableObstr+1, possibleObstrN+1, nProb*sizeof(IX));

		/* special test for extreme clipping; clipped surface amost
		   in the plane of the other surface.  */
		if(srfN.area < 1.0e-4*srf[n].area ||
		   srfM.area < 1.0e-4*srf[m].area) {
		  nProb = 0;
		  if(vfCtrl->col) {
			fprintf(_ulog, "Extreme Clipping\n");
		  }
		}

		if(nProb) {
		  nProb = ConeRadiusTest(srf, &srfN, &srfM,
								 vfCtrl, probableObstr, nProb, distNM);
		}
		//DumpOS(" Rad LOS:", nProb, probableObstr);

		if(nProb) {
		  nProb = BoxTest(srf, &srfN, &srfM, vfCtrl, probableObstr, nProb);
		}
		//DumpOS(" Box LOS:", nProb, probableObstr);

		if(nProb) {  /* test/set obstruction orientations */
		  nProb = OrientationTest(srf, &srfN, &srfM,
								  vfCtrl, probableObstr, nProb);
		}
		//DumpOS(" Orn LOS:", nProb, probableObstr);

		if(vfCtrl->nMaskSrf) { /* add masking surfaces */
		  nProb = AddMaskSrf(srf, &srfN, &srfM, maskSrf, base,
							 vfCtrl, probableObstr, nProb);
		}
		vfCtrl->nProbObstr = nProb;

		{
		  IX j, k=0;
		  for(j=1; j<=nProb; j++) {
			if(probableObstr[j] != _row && probableObstr[j] != _col) {
			  probableObstr[++k] = probableObstr[j];
			}
		  }
		  nProb = k;
		}
		//DumpOS(" Msk LOS:", nProb, probableObstr);

		if(vfCtrl->nProbObstr) {   /*** obstructed view factors ***/
		  SRFDAT3X subs[5];    /* subsurfaces of surface 1  */
		  IX j, nSubSrf;       /* count / number of subsurfaces */
		  R8 calcAF = 0.0;
		  /* set direction of projection */
		  if(ProjectionDirection(srf, &srfN, &srfM,
								 probableObstr, vfCtrl) > 0) {
			srf1 = &srfN;
			srf2 = &srfM;
		  } else {
			srf1 = &srfM;
			srf2 = &srfN;
		  }

		  if(vfCtrl->col) {
			fprintf(_ulog, " Project rays from srf %d to srf %d\n",
					srf1->nr, srf2->nr);
			//          DumpSrfNM("from srf", srf1);
			//          DumpSrfNM("  to srf", srf2);
			fprintf(_ulog, " %d probable obtructions:\n", vfCtrl->nProbObstr);
			for(j=1; j<=vfCtrl->nProbObstr; j++) {
			  DumpSrf3D("   surface", srf+probableObstr[j]);
			}
		  } else if(_list>0 && vfCtrl->row) {
			fprintf(_ulog, " %d probable obtructions\n", vfCtrl->nProbObstr);
		  }

		  if(vfCtrl->nProbObstr > maxSrfT) { /* expand srfOT array */
			Fre_V(vfCtrl->srfOT, 0, maxSrfT, sizeof(SRFDAT3X), __FILE__, __LINE__);
			maxSrfT = vfCtrl->nProbObstr + 4;
			vfCtrl->srfOT = Alc_V(0, maxSrfT, sizeof(SRFDAT3X), __FILE__, __LINE__);
		  }
		  CoordTrans3D(srf, srf1, srf2, probableObstr, vfCtrl);

		  nSubSrf = Subsurface(&vfCtrl->srf1T, subs);
		  for(vfCtrl->failRecursion=j=0; j<nSubSrf; j++) {
			//          minArea = MIN(subs[j].area, vfCtrl->srf2T.area);
			vfCtrl->epsAF = minArea * vfCtrl->epsAdap;
			if(subs[j].nv == 3) {
			  calcAF += ViewTP(subs[j].v, subs[j].area, 0, vfCtrl);
			} else {
			  calcAF += ViewRP(subs[j].v, subs[j].area, 0, vfCtrl);
			}
		  }
		  AF[n][m] = calcAF * srf2->rc * srf2->rc;   /* area scaling factor */
		  if(vfCtrl->failRecursion) {
			fprintf(_ulog, " row %d, col %d,  recursion did not converge, AF %g\n",
					_row, _col, AF[n][m]);
			vfCtrl->failConverge = 1;
		  }
		  nObstr += vfCtrl->nProbObstr;
		  nAFwO += 1;
		  vfCtrl->method = 5;
		} else {                    /*** unobstructed view factors ***/
		  //SRFDATNM *srf1;  /* pointer to surface 1 (smaller surface) */
		  //SRFDATNM *srf2;  /* pointer to surface 2 */
		  vfCtrl->method = 5;
		  vfCtrl->failViewALI = 0;
		  //if(srfN.rc >= srfM.rc)
		  //  { srf1 = &srfM; srf2 = &srfN; }
		  //else
		  //  { srf1 = &srfN; srf2 = &srfM; }
		  ViewMethod(&srfN, &srfM, distNM, vfCtrl);
		  //        minArea = MIN(srfN.area, srfM.area);
		  vfCtrl->epsAF = minArea * vfCtrl->epsAdap;
		  AF[n][m] = ViewUnobstructed(vfCtrl, _row, _col);
		  if(vfCtrl->failViewALI) {
			fprintf(_ulog, " row %d, col %d,  line integral did not converge, AF %g\n",
					_row, _col, AF[n][m]);
			vfCtrl->failConverge = 1;
		  }
		  if(vfCtrl->method<5) { // ???
			bins[vfCtrl->method][vfCtrl->nEdgeDiv] += 1;   /* count edge divisions */
		  }
		  nAFnO += 1;
		}
	  } else {         /* view not possible */
		AF[n][m] = 0.0;
		nAF0 += 1;
		vfCtrl->method = 6;
	  }

	  if(srf[n].area > srf[m].area) { /* remove very small values */
		if(AF[n][m] < 1.0e-12 * srf[n].area) {
		  AF[n][m] = 0.0;
		}
	  } else {
		if(AF[n][m] < 1.0e-12 * srf[m].area) {
		  AF[n][m] = 0.0;
		}
	  }

	  if(_list>0 && vfCtrl->row) {
		fprintf(_ulog, " AF(%d,%d): %.7e %.7e %.7e %s\n", _row, _col,
				AF[n][m], AF[n][m] / srf[n].area, AF[n][m] / srf[m].area,
				methods[vfCtrl->method]);
		fflush(_ulog);
	  }

	}  /* end of element M of row N */

  }  /* end of row N */
  fputc('\n', stderr);

  fprintf(_ulog, "\nSurface pairs where F(i,j) must be zero: %8u\n", nAF0);
  fprintf(_ulog, "\nSurface pairs without obstructed views:  %8u\n", nAFnO);
  bins[4][5] = bins[0][5] + bins[1][5] + bins[2][5] + bins[3][5];
  fprintf(_ulog, "   nd %7s %7s %7s %7s %7s\n",
		  methods[0], methods[1], methods[2], methods[3], methods[4]);
  fprintf(_ulog, "    2 %7u %7u %7u %7u %7u direct\n",
		  bins[0][2], bins[1][2], bins[2][2], bins[3][2], bins[4][2]);
  fprintf(_ulog, "    3 %7u %7u %7u %7u\n",
		  bins[0][3], bins[1][3], bins[2][3], bins[3][3]);
  fprintf(_ulog, "    4 %7u %7u %7u %7u\n",
		  bins[0][4], bins[1][4], bins[2][4], bins[3][4]);
  fprintf(_ulog, "  fix %7u %7u %7u %7u %7u fixes\n",
		  bins[0][5], bins[1][5], bins[2][5], bins[3][5], bins[4][5]);
  ViewsInit(4, 0);
  fprintf(_ulog, "Adaptive line integral evaluations used: %8lu\n",
		  vfCtrl->usedV1LIadapt);
  fprintf(_ulog, "\nSurface pairs with obstructed views:   %10u\n", nAFwO);
  if(nAFwO>0) {
	fprintf(_ulog, "Average number of obstructions per pair:   %6.2f\n",
			(R8)nObstr / (R8)nAFwO);
	fprintf(_ulog, "Adaptive viewpoint evaluations used:   %10u\n",
			vfCtrl->usedVObs);
	fprintf(_ulog, "Adaptive viewpoint evaluations lost:   %10u\n",
			vfCtrl->wastedVObs);
	fprintf(_ulog, "Non-zero viewpoint evaluations:        %10u\n",
			vfCtrl->totVpt);
	/***fprintf(_ulog, "Number of 1AI point-polygon evaluations: %8u\n",
	  vfCtrl->totPoly);***/
	fprintf(_ulog, "Average number of polygons per viewpoint:  %6.2f\n\n",
			(R8)vfCtrl->totPoly / (R8)vfCtrl->totVpt);
  }

  if(vfCtrl->failConverge) {
	error(1, __FILE__, __LINE__, "Some calculations did not converge, see VIEW3D.LOG");
  }

#ifdef DEBUG
  MemRem("After View3D() calculations");
#endif
  if(vfCtrl->nMaskSrf) {
	Fre_V(maskSrf, 1, vfCtrl->nMaskSrf, sizeof(IX), __FILE__, __LINE__);
  }
  Fre_MC(bins, 0, 4, 1, 5, sizeof(IX), __FILE__, __LINE__);
  Fre_V(vfCtrl->srfOT, 0, maxSrfT, sizeof(SRFDAT3X), __FILE__, __LINE__);
  Fre_V(probableObstr, 1, vfCtrl->nAllSrf, sizeof(IX), __FILE__, __LINE__);
  Fre_V(possibleObstrN, 1, vfCtrl->nAllSrf, sizeof(IX), __FILE__, __LINE__);

}  /* end of View3D */

/***  ProjectionDirection.c  *************************************************/

/*  Set direction of projection of obstruction shadows.
 *  Direction will be from surface 1 to surface 2.
 *  Want surface 2 large and distance from surface 2
 *  to obstructions to be small.
 *  Return direction indicator: 1 = N to M, -1 = M to N.
 */

IX ProjectionDirection(SRFDAT3D *srf, SRFDATNM *srfN, SRFDATNM *srfM,
					   IX *probableObstr, VFCTRL *vfCtrl)
/* srf  - data for all surfaces.
 * srfN - data for surface N.
 * srfM - data for surface M.
 * probableObstr  - list of probable obstructing surfaces (input and revised for output).
 * vfCtrl - computation controls.
 */
{
  IX j, k;   /* surface number */
  VECTOR3D v;
  R8 sdtoN, sdtoM; /* minimum distances from obstruction to N and M */
  IX direction=0;  /* 1 = N is surface 1; -1 = M is surface 1 */

#ifdef DEBUG
  fprintf(_ulog, "ProjectionDirection:\n");
#endif

#ifdef XXX
  if(fabs(srfN->rc - srfM->rc) > 0.002*(srfN->rc + srfM->rc))
	if(srfN->rc <= srfM->rc)    old code
		direction = 1;
	else
	  direction = -1;
  this is worse for OVERHANG.VS3
	#endif
	#ifdef XXX  // this great idea doesn't seem to work well
	  if(srfM->rc > 10.0 * srfN->rc)  // 2005/11/06
	  direction = 1;
  if(srfN->rc > 10.0 * srfM->rc)
	direction = -1;
#endif

  if(direction == 0) {
	/* determine distances from centroid of K to centroids of N & M */
	sdtoM = sdtoN = 1.0e9;
	for(j=1; j<=vfCtrl->nProbObstr; j++) {
	  R8 dist;
	  k = probableObstr[j];
	  if(srf[k].NrelS >= 0) {
		VECTOR((&srfN->ctd), (&srf[k].ctd), (&v));
		dist = VDOT((&v), (&v));
		if(dist<sdtoN) {
		  sdtoN = dist;
		}
	  }
	  if(srf[k].MrelS >= 0) {
		VECTOR((&srfM->ctd), (&srf[k].ctd), (&v));
		dist = VDOT((&v), (&v));
		if(dist<sdtoM) {
		  sdtoM = dist;
		}
	  }
	}
	sdtoN = sqrt(sdtoN);
	sdtoM = sqrt(sdtoM);
#ifdef DEBUG
	fprintf(_ulog, " min dist to srf %d: %e\n", srfN->nr, sdtoN);
	fprintf(_ulog, " min dist to srf %d: %e\n", srfM->nr, sdtoM);
#endif

	/* direction based on distances and areas of N & M */
	if(fabs(sdtoN - sdtoM) > 0.002*(sdtoN + sdtoM)) {
	  if(sdtoN < sdtoM) {
		direction = -1;
	  }
	  if(sdtoN > sdtoM) {
		direction = 1;
	  }
	}

#ifdef DEBUG
	fprintf(_ulog, " sdtoN %e, sdtoM %e, dir %d\n",
			sdtoN, sdtoM, direction);
#endif
  }

  if(!direction) { /* direction based on number of obstructions */
	IX nosN=0, nosM=0;   /* number of surfaces facing N or M */
	for(j=1; j<=vfCtrl->nProbObstr; j++) {
	  k = probableObstr[j];
	  if(srf[k].NrelS >= 0) {
		nosN++;
	  }
	  if(srf[k].MrelS >= 0) {
		nosM++;
	  }
	}
	if(nosN > nosM) {
	  direction = +1;
	} else {
	  direction = -1;
	}
  }

#ifdef DEBUG
  fprintf(_ulog, " rcN %e, rcM %e, dir %d, pdir %d\n",
		  srfN->rc, srfM->rc, direction, vfCtrl->prjReverse);
#endif

  if(vfCtrl->prjReverse) {     /* direction reversal parameter */
	direction *= -1;
  }

  k = 0;         /* eliminate probableObstr surfaces facing wrong direction */
  if(direction > 0) { /* projections from N toward M */
	for(j=1; j<=vfCtrl->nProbObstr; j++) {
	  if(srf[probableObstr[j]].MrelS >= 0) {
		probableObstr[++k] = probableObstr[j];
	  }
	}
  }
  if(direction < 0) { /* projections from M toward N */
	for(j=1; j<=vfCtrl->nProbObstr; j++) {
	  if(srf[probableObstr[j]].NrelS >= 0) {
		probableObstr[++k] = probableObstr[j];
	  }
	}
  }
  vfCtrl->nProbObstr = k;

  return direction;

}  /*  end of ProjectionDirection  */

/***  ViewMethod.c  **********************************************************/

/*  Determine method to compute unobstructed view factor.  */

void ViewMethod(SRFDATNM *srfN, SRFDATNM *srfM, R8 distNM, VFCTRL *vfCtrl)
{
  SRFDATNM *srf1;  /* pointer to surface 1 (smaller surface) */
  SRFDATNM *srf2;  /* pointer to surface 2 */

  if(srfN->rc >= srfM->rc) {
	srf1 = srfM;
	srf2 = srfN;
  } else {
	srf1 = srfN;
	srf2 = srfM;
  }
  memcpy(&vfCtrl->srf1T, srf1, sizeof(SRFDAT3X));
  memcpy(&vfCtrl->srf2T, srf2, sizeof(SRFDAT3X));

  vfCtrl->rcRatio = srf2->rc / srf1->rc;
  vfCtrl->relSep = distNM / (srf1->rc + srf2->rc);

  vfCtrl->method = UNK;
  if(vfCtrl->rcRatio > 4) {
	if(srf1->shape) {
	  R8 relDot = VDOTW((&srf1->ctd), (&srf2->dc));
	  if(vfCtrl->rcRatio > 10.0f &&
		 (vfCtrl->relSep > _sai10 || relDot > 2.0*srf1->rc)) {
		vfCtrl->method = SAI;
	  } else if(vfCtrl->relSep > _sai4 || relDot > 2.0*srf1->rc) {
		vfCtrl->method = SAI;
	  }
	}
	if(vfCtrl->method==UNK && vfCtrl->relSep > _sli4) {
	  vfCtrl->method = SLI;
	}
  }
  if(vfCtrl->method==UNK && vfCtrl->relSep > _dai1
	 && srf1->shape && srf2->shape) {
	vfCtrl->method = DAI;
  }
  if(vfCtrl->method==UNK && vfCtrl->relSep > _sli1) {
	vfCtrl->method = SLI;
  }
  if(vfCtrl->method==SLI && vfCtrl->epsAdap < 0.5e-6) {
	vfCtrl->method = ALI;
  }
  if(vfCtrl->method==UNK) {
	vfCtrl->method = ALI;
  }

  if(vfCtrl->col) {
	fprintf(_ulog, "ViewMethod %s; R-ratio %.3f, A-ratio %.3f, relSep %.4f, vertices %d %d\n",
			methods[vfCtrl->method], vfCtrl->rcRatio, srf2->area / srf1->area,
			vfCtrl->relSep, srf1->nv, srf2->nv);
  }

}  /* end ViewMethod */

/***  InitViewMethod.c  ******************************************************/

/*  Initialize ViewMethod() coefficients.  */

void InitViewMethod(VFCTRL *vfCtrl)
{
  if(vfCtrl->epsAdap < 0.99e-7) {
	_sli4 = 0.7f;
	_sai4 = 1.5f;
	_sai10 = 1.8f;
	_dai1 = 3.0f;
	_sli1 = 3.0f;
  } else if(vfCtrl->epsAdap < 0.99e-6) {
	_sli4 = 0.5f;
	_sai4 = 1.2f;
	_sai10 = 1.2f;
	_dai1 = 2.3f;
	_sli1 = 2.2f;
  } else if(vfCtrl->epsAdap < 0.99e-5) {
	_sli4 = 0.45f;
	_sai4 = 1.1f;
	_sai10 = 1.0f;
	_dai1 = 1.7f;
	_sli1 = 1.5f;
  } else if(vfCtrl->epsAdap < 0.99e-4) {
	_sli4 = 0.4f;
	_sai4 = 1.0f;
	_sai10 = 0.8f;
	_dai1 = 1.3f;
	_sli1 = 0.9f;
  } else {
	_sli4 = 0.3f;
	_sai4 = 0.9f;
	_sai10 = 0.6f;
	_dai1 = 1.0f;
	_sli1 = 0.6f;
  }

}  /* end InitViewMethod */

