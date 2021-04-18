/*subfile:  heap.c  ***********************************************************/
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

/*  Functions for heap (memory) processing.
 *  All allocations occur through the Alc_E() function.
 *  Deallocations occur through the corresponding Fre_E() function.
 *  If MEMTEST > 0, guard bytes will be tested during deallocation.
 *  MemNet() is useful to check that all heap has been deallocated.  */

/* MEMTEST: 0 = no tests; 1 = test guard bytes;
 *          2 = test for leaks; 3 = log actions */

//#define MEMTEST 3
//#define MEMTEST 2
//#define MEMTEST 1
#define MEMTEST 0

#include <stdio.h>
#include <string.h> // prototypes: memset, ...
#include <stdlib.h> // prototype: malloc, free
#include <limits.h> // define UINT_MAX
#include "types.h"  // define U1, I2, etc.
#include "prtyp.h"  // miscellaneous function prototypes

extern FILE *_ulog;   // program log file

I4 _bytesAllocated=0L;  // through Alc_E()
I4 _bytesFreed=0L;      // through Fre_E()

#if( MEMTEST > 0 )
#define MCHECK 0x7E7E7E7EL  // 5A='z'; 7E='~'
# if( MEMTEST > 1 )
typedef struct memlist {   // record of memory allocation
  struct memlist *next;   // pointer to next struct
  U1 *pam;    // pointer to allocated memory
  UX length;  // length of allocated variable
  IX line;    // line in source file
  I1 file[1]; // name of source file; allocate for exact length
} MEMLIST;
MEMLIST *_memList=NULL;
# endif
#endif

/***  Alc_E  ******************************************************************/

/*  Allocate memory for a single element, i.e. contiguous portion of the heap.
 *  This may be a single structure or an array. All allocated bytes set to 0.
 *  All memory allocations and de-allocations should go through Alc_E()
 *  and Fre_E() to allow some useful heap checking options.
 *
 *  Define MEMTEST to activate the following print and test operations.
 *  Otherwise, they are not available, but the size of the executable
 *  program is reduced and its speed increased.
 *  When MEMTEST = 1, 4 guard bytes are added before and after the normal
 *  heap memory allocation.  These guard bytes help to test for accessing
 *  beyond the ends of the allocated vector -- especially for off-by-one
 *  indexing.  They will be tested when the allocation is freed or when
 *  Chk-E() is called. Use MemNet() to see if all allocations have been freed.
 *  This is based on idea & code by Paul Anderson, "Dr. Dobb's C Sourcebook",
 *  Winter 1989/90, pp 62 - 66, 94.
 *  When MEMTEST = 2, every call to Alc_E() creates a record of the
 *  allocation in the _memList linked list. When the corresponding
 *  Fre_E() is called that record is deleted from the linked list.
 *  Use MemList() to list all allocations which have not been freed.
 *  When MEMTEST = 3, every call to Alc_E() and Fre_E() is noted in
 *  the LOG file, _ulog, which is created when the program starts.
 *  The old Turbo C++ compiler has some functions to directly test the
 *  heap integrity. They have been placed in MemRem(). Since it was
 *  a 16-bit compiler, tests exist to check for the element size.   */

