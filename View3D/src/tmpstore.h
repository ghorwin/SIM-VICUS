/*subfile:  tmpstore.h  *******************************************************/
/*                                                                            */
/*  Copyright (c) 2011 Jason W. DeGraw                                        */
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
#ifndef tmpstore_h
#define tmpstore_h
/*! \file tmpstore.h
 * \brief Header file for block storage memory allocation implementation.
 *
 * This is the header file for a block-based memory storage method.  The basic
 * idea is to avoid allocating lots of little structures individually by
 * allocating a large number all at once (as a block).  Each time a new
 * structure is requested, the current block is checked to see if there are
 * any more available.  If there are not, a new block is allocated.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "prtyp.h"  // miscellaneous function prototypes
/*! \cond NEVER */
typedef struct TMPBLOCK TMPBLOCK; /*!< Simplify declarations */
/*! \endcond */
//! Block storage container structure
/*! This structure contains a chunk of memory and a pointer to the next
 *  block storage structure.  It is not given a first-class place in the
 *  storage scheme and is not intended for end use.
 */
struct TMPBLOCK {
  struct TMPBLOCK *next; /*!< Pointer to the next block structure */
  char *memory;         /*!< Pointer to allocated memory */
};

//! Block allocation storage structure
/*! Container structure that is used to manage a block based storage
 *  strategy.
 */
typedef struct {
  size_t size;     /*!< The size of the structures to be allocated */
  size_t Nmax;     /*!< The maximum number of structures per block */
  size_t n;        /*!< The index of the next structure to hand out */
  TMPBLOCK *first;  /*!< The first block allocated */
  TMPBLOCK *last;   /*!< The last block allocated */
  size_t nblock;   /*!< The number of blocks allocated */
} TMPSTORE;

//! Create and initialize a new store_t struct
/*!
 * \param count the number of structures to be created in each block
 * \param size the size of the structure
 * \param file character string of file in which call occurs
 * \param line line number of the call
 * \return Pointer to new store_t structure
 */
TMPSTORE *newStore(size_t count, size_t size, I1 *file, IX line);
//! Obtain a pointer to memory within a store_t struct
/*!
 * \param s storage structure to obtain memory from
 * \param file character string of file in which call occurs
 * \param line line number of the call
 * \return Pointer to memory
 */
void *getMemory(TMPSTORE *s, I1 *file, IX line);
//! Return a storage structure to unused state
/*!
 * \param s storage structure to deallocate
 * \param file character string of file in which call occurs
 * \param line line number of the call
 */
void cleanStore(TMPSTORE *s, I1 *file, IX line);
//! Clean up and deallocate a storage structure
/*!
 * \param s storage structure to deallocate
 * \param file character string of file in which call occurs
 * \param line line number of the call
 */
void deleteStore(TMPSTORE *s, I1 *file, IX line);
//! Write out a summary of a storage structure
/*!
 * \param fp file pointer to write summary to
 * \param s storage structure to summarize
 * \param name character string name of structures being stored
 */
void summarizeStore(FILE *fp, TMPSTORE *s, I1 *name);
#endif
