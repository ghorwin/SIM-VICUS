/*subfile:  v3main.c  *********************************************************/
/*                                                                            */
/*  Copyright (c) 2014 Jason W. DeGraw                                        */
/*                                                                            */
/*  This file is part of View3D.                                              */
/*                                                                            */
/*  View3D is free software: you can redistribute it and/or modify it         */
/*  under the terms of the GNU General Public License as published by         */
/*  the Free Software Foundation, either version 3 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  View3D is distributed in the hope that it will be useful, but             */
/*  WITHOUT ANY WARRANTY; without even the implied warranty of                */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         */
/*  General Public License for more details.                                  */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with View3D.  If not, see <http://www.gnu.org/licenses/>.           */
/*                                                                            */
/******************************************************************************/
/*  Main program for batch processing of 3-D view factors.  */

#include <stdio.h>
#include <string.h> /* prototype: strcpy */
#include <stdlib.h> /* prototype: exit */
#include <math.h>   /* prototype: sqrt */
#include <time.h>   /* prototypes: time, localtime, asctime;
					   define: tm, time_t */
#include "types.h"
#include "view3d.h"
#include "prtyp.h"

FILE *_unxt = NULL; /* input file */
FILE *_ulog = NULL; /* log file */
IX _echo=0;  /* true = echo input file */
IX _list=0;  /* output control, higher value = more output:
				0 = summary;
				1 = list view factors;
				2 = echo input, note calculations;
				3 = note obstructions. */
I1 _string[LINELEN];  /* buffer for a character string */
I1 *methods[7] = {"2AI", "1AI", "2LI", "1LI",
				  "ALI", "Adapt", "Blocked"}; /* abbreviations */
IX _maxNVT=12;   /* maximum number of temporary polygon overlap vertices */

void ReadVF(I1 *fileName, I1 *program, I1 *version, IX *format, IX *encl,
			IX *didemit, IX *nSrf, R4 *area, R4 *emit, R8 **AF, R4 **F,
			IX init, IX shape);
void SaveVF(I1 *fileName, I1 *program, I1 *version, IX format, IX encl,
			IX didemit, IX nSrf, R4 *area, R4 *emit, R8 **AF, R4 *vtmp);
//void ReportAF(const IX nSrf, const IX encl, const I1 *title, I1 ** const name,
//              const R4 *area, const R4 *emit, const IX *base, const R8 **AF,
//              IX flag);

/***  usage  ******************************************************************/
/*  Describe usage of the program  */
void usage(FILE *fp)
{
  fprintf(fp,
	  "%s, version %s - calculation of view factors between simple polygons\n\n",
	  PROGRAMSTR,VERSIONSTR);
  fprintf(fp,"usage: %s <input file> <output file>\n", PROGRAMSTR);
  fputs("Arguments:\n\tinput file     VS3 file to read\n",fp);
  fputs("\toutput file    View factor file to write\n",fp);
  //fputs("Options:\n",fp);
  //fputs("\t-h             Print this help and exit\n",fp);
} /* end usage */

/***  fileCheck  **************************************************************/
/*  Check that the named file can be opened in the given mode  */
IX fileCheck(I1 *filename, I1 *mode)
{
  FILE *fp = fopen(filename,mode);
  if(!fp) {
	return 0;
  }
  fclose(fp);
  return 1;
} /* end fileCheck */