void *Alc_E(I4 length, I1 *file, IX line)
/*  length; length of element (bytes).
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  U1 *p;     // pointer to allocated memory
#if( MEMTEST > 0 )
  U4 *pt;    // pointer to heap guard bytes
# if( MEMTEST > 1 )
  MEMLIST *pml;
  //I1 *fname;
# endif
#endif

#ifdef __TURBOC__  //  (16-bit limit: 2^16-1 = 65536 = 16384*4 = 8192*8)
  if(length > UINT_MAX - 32) {
    error(3, file, line,"Element too large to allocate: %ld bytes\n", length);
  }
#endif
  if(length < 1) {
    error(3, file, line, "Element too small to allocate: %ld bytes\n", length);
  }

#if( MEMTEST > 0 )
  p = (U1 *)malloc(length+8);
  _bytesAllocated += length+8;
#else
  p = (U1 *)malloc(length);
  _bytesAllocated += length;
#endif

#if( MEMTEST > 1 )
  //fname = sfname( file );
  pml = calloc(sizeof(MEMLIST) + strlen(file), 1);  // note:
  pml->pam = p;            // allocation to store file name;
  pml->length = length;    // no recursive call to Alc_e().
  pml->line = line;
  strcpy(pml->file, file);
  if(_memList) {
    pml->next = _memList;  // stack data structure
  }
  _memList = pml;
# if( MEMTEST > 2 )
  fprintf(_ulog, "Allocate %5ld bytes at [%p] at line %d in %s\n",
          length, p, line, file);
  fflush(_ulog);
# endif
#endif

  if(p == NULL) {
    MemNet("Alc_E error");
    error(3, file, line, "Memory allocation failed for %u bytes\n", length);
  }

#if( MEMTEST > 0 )      // set guard bytes
  pt = (U4 *)p;
  *pt = MCHECK;
  p+= 4;
  pt = (U4 *)(p+length);
  *pt = MCHECK;
#endif

  memset(p, 0, length);  // zero allocated memory

  return (void *)p;

}  /*  end of Alc_E  */

/***  Chk_E  ******************************************************************/

/*  Check guard bytes around memory allocated by Alc_E().
 *  Return non-zero if heap is in error.  */

IX Chk_E(void *pm, UX length, I1 *file, IX line)
/*  *pm;    pointer to allocated memory.
 *  length; length of element (bytes).
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  IX status=0;
#if( MEMTEST > 0 )   // test guard bytes
  U1 *p;     // pointer to allocated memory
  U4 *pt;    // pointer to guard bytes

  p = (U1 *)pm + length;
  pt = (U4 *)p;
  if(*pt != MCHECK) {
    error(2, file, line, "Overrun at end of allocated memory");
    status = 1;
  }
  p = (U1 *)pm - 4;
  pt = (U4 *)p;
  if(*pt != MCHECK) {
    error(2, file, line, "Overrun at start of allocated memory");
    status = 1;
  }
  pm = (void *)p;
#endif

  if(status) {
#if( MEMTEST > 1 )
    MEMLIST *pml;
    for(pml=_memList; pml; pml=pml->next) {
      if(pml->pam == p) { /* report allocation data*/
        if(length != pml->length) {
          error(2, file, line, "Length error: %d vs. %d originally allocated.",
                length, pml->length);
        }
        error(0, file, line, "Memory was allocated at line %d in %s",
              pml->line, pml->file);
        break;
      }
    }
    if(!pml) {
      error(0, file, line,"Allocation not found at [%p]", p);
    }
#endif
#if( _MSC_VER )  // Overrun causes crash in MS C++ 
    error(3, __FILE__, __LINE__, "Fix memory overrun error");
#endif
  }

  return status;

}  /*  end of Chk_E  */

/***  Fre_E  ******************************************************************/

/*  Free pointer to previously memory allocated by Alc_E().
 *  Includes a memory check.  */

void *Fre_E(void *pm, UX length, I1 *file, IX line)
/*  *pm;    pointer to allocated memory.
 *  length; length of element (bytes).
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  U1 *p=(U1 *)pm;     // pointer to allocated memory
#if( MEMTEST > 1 )
  MEMLIST *pml, *pmlt=NULL;
#endif

#if( MEMTEST > 0 )
  Chk_E(pm, length, file, line);
  p -= 4;
# if( MEMTEST > 1 )
  for(pml=_memList; pml; pmlt=pml,pml=pml->next) {
    if(pml->pam == p) {
      if(pmlt) {     // remove pml from linked list
        pmlt->next = pml->next;
      } else {
        _memList = pml->next;
      }
      free(pml);  // free for reuse
      break;
    }
  }
  if(!pml) {
    error(2, file, line, "Failed to find [%p] to free memory", p);
  }
#  if( MEMTEST > 2 )
  fprintf(_ulog, "    Free %5u bytes at [%p] at line %d in %s\n",
          length, p, line, file);
  fflush(_ulog);
#  endif
# endif
#endif

#if( MEMTEST > 0 )
  _bytesFreed += length+8;
#else
  _bytesFreed += length;
#endif
  free(p);

  return (NULL);

}  /*  end of Fre_E  */

