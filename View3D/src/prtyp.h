/*subfile:  prtyp.h  **********************************************************/
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

#define PROGRAMSTR "View3D"
#define VERSIONSTR "4.0.0"
/* #define DEBUG */

/*  Common Function Prototypes  */

#ifndef COM_FUNCS
#define COM_FUNCS

/* functions in misc.c */
IX error(IX severity, I1 *file, IX line, ...);
I1 *sfname(I1* longfilename);

IX LongCon(I1 *str, I4 *i);
IX IntCon(I1 *str, IX *i);
I1 *IntStr(I4 i);

IX DblCon(I1 *str, R8 *f);
IX FltCon(I1 *str, R4 *f);

void NxtOpen(I1 *file_name, I1 *file, IX line);
void NxtClose(void);
I1 *NxtWord(I1 *str, IX flag, IX maxlen);

I2 ReadI2(IX flag);
IX ReadIX(IX flag);
I4 ReadI4(IX flag);
R4 ReadR4(IX flag);
R8 ReadR8(IX flag);
U1 ReadU1(IX flag);
U2 ReadU2(IX flag);
U4 ReadX4(IX flag);
I4 ReadHMS(IX flag);
IX ReadMD(IX flag);
IX ReadMDx(IX flag);

IX FileCopy(I1 *from_file, I1 *to_file);
void FileNameFix(I1 *filename);
FILE *FileOpen(I1 *prompt, I1 *fileName, I1 *mode, IX flag);
FILE *NewFile(const I1 *filename, const I1 *mode);
FILE *LogFile(I1 *filename);

R4 CPUtime(R4 t1);
void Delay(R4 seconds);
IX max0(IX n1, IX n2);
IX min0(IX n1, IX n2);

I1 *strctr(I1 *s, IX n);
I1 *strcpys(I1 *s, const IX mx, ... );
IX streql(I1 *s1, I1 *s2);

/* functions in heap.c */
void *Alc_E(I4 length, I1 *file, IX line);
IX Chk_E(void *pm, UX, I1 *, IX);
void *Fre_E(void *pm, UX, I1 *, IX);
I4 MemNet(I1 *msg);
void MemList(void);
void MemRem(I1 *msg);

void *Alc_EC(I1 **block, I4 size, I1 *file, IX line);
void *Alc_ECI(I4 size, I1 *, IX);
void Chk_EC(I1 *block, I1 *, IX);
void *Clr_EC(I1 *block);
void *Fre_EC(I1 *block, I1 *, IX);

void *Alc_V(IX min_index, I4 max_index, IX size, I1 *file, IX line);
void Chk_V(void *, IX, IX, IX, I1 *, IX);
void Clr_V(void *, IX, IX, IX, I1 *, IX);
void *Fre_V(void *, IX, IX, IX, I1 *, IX);

void *Alc_MC(IX min_row, IX max_row, IX min_col, IX max_col,
             IX size, I1 *file, IX line);
void Chk_MC(void *, IX, IX, IX, IX, IX, I1 *, IX);
void Clr_MC(void *, IX, IX, IX, IX, IX, I1 *, IX);
void *Fre_MC(void *, IX, IX, IX, IX, IX, I1 *, IX);

void *Alc_MR(IX min_row, IX max_row, IX min_col, IX max_col,
             IX size, I1 *file, IX line);
void Chk_MR(void *, IX, IX, IX, IX, IX, I1 *, IX);
void *Fre_MR(void *, IX, IX, IX, IX, IX, I1 *, IX);

void *Alc_AC(IX min_plane, IX max_plane, IX min_row, IX max_row,
             IX min_col, IX max_col, IX size, I1 *file, IX line);
void Chk_AC(void *, IX, IX, IX, IX, IX, IX, IX, I1 *, IX);
void Clr_AC(void *, IX, IX, IX, IX, IX, IX, IX, I1 *, IX);
void *Fre_AC(void *, IX, IX, IX, IX, IX, IX, IX, I1 *, IX);

void *Alc_MSC(IX min_index, IX max_index, IX size, I1 *file, IX line);
void Chk_MSC(void *m, IX, IX, IX, I1 *, IX);
void Clr_MSC(void *m, IX, IX, IX, I1 *, IX);
void *Fre_MSC(void *m, IX, IX, IX, I1 *, IX);

void *Alc_MSR(IX min_index, IX max_index, IX size, I1 *file, IX line);
void Chk_MSR(void *m, IX, IX, IX, I1 *, IX);
void *Fre_MSR(void *m, IX, IX, IX, I1 *, IX);

void *Alc_MVC(IX min_index, IX max_index, IX *elements, IX size,
              IX *tne, I1 *file, IX line);
void *Chk_MVC(void *m, IX, IX, IX *, IX, I1 *f, IX);
void *Fre_MVC(void *m, IX, IX, IX *, IX, I1 *f, IX);

/* functions in misc.c or heap.c */
void DumpV_IX(IX *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
void DumpV_R4(R4 *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
void DumpV_R8(R8 *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
void DumpM_IX(IX **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
void DumpM_R4(R4 **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
void DumpM_R8(R8 **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file);
#endif