IX main(IX argc, I1 **argv)
  {
  I1 inFile[_MAX_PATH]=""; /* input file name */
  I1 outFile[_MAX_PATH]="";/* output file name */
  I1 title[LINELEN];  /* project title */
  I1 **name;       /* surface names [1:nSrf][0:NAMELEN] */
  I1 *types[]={"rsrf","subs","mask","nuls","obso"};
  VERTEX3D *xyz;   /* vector of vertces [1:nVrt] - for ease in
						converting V3MAIN to a subroutine */
  SRFDAT3D *srf;   /* vector of surface data structures [1:nSrf] */
  VFCTRL vfCtrl;   /* VF calculation control parameters - avoid globals */
  R8 **AF;         /* triangular array of area*view factor values [1:nSrf][] */
  R4 *area;        /* vector of surface areas [1:nSrf] */
  R4 *emit;        /* vector of surface emittances [1:nSrf] */
  IX *base;        /* vector of base surface numbers [1:nSrf] */
  IX *cmbn;        /* vector of combine surface numbers [1:nSrf] */
  R4 *vtmp;        /* temporary vector [1:nSrf] */
  IX *possibleObstr;  /* list of possible view obstructing surfaces */
  struct tm *curtime; /* time structure */
  time_t bintime;  /* seconds since 00:00:00 GMT, 1/1/70 */
  R4 time0, time1; /* elapsed time values */
  IX nSrf;         /* current number of surfaces */
  IX nSrf0;        /* initial number of surfaces */
  IX encl;         /* 1 = surfaces form enclosure */
  IX n, flag;

  if(argc != 3) {
	  usage(stderr);
	  return EXIT_FAILURE;
	}
  /* open log file */
  _ulog = fopen("View3D.log", "w");
  //_ulog = stdout;
  if(!_ulog) {
	error(3, __FILE__, __LINE__, "Failed to open VIEW3D.LOG");
  }

  strcpy(inFile, argv[1]);
  NxtOpen(inFile, __FILE__, __LINE__);

  strcpy(outFile, argv[2]);
  if(!fileCheck(outFile,"w"))
	error(3, __FILE__, __LINE__, "Cannot open \"%s\" for output", outFile);

  fprintf(_ulog, "  Program:  %s\n", argv[0]);
#ifdef DEBUG
  fprintf(_ulog, "  Version:  %s DEBUG\n", VERSIONSTR);
#else
  fprintf(_ulog, "  Version:  %s\n", VERSIONSTR);
#endif
  fprintf(_ulog, "  Created:  %s at %s\n", __DATE__, __TIME__);
#if( _MSC_VER ) /* Output _MSC_VER */
  fprintf(_ulog, "  Compiler: Visual C++ Version %d\n", _MSC_FULL_VER );
#elif __GNUC__
#if __MINGW64__
  fprintf(_ulog, "  Compiler: MinGW-w64 GCC %d.%d.%d\n", __GNUC__,
		  __GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#elif( __MINGW32__ )
  fprintf(_ulog, "  Compiler: MinGW GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__,
		  __GNUC_PATCHLEVEL__);
#else
  fprintf(_ulog, "  Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__,
		  __GNUC_PATCHLEVEL__);
#endif
#endif

#ifdef DEBUG
  _echo = 1;
#endif

  fprintf(_ulog, "Data file:  %s\n", inFile);
  fprintf(_ulog, "Output file:  %s\n", outFile);

  time(&bintime);
  curtime = localtime(&bintime);
  fprintf(_ulog, "Time:  %s", asctime(curtime));
  fputs("\nView3D - calculation of view factors between simple polygons.\n\n\
  This software is distributed in the hope that it will be useful,\n\
  but WITHOUT ANY WARRANTY; without even the implied warranty\n\
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n",stderr);

  fprintf(stderr,"Beginning view factor calculation\n");
  /* Record the start-of-run time */
  time0 = CPUtime(0.0);

  /* Initialize control data */
  memset(&vfCtrl, 0, sizeof(VFCTRL));
  /* Non-zero control values */
  vfCtrl.epsAdap = 1.0e-4f; // convergence for adaptive integration
  vfCtrl.maxRecursALI = 12; // maximum number of recursion levels
  vfCtrl.maxRecursion = 8;  // maximum number of recursion levels

  /* Read Vertex/Surface data file */
  //NxtOpen(inFile, __FILE__, __LINE__);
  CountVS3D(title, &vfCtrl);
  fprintf(_ulog, "\nTitle: %s\n", title);
  fprintf(_ulog, "Control values for 3-D view factor calculations:\n");
  if(vfCtrl.enclosure) {
	fprintf(_ulog, "  surfaces form enclosure.\n");
  }
  if(vfCtrl.emittances) {
	fprintf(_ulog, "  will process emittances.\n");
  }
  fprintf(_ulog, "     adaptive convergence: %g", vfCtrl.epsAdap);
  if(vfCtrl.epsAdap != 1.e-4f) {
	fprintf(_ulog, " *");
  }
  fprintf(_ulog, "\n  unobstructed recursions: %d", vfCtrl.maxRecursALI);
  if(vfCtrl.maxRecursALI != 12) {
	fprintf(_ulog, " *");
  }
  fprintf(_ulog, "\nmax obstructed recursions: %d", vfCtrl.maxRecursion);
  if(vfCtrl.maxRecursion != 8) {
	fprintf(_ulog, " *");
  }
  fprintf(_ulog, "\nmin obstructed recursions: %d", vfCtrl.minRecursion);
  if(vfCtrl.minRecursion) {
	fprintf(_ulog, " *");
  }
  fprintf(_ulog, "\n              solving row:");
  if(vfCtrl.row) {
	fprintf(_ulog, " %d *", vfCtrl.row);
  } else {
	fprintf(_ulog, " all");
  }
  fprintf(_ulog, "\n           solving column:");
  if(vfCtrl.col) {
	fprintf(_ulog, " %d *", vfCtrl.col);
  } else {
	fprintf(_ulog, " all");
  }
  if(vfCtrl.prjReverse) {
	fprintf(_ulog, "\n      reverse projections. **");
  }
  fprintf(_ulog, "\n output control parameter: %d\n", _list);

  fprintf(_ulog, "\n");
  fprintf(_ulog, " total number of surfaces: %d \n", vfCtrl.nAllSrf);
  fprintf(_ulog, "   heat transfer surfaces: %d \n", vfCtrl.nRadSrf);

  nSrf = nSrf0 = vfCtrl.nRadSrf;
  encl = vfCtrl.enclosure;
  if(vfCtrl.format == 4) {
	vfCtrl.nVertices = 4 * vfCtrl.nAllSrf;
  }
  name = Alc_MC(1, nSrf0, 0, NAMELEN, sizeof(I1), __FILE__, __LINE__);
  area = Alc_V(1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  emit = Alc_V(1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  vtmp = Alc_V(1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  for(n=nSrf0; n; n--) {
	vtmp[n] = 1.0;
  }
  base = Alc_V(1, nSrf0, sizeof(IX), __FILE__, __LINE__);
  cmbn = Alc_V(1, nSrf0, sizeof(IX), __FILE__, __LINE__);
  xyz = Alc_V(1, vfCtrl.nVertices, sizeof(VERTEX3D), __FILE__, __LINE__);
  srf = Alc_V(1, vfCtrl.nAllSrf, sizeof(SRFDAT3D), __FILE__, __LINE__);
  InitTmpVertMem();  /* polygon operations in GetDat() and View3D() */
  InitPolygonMem(0, 0);

			   /* read v/s data file */
  if(_list>2) {
	_echo = 1;
  }
  if(vfCtrl.format == 4) {
	GetVS3Da(name, emit, base, cmbn, srf, xyz, &vfCtrl);
  } else {
	GetVS3D(name, emit, base, cmbn, srf, xyz, &vfCtrl);
  }
  for(n=nSrf; n; n--) {
	area[n] = (R4)srf[n].area;
  }
  NxtClose();

  if(encl) {   /* determine volume of enclosure */
	R8 volume=0.0;
	for(n=vfCtrl.nAllSrf; n; n--) {
	  if(srf[n].type == SUBS) {
		continue;
	  }
	  volume += VolPrism(srf[n].v[0], srf[n].v[1], srf[n].v[2]);
	  if(srf[n].nv == 4) {
		volume += VolPrism(srf[n].v[2], srf[n].v[3], srf[n].v[0]);
	  }
	}
	volume /= -6.0;        /* see VolPrism() */
	fprintf(_ulog, "      volume of enclosure: %.3f\n", volume);
	}

  if(nSrf0 >= 1000) {
	  fprintf(stderr, "\n %.3e seconds to process input data\n",
			  CPUtime(time0));
	  fprintf(_ulog, "\n %.3e seconds to process input data\n",
			  CPUtime(time0));
	}

  if(_list>2) {
	fprintf(_ulog, "Surfaces:\n");
	fprintf(_ulog, "   #        name     area   emit  type bsn csn (dir cos) (centroid)\n");
	for(n=1; n<=nSrf; n++) {
	  fprintf(_ulog, "%4d %12s %9.2e %5.3f %4s %3d %3d (%g %g %g %g) (%g %g %g)\n",
		n, name[n], area[n], emit[n], types[srf[n].type], base[n], cmbn[n],
		srf[n].dc.x, srf[n].dc.y, srf[n].dc.z, srf[n].dc.w,
		srf[n].ctd.x, srf[n].ctd.y, srf[n].ctd.z);
	}
	for(; n<=vfCtrl.nAllSrf; n++) {
	  fprintf(_ulog, "%4d %12s %9.2e       %4s         (%g %g %g %g) (%g %g %g)\n",
		n, " ", area[n], types[srf[n].type],
		srf[n].dc.x, srf[n].dc.y, srf[n].dc.z, srf[n].dc.w,
		srf[n].ctd.x, srf[n].ctd.y, srf[n].ctd.z);
	}
	fprintf(_ulog, "Vertices:\n");
	for(n=1; n<=vfCtrl.nAllSrf; n++) {
	  IX j;
	  fprintf(_ulog, "%4d ", n);
	  for(j=0; j<srf[n].nv; j++)
		fprintf(_ulog, " (%g %g %g)",
		  srf[n].v[j]->x, srf[n].v[j]->y, srf[n].v[j]->z);
	  fprintf(_ulog, "\n");
	  }
	}

  time1 = CPUtime(0.0);  /* start-of-VF-calculation time */
  possibleObstr = Alc_V(1, vfCtrl.nAllSrf, sizeof(IX), __FILE__, __LINE__);
  vfCtrl.nPossObstr = SetPosObstr3D(vfCtrl.nAllSrf, srf, possibleObstr);
  sprintf(_string, " %.2f seconds to determine %d possible view obstructing surfaces",
	CPUtime(time1), vfCtrl.nPossObstr);
  fputs(_string, stderr);
  fputs("\n\n", stderr);
  fputs(_string, _ulog);
  if(vfCtrl.nPossObstr > 0 && _list > 1) {
	DumpOS(":", vfCtrl.nPossObstr, possibleObstr);
  } else {
	fputs("\n", _ulog);
  }
  fflush(_ulog);

  if(vfCtrl.row) {
	AF = Alc_MC(vfCtrl.row, vfCtrl.row, 1, nSrf0, sizeof(R8), __FILE__, __LINE__);
	if(vfCtrl.col) {
	  fprintf(stderr, "Computing view factor for surface %d to surface %d\n\n",
			  vfCtrl.row, vfCtrl.col);
	} else {
	  fprintf(stderr, "Computing view factors for surface %d\n\n",
			  vfCtrl.row);
	}
  } else {
	time1 = CPUtime(0.0);
	AF = Alc_MSR(1, nSrf0, sizeof(R8), __FILE__, __LINE__);
	time1 = CPUtime(time1);
	if(time1 > 1) {
	  sprintf(_string, "\n %.2f seconds to allocate %ld byte view factor matrix\n",
			  time1, 4*(n+1)*n);
	  fputs(_string, stderr);
	  fputs(_string, _ulog);
	}
	fprintf(stderr, "Computing view factors for all %d surfaces\n\n", nSrf0);
  }

  if(_list>0) {
	MemRem("At start of View3D()");
  }
  time1 = CPUtime(0.0);

  View3D(srf, base, possibleObstr, AF, &vfCtrl);  /*** view factor calculation ***/

  //fprintf(_ulog, "\n%7.2f seconds to compute view factors\n", CPUtime(time1));

  sprintf(_string, " %.2f seconds to compute view factors\n", CPUtime(time1));
  fputs(_string, stderr);
  fputs(_string, _ulog);

  if(_list>0) {
	MemRem("At end of View3D()");
  }

  Fre_V(possibleObstr, 1, vfCtrl.nAllSrf, sizeof(IX), __FILE__, __LINE__);
  FreeTmpVertMem();  /* free polygon overlap vertices */
  FreePolygonMem();
  Fre_V(xyz, 1, vfCtrl.nVertices, sizeof(VERTEX3D), __FILE__, __LINE__);

  if(vfCtrl.row) {
	IX n=vfCtrl.row,m=vfCtrl.col;
	R8 ai=1/area[n];
	R8 F, sum;
	fprintf(_ulog, "\n");
	if(vfCtrl.col) {
	  fprintf(_ulog, "F[%d][%d] = %.5e\n\n", n, m, AF[n][m]*ai);
	  goto FreeMemory;
	  }
	if(_list>0) {
	  fprintf(_ulog, "View factors from surface n (%d = %s) to surface m:\n", n, name[n]);
	  fprintf(_ulog, "srf m   A(n)*F(n,m)    F(n,m)        F(m,n)      base   cmb  name\n");
	  for(sum=0,m=1; m<=nSrf; m++) {
		F = AF[n][m]*ai;
		sum += F;
		fprintf(_ulog, "%5d %13.5e %13.5e %13.5e %5d %5d  %s\n",
		  m, AF[n][m], F, AF[n][m]/area[m], base[m], cmbn[m], name[m]);
		}
	  fprintf(_ulog, "    sum[F(n,m)] = %.8f\n\n", sum);
	  }
	// separate subsurfaces
	// combine surfaces
	for(m=1; m<=nSrf; m++) {
	  if(cmbn[m]) {
		AF[n][cmbn[m]] += AF[n][m];
		area[cmbn[m]] += area[m];
		}
	}
	fprintf(_ulog, "View factors from surface n (%d = %s) to combined surface m:\n", n, name[n]);
	fprintf(_ulog, "srf m   A(n)*F(n,m)    F(n,m)        F(m,n)      name\n");
	sum = 0;
	for(sum=0,m=1; m<=nSrf; m++) {
	  if(cmbn[m] == 0) {
		F = AF[n][m]*ai;
		sum += F;
		fprintf(_ulog, "%5d %13.5e %13.5e %13.5e  %s\n",
		  m, AF[n][m], F, AF[n][m]/area[m], name[m]);
	  }
	}
	fprintf(_ulog, "\n    sum[F(n,m)] = %.8f\n\n", sum);
	fflush(_ulog);
	goto FreeMemory;
	}

  for(n=nSrf; n; n--) { /* clear base pointers to OBSO & MASK srfs */
	if(srf[base[n]].type == OBSO) { /* Base is used for several things. */
	  base[n] = 0;                  /* It must be progressively cleared */
	}                               /* as each use is completed. */
	if(srf[n].type == MASK) {
	  base[n] = 0;
	}
  }

  if(_list>1) {
	IX *jtmp = Alc_V(1, nSrf, sizeof(IX), __FILE__, __LINE__);
	for(n=nSrf; n; n--) {
	 if(srf[n].type == NULS) {
	   jtmp[n] = 0;
	 } else {
	   jtmp[n] = base[n];
	 }
	}
	ReportAF(nSrf, encl, "Initial view factors:",
	  name, area, vtmp, jtmp, AF, 0);
	Fre_V(jtmp, 1, nSrf, sizeof(IX), __FILE__, __LINE__);
	}

  fprintf(stderr, "\nAdjusting view factors\n");
  time1 = CPUtime(0.0);

  for(flag=0,n=nSrf; n; n--) {
	if(srf[n].type==NULS) {
	  flag = 1;
	}
  }
  if(flag) {                       /* remove null surfaces */
	nSrf = DelNull(nSrf, srf, base, cmbn, emit, area, name, AF);
	if(_list>1) {
	  ReportAF(nSrf, encl, "View factors after removing null surfaces:",
		name, area, vtmp, base, AF, 0);
	}
	}

  for(flag=0,n=nSrf; n; n--) {
	if(base[n]>0) {
	  flag = 1;
	}
  }
  if(flag) {                       /* separate subsurfaces */
	Separate(nSrf, base, area, AF);
	for(n=nSrf; n; n--) {
	  base[n] = 0;
	}
	if(_list>1) {
	  ReportAF(nSrf, encl, "View factors after separating included surfaces:",
		name, area, vtmp, base, AF, 0);
	}
	}

  for(flag=0,n=nSrf; n; n--) {
	if(cmbn[n]>0) {
	  flag = 1;
	}
  }
  if(flag) {                       /* combine surfaces */
	nSrf = Combine(nSrf, cmbn, area, name, AF);
	if(_list>1) {
	  fprintf(_ulog,"Surfaces:\n");
	  fprintf(_ulog,"  n   base  cmbn   area\n");
	  for(n=1; n<=nSrf; n++) {
		fprintf(_ulog,"%3d%5d%6d%12.4e\n", n, base[n], cmbn[n], area[n]);
	  }
	  ReportAF(nSrf, encl, "View factors after combining surfaces:",
		name, area, vtmp, base, AF, 0);
	  }
	}

  if(encl || vfCtrl.emittances) {   /* intermediate report - unnormalized view factors */
	if(_list < 2) {
	  ReportAF(nSrf, encl, title, name, area, vtmp, base, AF, 1);
	}
  }

  if(encl) {                       /* normalize view factors */
	NormAF(nSrf, vtmp, area, AF, 1.0e-7f, 100);
	if(_list>1) {
	  ReportAF(nSrf, encl, "View factors after normalization:",
		name, area, vtmp, base, AF, 0);
	}
	}
  sprintf(_string, " %.2f seconds to adjust view factors\n", CPUtime(time1));
  fputs(_string, stderr);
  fputs(_string, _ulog);

  if(vfCtrl.emittances) {
	fprintf(stderr, "\nProcessing surface emissivites\n");
	time1 = CPUtime(0.0);
	IntFac(nSrf, emit, area, AF);
	sprintf(_string, " %.2f seconds to include emissivities\n", CPUtime(time1));
	fputs(_string, stderr);
	fputs(_string, _ulog);
	if(_list>1) {
	  ReportAF(nSrf, encl, "View factors including emissivities:", name, area, emit, base, AF, 0);
	}
	if(encl) {
	  NormAF(nSrf, emit, area, AF, 1.0e-7f, 30);   /* fix rounding errors */
	  if(_list>1) {
		ReportAF(nSrf, encl, "View factors accounting for enclosure:", name, area, emit, base, AF, 0);
	  }
	}
  }

  fprintf(_ulog, "\nFinal view factors:");
  if(vfCtrl.emittances) {
	ReportAF(nSrf, encl, title, name, area, emit, base, AF, 0);
  }
  else {
	ReportAF(nSrf, encl, title, name, area, vtmp, base, AF, 0);
  }

  CPUtime(0.0);
  SaveVF(outFile, PROGRAMSTR, VERSIONSTR, vfCtrl.outFormat, vfCtrl.enclosure,
		 vfCtrl.emittances, nSrf, area, emit, AF, vtmp);
  sprintf(_string, " %.2f seconds to write view factors\n", CPUtime(time1));
  fputs(_string, stderr);
  fputs(_string, _ulog);

#ifdef DEBUG
  fprintf(_ulog, "\nFinal list of surfaces:\n");
  fprintf(_ulog, "   #        name     area  emit\n");
  for(n=1; n<=nSrf; n++) {
	fprintf(_ulog, "%4d %12s %8.3f %5.3f\n", n, name[n], area[n], emit[n]);
  }
#endif

FreeMemory:
  if(vfCtrl.row) {
	Fre_MC(AF, vfCtrl.row, vfCtrl.row, 1, nSrf0, sizeof(R8), __FILE__, __LINE__);
  } else {
	Fre_MSR((void **)AF, 1, nSrf0, sizeof(R8), __FILE__, __LINE__);
  }
  Fre_V(srf, 1, vfCtrl.nAllSrf, sizeof(SRFDAT3D), __FILE__, __LINE__);
  Fre_V(cmbn, 1, nSrf0, sizeof(IX), __FILE__, __LINE__);
  Fre_V(base, 1, nSrf0, sizeof(IX), __FILE__, __LINE__);
  Fre_V(vtmp, 1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  Fre_V(emit, 1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  Fre_V(area, 1, nSrf0, sizeof(R4), __FILE__, __LINE__);
  Fre_MC((void **)name, 1, nSrf0, 0, NAMELEN, sizeof(I1), __FILE__, __LINE__);
  MemRem("After all calculations");

  sprintf(_string, "\n%.2f seconds for all calculations.\n", CPUtime(time0));
  fputs(_string,_ulog);
  fputs(_string,stderr);
  time(&bintime);
  curtime = localtime(&bintime);
  fprintf(_ulog, "Time:  %s", asctime(curtime));
  fprintf(_ulog, "\n**********\n\n");

  fclose(_ulog);

  fprintf(stderr, "\nDone!\n");

  return 0;

  }  /* end of main */

/***  VolPrism.c  ************************************************************/

/*  Compute 6 * volume of a prism defined by vertices a, b, c, and (0,0,0).
 *  Ref: E Kreyszig, _Advanced Engineering Mathematics_, 3rd ed, Wiley, 1972,
 *  pp 214,5.  Volume = A dot (B cross C) / 6; A = vector from 0 to a, ...;
 *  Uses the fact that VECTOR3D A = VERTEX3D a, ...; Sign of result depends
 *  on sequence (clockwise or counter-clockwise) of vertices.   */

R8 VolPrism(VERTEX3D *a, VERTEX3D *b, VERTEX3D *c)
  {
  VECTOR3D bxc;

  VCROSS(b, c, (&bxc));
  return VDOT(a, (&bxc));

  }  /* end of VolPrism */

/***  ReportAF.c  ************************************************************/
/* const removal: const I1 ** name and const R8 ** AF */
void ReportAF(const IX nSrf, const IX encl, const I1 *title, I1 ** name,
			  const R4 *area, const R4 *emit, const IX *base, R8 ** AF,
			  IX flag)
  {
  IX n;    /* row */
  IX m;    /* column */
  R8 err;  /* error values assuming enclosure */
  R8 F, sumF;  /* view factor, sum of F for row */
  R8 eMax=0.0;     /* maximum row error, if enclosure */
  R8 eRMS=0.0;     /* RMS row error, if enclosure */
#define MAXEL 10
  struct {
	R8 err;   /* row sumF error */
	IX n;     /* row number */
	} elist[MAXEL+1];
  IX i;

  fprintf(_ulog, "\n%s\n", title);
  if(encl && _list>0) {
	fprintf(_ulog, "          #        name   SUMj Fij (encl err)\n");
  }
  memset(elist, 0, sizeof(elist));

  for(n=1; n<=nSrf; n++) {    /* process AF values for row n */
	for(sumF=0.0,m=1; m<=n; m++) { /* compute sum of view factors */
	  if(base[m] == 0) {
		sumF += AF[n][m];
	  }
	}
	for(; m<=nSrf; m++) {
	  if(base[m] == 0) {
		sumF += AF[m][n];
	  }
	}
	sumF /= area[n];
	if(_list>0) {
	  fprintf(_ulog, " Row:  %6d, Name: %s, sum of view factors: %9.6f", n, name[n], sumF);
	  if(encl) {
		fprintf(_ulog, " (%.6f)", fabs(sumF - emit[n]));
	  }
	  fputc('\n', _ulog);
	  }

	if(encl) {              /* compute row sumF error value */
	  err = fabs(sumF - emit[n]);
	  eRMS += err * err;
	  for(i=MAXEL; i>0; i--) {
		if(err<=elist[i-1].err) {
		  break;
		}
		elist[i].err = elist[i-1].err;
		elist[i].n = elist[i-1].n;
		}
	  elist[i].err = err;
	  elist[i].n = n;
	  }

	if(_list>0) { /* print row n values */
	  R8 invArea = 1.0 / area[n];
	  for(m=1; m<=nSrf; m++) {
		I1 *s = _string;
		if(m>=n) {
		  F = AF[m][n] * invArea;
		} else {
		  F = AF[n][m] * invArea;
		}
		sprintf(_string, "%8.6f ", F);
		if(_string[0] == '0') {
		  s += 1;
		  if(m%10==0) {
			_string[8] = '\n';
		  }
		  } else {
		  sprintf(_string, "%7.5f ", F);  /* handle F = 1.0 */
		  if(m%10==0) _string[7] = '\n';
		  }
		fprintf(_ulog, "%s", s);
		}
	  if(m%10!=1) {
		fputc('\n', _ulog);
	  }
	  }
	}  /* end of row n */

  if(encl) {   /* print row sumF error summary */
	fprintf(_ulog, "Summary:\n");
	eMax = elist[0].err;
	fprintf(_ulog, "Max row sumF error:  %.2e\n", eMax);
	eRMS = sqrt(eRMS/nSrf);
	fprintf(_ulog, "RMS row sumF error:  %.2e\n", eRMS);
	if(flag) {
	  fprintf(stderr, "\nMax row sumF error:  %.2e\n", eMax);
	  fprintf(stderr, "RMS row sumF error:  %.2e\n", eRMS);
	  }
	if(elist[0].err>0.5e-6) {
	  fprintf(_ulog, "Largest errors [row, error]:\n");
	  for(i=0; i<MAXEL; i++) {
		if(elist[i].err<0.5e-6) {
		  break;
		}
		fprintf(_ulog, "%8d%10.6f\n", elist[i].n, elist[i].err);
		}
	  }
	fprintf(_ulog, "\n");
	}

  }  /* end of ReportAF */