/***  MemNet  *****************************************************************/

/*  Report memory allocated and freed.  */

I4 MemNet(I1 *msg)
{
  I4 netBytes=_bytesAllocated-_bytesFreed;

  fprintf(_ulog, "%s: %ld bytes allocated, %ld freed, %ld net\n",
          msg, _bytesAllocated, _bytesFreed, netBytes);
  fflush(_ulog);

  return netBytes;

}  /* end of MemNet */

/***  MemList  ****************************************************************/

/*  Report current allocated memory.  */

void MemList(void)
{
#if( MEMTEST > 1 )
  MEMLIST *pml;

  if(_memList) {
    fprintf(_ulog, "Heap: loc,   size, line, file\n");
    for(pml=_memList; pml; pml=pml->next)
      fprintf(_ulog, "[%p] %6d %5d %s\n",
              pml->pam, pml->length, pml->line, pml->file);
  }
  else {
    fprintf(_ulog, "No unfreed allocations.\n");
  }
#else
  if(_bytesAllocated - _bytesFreed) {
    fprintf(_ulog, "Recompile Heap.c to list unfreed allocations.\n");
  }
#endif
  fflush(_ulog);

}  // end of MemList

#ifdef __TURBOC__   // using old TURBO C compiler
# include <alloc.h> // prototype: heapcheck, heapwalk
#endif

/***  MemRem  *****************************************************************/

/*  Report memory allocated and freed. Tubro C's coreleft() reports the amount
 *  of memory between the highest allocated block and the top of the heap.
 *  Freed lower blocks are not counted as available memory.
 *  heapwalk() shows the details.  */

void MemRem(I1 *msg)
{
#if( __TURBOC__ >= 0x295 )
  struct heapinfo hp;   // heap information
  U4 bytes = coreleft();
  fprintf(_ulog, "%s:\n", msg);
  fprintf(_ulog, "  Unallocated heap memory:  %ld bytes\n", bytes);

# if( MEMTEST > 1 )
  switch(heapcheck()) {
  case _HEAPEMPTY:
    fprintf(_ulog, "The heap is empty.\n");
    break;
  case _HEAPOK:
    fprintf(_ulog, "The heap is O.K.\n");
    break;
  case _HEAPCORRUPT:
    fprintf(_ulog, "The heap is corrupted.\n");
    break;
  }  // end switch

  fprintf(_ulog, "Heap: loc, size, used?\n");
  hp.ptr = NULL;
  while(heapwalk(&hp) == _HEAPOK) {
    fprintf(_ulog, "[%p]%8lu %s\n",
            hp.ptr, hp.size, hp.in_use ? "used" : "free");
  }
# endif
}
#else
  MemNet(msg);  // for non-TurboC code
#endif

}  /* end of MemRem */

#define ANSIOFFSET 1 // 1 = include V[0] in vector allocation

/***  Alc_V  ******************************************************************/

/*  Allocate pointer for a vector with optional debugging data.
 *  This vector is accessed and stored as:
 *     V[J], V[J+1], V[J+2], ..., V[N-1], V[N]
 *  where J is the minimum vector index and N is the maximum index --
 *  J is typically 0 or 1. A pointer to V[0] is returned even if V[0]
 *  was not physically allocated. This is not strictly ANSI compliant.
 *  The index offset idea is from "Numeric Recipes in C" by Press, et.al,
 *  1988. The 2nd edition (1992) includes another offset to guarantee ANSI
 *  compliance for min_index = 1; a more general method is used here which
 *  is activated by #define ANSIOFFSET 1  (applies when min_index > 0)  */

