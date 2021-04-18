/*subfile:  tmpstore.c  *******************************************************/
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
/*                                                                            */
/*  Source file for block storage memory allocation.                          */
/*                                                                            */
/******************************************************************************/
#include "tmpstore.h"

TMPSTORE *newStore(size_t count, size_t size, I1 *file, IX line)
{
  TMPSTORE *s;
  s = (TMPSTORE*) Alc_E(sizeof(TMPSTORE), file, line);
  if(s == NULL) {
    return NULL;
  }
  s->size = size;
  s->Nmax = count;
  s->n = s->nblock = 0;
  s->first = s->last = (TMPBLOCK*) Alc_E(sizeof(TMPBLOCK), file, line);
  if(s->first == NULL) {
    Fre_E(s, sizeof(TMPSTORE), file, line);
    return NULL;
  }
  s->first->memory = (I1*) Alc_E(count * size, file, line);
  if(s->first->memory == NULL) {
    Fre_E(s->first, sizeof(TMPBLOCK), file, line);
    Fre_E(s, sizeof(TMPSTORE), file, line);
    return NULL;
  }
  s->first->next = NULL;
  s->nblock++;
  return s;
}

void *getMemory(TMPSTORE *s, I1 *file, IX line)
{
  TMPBLOCK *next = NULL;
  if(s->n < s->Nmax) {
    return (void*) (s->last->memory + s->size * s->n++);
  }
  s->n = 1;
  next = (TMPBLOCK*) Alc_E(sizeof(TMPBLOCK), file, line);
  if(next == NULL) {
    return 0;
  }
  next->memory = (I1*) Alc_E(s->Nmax * s->size, file, line);
  if(next->memory == NULL) {
    Fre_E(next, sizeof(TMPBLOCK), file, line);
    return NULL;
  }
  s->last->next = next;
  s->last = next;
  s->last->next = NULL;
  s->nblock++;
  return s->last->memory;
}

void cleanStore(TMPSTORE *s, I1 *file, IX line)
{
  TMPBLOCK *next = s->first->next;
  s->first->next = NULL;
  while(next != NULL) {
    s->last = next->next;
    Fre_E(next->memory, s->Nmax * s->size, file, line);
    Fre_E(next, sizeof(TMPBLOCK), file, line);
    next = s->last;
  }
  s->last = s->first;
  memset(s->first->memory, 0, s->Nmax * s->size);
  s->nblock = 1;
  s->n = 0;
  return;
}

void deleteStore(TMPSTORE *s, I1 *file, IX line)
{
  cleanStore(s, file, line);
  Fre_E(s->first->memory, s->Nmax * s->size, file, line);
  Fre_E(s->first, sizeof(TMPBLOCK), file, line);
  Fre_E(s, sizeof(TMPSTORE), file, line);
  return;
}

void summarizeStore(FILE *fp, TMPSTORE *s, I1 *name)
{
  if(name) {
    fprintf(fp,"%s Store Summary:\n",name);
  } else {
    fprintf(fp,"Store Summary:\n");
  }
  fprintf(fp,"\tStructure size: %d\n", s->size);
  fprintf(fp,"\tStructures per block: %d\n", s->Nmax);
  fprintf(fp,"\tNumber of blocks: %d\n", s->nblock);
  fprintf(fp,"\tTotal number of structures allocated: %d\n",
          s->Nmax * s->nblock);
  fprintf(fp,"\tTotal size of of structures allocated: %d\n",
          s->Nmax * s->nblock * s->size);
  fprintf(fp,"\tTotal number of structures returned: %d\n",
          s->Nmax * (s->nblock - 1) + s->n);
  fprintf(fp,"\tTotal size of structures returned: %d\n",
          (s->Nmax * (s->nblock - 1) + s->n) * s->size);
  return;
}