void *Alc_V(IX min_index, I4 max_index, IX size, I1 *file, IX line)
/*  min_index;  minimum vector index:  vector[min_index] valid.
 *  max_index;  maximum vector index:  vector[max_index] valid.
 *  size;   size (bytes) of one data element.
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  I1 *p; /* pointer to the vector */
  I4 length = (max_index - min_index + 1) * size; /* length of vector (bytes) */

  if(length < 1) {
    error(3, file, line,"Max index (%d) < min index (%d)", max_index, min_index);
  }

#if( ANSIOFFSET > 0 )
  if(min_index > 0) {
    length += min_index * size;
  }
  p = (I1 *)Alc_E(length, file, line);
  if(min_index < 0) {
    p -= min_index * size;
  }
#else
  p = (I1 *)Alc_E(length, file, line);
  p -= min_index * size;
#endif

#if( MEMTEST > 0 )
  {
    //# ifdef XXX
    I1 *p1=p+min_index*size-4;  // start of guard bytes
    //# endif
# if( ANSIOFFSET > 0 )
    if(min_index > 0) {
      U4 *pt = (U4 *)p1;
      *pt = MCHECK;  // add guard bytes before v[min_index];
    }
# endif
# ifdef XXX
    fprintf(_ulog, "Alc_V {");  // display allocation
    while(p1<p+(max_index+1)*size+4) { // end of guard bytes
      if(*p1) {
        fprintf(_ulog, "%c", *p1++);
      } else {
        fprintf(_ulog, ".", *p1++);
      }
    }
    fprintf(_ulog, "}\n");
    fflush(_ulog);
# endif
  }
#endif

  return ((void *)p);

}  /*  end of Alc_V  */

#if( MEMTEST > 0 )
/***  Chk_V  ******************************************************************/

/*  Check a vector allocated by Alc_V().  */

void Chk_V(void *v, IX min_index, IX max_index, IX size, I1 *file, IX line)
/*  v;      pointer to allocated vector.
 *  min_index;  minimum vector index.
 *  max_index;  maximum vector index.
 *  size;   size (bytes) of one data element.
 *  name;   name of variable being checked.
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  I1 *p=(I1 *)v;  // pointer to the vector data
  UX length = (UX)(max_index-min_index+1)*size; /* number of bytes in vector data */


#if( ANSIOFFSET > 0 )
  if(min_index > 0) {
    length += min_index * size;
  } else {
    p += min_index * size;
  }
#else
  p += min_index * size;
#endif

  Chk_E((void *)p, length, file, line);

#if( ANSIOFFSET > 0 )
  if(min_index > 0) {
    U4 *pt = (U4 *)(p + min_index* size) - 1;
    if(*pt != MCHECK) { // check guard bytes before v[min_index];
      error(2, file, line, "Overrun before start of vector");
    }
  }
#endif

}  /*  end of Chk_V  */
#endif

/***  Clr_V  ******************************************************************/

/*  Clear (zero all elements of) a vector created by Alc_V().  */

void Clr_V(void *v, IX min_index, IX max_index, IX size, I1 *file, IX line)
/*  min_index;  minimum vector index:  vector[min_index] valid.
 *  max_index;  maximum vector index:  vector[max_index] valid.
 *  size;   size (bytes) of one data element.
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  I1 *pdata = (I1 *)v + min_index * size;
  UX length = (UX)(max_index - min_index + 1) * size;
  memset(pdata, 0, length);

#if( MEMTEST > 0 )
  Chk_V(v, min_index, max_index, size, file, line);
#endif

}  /*  end of Clr_V  */

/***  Fre_V  ******************************************************************/

/*  Free pointer to a vector allocated by Alc_V().  */

void *Fre_V(void *v, IX min_index, IX max_index, IX size, I1 *file, IX line)
/*  v;      pointer to allocated vector.
 *  min_index;  minimum vector index.
 *  max_index;  maximum vector index.
 *  size;   size (bytes) of one data element.
 *  file;   name of file for originating call.
 *  line;   line in file. */
{
  I1 *p=(I1 *)v;  // pointer to the vector data
  UX length = (UX)(max_index-min_index+1)*size; // number of bytes in vector data


#if( ANSIOFFSET > 0 )
  if(min_index > 0) {
    length += min_index * size;
  }
  if(min_index < 0) {
    p += min_index * size;
  }
#else
  p += min_index * size;
#endif

  Fre_E((void *)p, length, file, line);

  return (NULL);

}  /*  end of Fre_V  */

/***  Alc_MC  *****************************************************************/

/*  Allocate (contiguously) a matrix, M[i][j].
 *  This matrix is accessed (and stored) in a rectangular form:
 *  +------------+------------+------------+------------+-------
 *  | [r  ][c  ] | [r  ][c+1] | [r  ][c+2] | [r  ][c+3] |   ...
 *  +------------+------------+------------+------------+-------
 *  | [r+1][c  ] | [r+1][c+1] | [r+1][c+2] | [r+1][c+3] |   ...
 *  +------------+------------+------------+------------+-------
 *  | [r+2][c  ] | [r+2][c+1] | [r+2][c+2] | [r+2][c+3] |   ...
 *  +------------+------------+------------+------------+-------
 *  | [r+3][c  ] | [r+3][c+1] | [r+3][c+2] | [r+3][c+3] |   ...
 *  +------------+------------+------------+------------+-------
 *  |    ...     |    ...     |    ...     |    ...     |   ...
 *  where r is the minimum row index and
 *  c is the minimum column index (both usually 0 or 1).  */

void *Alc_MC(IX min_row_index, IX max_row_index, IX min_col_index,
             IX max_col_index, IX size, I1 *file, IX line)
/*  min_row_index;  minimum row index.
 *  max_row_index;  maximum row index.
 *  min_col_index;  minimum column index.
 *  max_col_index;  maximum column index.
 *  size;   size (bytes) of one data element.
 *  file;   name of file for originating call.
 *  line;   line in file. */
{  // prow - vector of pointers to rows of M
  IX nrow = max_row_index - min_row_index + 1;    // number of rows [i]
  I4 tot_row_index = min_row_index + nrow - 1;    // max prow index
  I1 **prow = (I1 **)Alc_V(min_row_index, tot_row_index, sizeof(I1 *), file, line);

  // pdata - vector of contiguous A[i][j] data values
  IX ncol = max_col_index - min_col_index + 1;    // number of columns [j]
  I4 tot_col_index = min_col_index + nrow*ncol - 1;  // max pdata index
  I1 *pdata = (I1 *)Alc_V(min_col_index, tot_col_index, size, file, line);
  // Note: must have nrow >= 1 and ncol >= 1.
  // If nrow < 1, allocation of prow will fail with fatal error message.
  // If ncol < 1, allocation of pdata will fail since nrow*ncol <= 0.

  IX n, m;
  for(m=0,n=min_row_index; n<=tot_row_index; n++) {
    prow[n] = pdata + ncol*size*m++;  // set row pointers
  }

  return ((void *)prow);

}  /*  end of Alc_MC  */

#if( MEMTEST > 0 )
/***  Chk_MC  *****************************************************************/

/*  Check a matrix allocated by Alc_MC().  */

void Chk_MC(void *m, IX min_row_index, IX max_row_index, IX min_col_index,
            IX max_col_index, IX size, I1 *file, IX line)
{
  IX nrow = max_row_index - min_row_index + 1;
  IX tot_row_index = min_row_index + nrow - 1;
  I1 **prow = (I1 **)m;
  IX ncol = max_col_index - min_col_index + 1;
  IX tot_col_index = min_col_index + nrow*ncol - 1;
  I1 *pdata = prow[min_row_index];

  Chk_V(pdata, min_col_index, tot_col_index, size, file, line);
  Chk_V(prow, min_row_index, tot_row_index, sizeof(I1 *), file, line);

}  /*  end of Chk_MC  */
#endif

/***  Clr_MC  *****************************************************************/

/*  Clear (zero all elements of) a matrix created by Alc_MC().  */

void Clr_MC(void *m, IX min_row_index, IX max_row_index, IX min_col_index,
            IX max_col_index, IX size, I1 *file, IX line)
{
  IX nrow = max_row_index - min_row_index + 1;
  IX ncol = max_col_index - min_col_index + 1;
  IX tot_col_index = min_col_index + nrow*ncol - 1;
  I1 **prow = (I1 **)m;
  I1 *pdata = prow[min_row_index];

  Clr_V(pdata, min_col_index, tot_col_index, size, file, line);

#if( MEMTEST > 0 )
  Chk_MC(m, min_row_index, max_row_index,
         min_col_index, max_col_index, size, file, line);
#endif

}  /*  end of Clr_MC  */

/***  Fre_MC  *****************************************************************/

/*  Free pointer to a matrix allocated by Alc_MC().  */

void *Fre_MC(void *m, IX min_row_index, IX max_row_index, IX min_col_index,
             IX max_col_index, IX size, I1 *file, IX line)
{
  IX nrow = max_row_index - min_row_index + 1;
  IX tot_row_index = min_row_index + nrow - 1;
  I1 **prow = (I1 **)m;
  IX ncol = max_col_index - min_col_index + 1;
  IX tot_col_index = min_col_index + nrow*ncol - 1;
  I1 *pdata = prow[min_row_index];

  Fre_V(pdata, min_col_index, tot_col_index, size, file, line);
  Fre_V(prow, min_row_index, tot_row_index, sizeof(I1 *), file, line);

  return (NULL);

}  /*  end of Fre_MC  */

/***  Alc_MSR  ****************************************************************/

/*  Allocate (by rows) a symmertic matrix.
 *  This matrix is accessed (and stored) in a triangular form:
 *  +------------+------------+------------+------------+-------
 *  | [i  ][i  ] |     -      |     -      |     -      |    -
 *  +------------+------------+------------+------------+-------
 *  | [i+1][i  ] | [i+1][i+1] |     -      |     -      |    -
 *  +------------+------------+------------+------------+-------
 *  | [i+2][i  ] | [i+2][i+1] | [i+2][i+2] |     -      |    -
 *  +------------+------------+------------+------------+-------
 *  | [i+3][i  ] | [i+3][i+1] | [i+3][i+2] | [i+3][i+3] |    -
 *  +------------+------------+------------+------------+-------
 *  |    ...     |    ...     |    ...     |    ...     |   ...
 *  where i is the minimum array index (usually 0 or 1).
 */

void *Alc_MSR(IX min_index, IX max_index, IX size, I1 *file, IX line)
/*  min_index;  minimum vector index:  matrix[min_index][min_index] valid.
 *  max_index;  maximum vector index:  matrix[max_index][max_index] valid.
 *  size;   size (bytes) of one data element.
 *  name;   name of variable being allocated.  */
{
  I1 **p;  /*  pointer to the array of row pointers  */
  IX i;     /* row number */
  IX j;     /* column number */

  /* Allocate vector of row pointers; then allocate individual rows. */

  p = (I1 **)Alc_V(min_index, max_index, sizeof(I1 *), file, line);
  for(j=i=max_index; i>=min_index; i--,j--) {
    p[i] = (I1 *)Alc_V(min_index, j, size, file, line);
  }

  /* Vectors allocated from largest to smallest to allow
     maximum filling of holes in the heap. */

  return ((void *)p);

}  /*  end of Alc_MSR  */

#if( MEMTEST > 0 )
/***  Chk_MSR  ****************************************************************/

/*  Check a symmetric matrix allocated by Alc_MSR().  */

void Chk_MSR(void *m, IX min_index, IX max_index, IX size, I1 *file, IX line)
{
  I1 **p;  /*  pointer to the array of row pointers  */
  IX i;     /* row number */
  IX j;     /* column number */

  p = (I1 **)m;
  for(j=i=min_index; i<=max_index; i++,j++) {
    Chk_V(p[i], min_index, j, size, file, line);
  }
  Chk_V(p, min_index, max_index, sizeof(I1 *), file, line);

}  /*  end of Chk_MSR  */
#endif

/***  Clr_MSR  ****************************************************************/

/*  Clear (zero all elements of) a symmetric matrix created by Alc_MSR().  */

void Clr_MSR(void *m, IX min_index, IX max_index, IX size, I1 *file, IX line)
{
  I1 **p;  /*  pointer to the array of row pointers  */
  IX i;     /* row number */
  IX j;     /* column number */

  p = (I1 **)m;
  for(j=i=min_index; i<=max_index; i++,j++) {
    Clr_V(p[i], min_index, j, size, file, line);
  }

#if( MEMTEST > 0 )
  Chk_MSR(m, min_index, max_index, size, file, line);
#endif

}  /*  end of Clr_MSR  */

/***  Fre_MSR  ****************************************************************/

/*  Free a symmertic matrix allocated by Alc_MSR.  */

void *Fre_MSR(void *v, IX min_index, IX max_index, IX size, I1 *file, IX line)
/*  v;      pointer to allocated vector.
 *  min_index;  minimum vector index:  matrix[min_index][min_index] valid.
 *  max_index;  maximum vector index:  matrix[max_index][max_index] valid.
 *  size;   size (bytes) of one data element.
 *  name;   name of variable being freed.  */
{
  I1 **p;  /*  pointer to the array of row pointers  */
  IX i;     /* row number */
  IX j;     /* column number */

  p = (I1 **)v;
  for(j=i=min_index; i<=max_index; i++,j++) {
    Fre_V(p[i], min_index, j, size, file, line);
  }
  Fre_V(p, min_index, max_index, sizeof(I1 *), file, line);

  return (NULL);

}  /*  end of Fre_MSR  */

/***  DumpV_IX  ***************************************************************/

/*  Dump IX vector to FILE (with format control).  */

void DumpV_IX(IX *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX j;  // vector index
  IX n=0;  // number of values printed

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__, "Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d]", title, jmin, jmax);  // header line
  if(jmax-jmin+2 > nmax) {
    fprintf(file, "\n");  // force data to next line
  }
  for(j=jmin; j<=jmax; j++) {
    fprintf(file, format, v[j]);
    if(++n == nmax) {  // nmax values per row
      n = 0;
      fprintf(file, "\n");
    }
  }
  if(n != 0) { // terminate last line of values
    fprintf(file, "\n");
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpV_IX */

/***  DumpV_R4  ***************************************************************/

/*  Dump R4 vector to FILE (with format control).  */

void DumpV_R4(R4 *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX j,  // vector index
      n=0;  // number of values printed

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__,"Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d]", title, jmin, jmax);  // header line
  if(jmax-jmin+2 > nmax) {
    fprintf(file, "\n");  // force data to next line
  }
  for(j=jmin; j<=jmax; j++) {
    fprintf(file, format, v[j]);
    if(++n == nmax) {  // nmax values per row
      n = 0;
      fprintf(file, "\n");
    }
  }
  if(n != 0) { // terminate last line of values
    fprintf(file, "\n");
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpV_R4 */

/***  DumpV_R8  ***************************************************************/

/*  Dump R8 vector to FILE (with format control).  */

void DumpV_R8(R8 *v, IX jmin, IX jmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX j;  // vector index
  IX n=0;  // number of values printed

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__,"Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d]", title, jmin, jmax);  // header line
  if(jmax-jmin+2 > nmax) {
    fprintf(file, "\n");  // force data to next line
  }
  for(j=jmin; j<=jmax; j++) {
    fprintf(file, format, v[j]);
    if(++n == nmax) { // nmax values per row
      n = 0;
      fprintf(file, "\n");
    }
  }
  if(n != 0) { // terminate last line of values
    fprintf(file, "\n");
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpV_R8 */

/***  DumpM_IX  ***************************************************************/

/*  Dump IX matrix to FILE (with format control).  */

void DumpM_IX(IX **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX i;  // row index
  IX j;  // column index
  IX n;  // number of values printed - current row

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__,"Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d][%d:%d]\nrow %2d:",
          title, rmin, rmax, cmin, cmax, rmin);  // matrix header
  for(i=rmin; i<=rmax; i++) {
    n = 0;
    for(j=cmin; j<=cmax; j++) {
      fprintf(file, format, v[i][j]);
      if(++n == nmax) {  // nmax values per row
        n = 0;
        fprintf(file, "\n");
        if(j < cmax) {
          fprintf(file, "       ");
        }
      }
    }
    if(n != 0) { // terminate line of values
      fprintf(file, "\n");
    }
    if(i < rmax) {    // next row header
      fprintf(file, "row %2d:", i+1);
    }
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpM_IX */

/***  DumpM_R4  ***************************************************************/

/*  Dump R4 matrix to FILE (with format control).  */

void DumpM_R4(R4 **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX i;  // row index
  IX j;  // column index
  IX n;  // number of values printed - current row

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__,"Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d][%d:%d]\nrow %2d:",
          title, rmin, rmax, cmin, cmax, rmin);  // matrix header
  for(i=rmin; i<=rmax; i++) {
    n = 0;
    for(j=cmin; j<=cmax; j++) {
      fprintf(file, format, v[i][j]);
      if(++n == nmax) { // nmax values per row
        n = 0;
        fprintf(file, "\n");
        if(j < cmax) {
          fprintf(file, "       ");
        }
      }
    }
    if(n != 0) { // terminate line of values
      fprintf(file, "\n");
    }
    if(i < rmax) {    // next row header
      fprintf(file, "row %2d:", i+1);
    }
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpM_R4 */

/***  DumpM_R8  ***************************************************************/

/*  Dump R8 matrix to FILE (with format control).  */

void DumpM_R8(R8 **v, IX rmin, IX rmax, IX cmin, IX cmax,
              I1 *title, I1 *format, IX nmax, FILE *file)
{
  IX i;  // row index
  IX j;  // column index
  IX n;  // number of values printed - current row

#ifdef DEBUG
  if(!v) {
    error(3, __FILE__, __LINE__, "NULL vector for %s", title);
  }
  if(!file) {
    error(3, __FILE__, __LINE__, "NULL file for %s", title);
  }
  if(!strchr(format, '%')) {
    error(3, __FILE__, __LINE__,"Invalid format [%s] for ", title);
  }
#endif

  fprintf(file, "%s [%d:%d][%d:%d]\nrow %2d:",
          title, rmin, rmax, cmin, cmax, rmin);  // matrix header
  for(i=rmin; i<=rmax; i++)
  {
    n = 0;
    for(j=cmin; j<=cmax; j++) {
      fprintf(file, format, v[i][j]);
      if(++n == nmax) { // nmax values per row
        n = 0;
        fprintf(file, "\n");
        if(j < cmax) {
          fprintf(file, "       ");
        }
      }
    }
    if(n != 0) { // terminate line of values
      fprintf(file, "\n");
    }
    if(i < rmax) {   // next row header
      fprintf(file, "row %2d:", i+1);
    }
  }
#ifdef DEBUG
  fflush(file);
#endif

}  /* end DumpM_R8 */

