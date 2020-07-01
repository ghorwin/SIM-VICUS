/*
 * -----------------------------------------------------------------
 * $Revision: 4272 $
 * $Date: 2014-12-02 11:19:41 -0800 (Tue, 02 Dec 2014) $
 * -----------------------------------------------------------------
 * Programmer: Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 * This is the implementation file for operations to be used by a
 * generic direct linear solver.
 * -----------------------------------------------------------------
 */ 

#include <stdio.h>
#include <stdlib.h>

#include <sundials/sundials_direct.h>
#include <sundials/sundials_math.h>

#define ZERO RCONST(0.0)
#define ONE  RCONST(1.0)

DlsMat NewDenseMat(long int M, long int N)
{
  DlsMat A;
  long int j;

  if ( (M <= 0) || (N <= 0) ) return(NULL);

  A = NULL;
  A = (DlsMat) malloc(sizeof *A);
  if (A==NULL) return (NULL);
  
  A->data = (realtype *) malloc(M * N * sizeof(realtype));
  if (A->data == NULL) {
    free(A); A = NULL;
    return(NULL);
  }
  A->cols = (realtype **) malloc(N * sizeof(realtype *));
  if (A->cols == NULL) {
    free(A->data); A->data = NULL;
    free(A); A = NULL;
    return(NULL);
  }

  for (j=0; j < N; j++) A->cols[j] = A->data + j * M;

  A->M = M;
  A->N = N;
  A->ldim = M;
  A->ldata = M*N;

  A->type = SUNDIALS_DENSE;

  return(A);
}

realtype **newDenseMat(long int m, long int n)
{
  long int j;
  realtype **a;

  if ( (n <= 0) || (m <= 0) ) return(NULL);

  a = NULL;
  a = (realtype **) malloc(n * sizeof(realtype *));
  if (a == NULL) return(NULL);

  a[0] = NULL;
  a[0] = (realtype *) malloc(m * n * sizeof(realtype));
  if (a[0] == NULL) {
    free(a); a = NULL;
    return(NULL);
  }

  for (j=1; j < n; j++) a[j] = a[0] + j * m;

  return(a);
}


DlsMat NewBandMat(long int N, long int mu, long int ml, long int smu)
{
  DlsMat A;
  long int j, colSize;

  if (N <= 0) return(NULL);
  
  A = NULL;
  A = (DlsMat) malloc(sizeof *A);
  if (A == NULL) return (NULL);

  colSize = smu + ml + 1;
  A->data = NULL;
  A->data = (realtype *) malloc(N * colSize * sizeof(realtype));
  if (A->data == NULL) {
    free(A); A = NULL;
    return(NULL);
  }

  A->cols = NULL;
  A->cols = (realtype **) malloc(N * sizeof(realtype *));
  if (A->cols == NULL) {
    free(A->data);
    free(A); A = NULL;
    return(NULL);
  }

  for (j=0; j < N; j++) A->cols[j] = A->data + j * colSize;

  A->M = N;
  A->N = N;
  A->mu = mu;
  A->ml = ml;
  A->s_mu = smu;
  A->ldim =  colSize;
  A->ldata = N * colSize;

  A->type = SUNDIALS_BAND;

  return(A);
}

realtype **newBandMat(long int n, long int smu, long int ml)
{
  realtype **a;
  long int j, colSize;

  if (n <= 0) return(NULL);

  a = NULL;
  a = (realtype **) malloc(n * sizeof(realtype *));
  if (a == NULL) return(NULL);

  colSize = smu + ml + 1;
  a[0] = NULL;
  a[0] = (realtype *) malloc(n * colSize * sizeof(realtype));
  if (a[0] == NULL) {
    free(a); a = NULL;
    return(NULL);
  }

  for (j=1; j < n; j++) a[j] = a[0] + j * colSize;

  return(a);
}

DlsMat NewBTridiagMat(long int NB, long int blocksize)
{
  DlsMat A;
  long int i;

  if (NB <= 0) return(NULL);
  if (blocksize <= 0) return(NULL);
  
  A = NULL;
  A = (DlsMat) malloc(sizeof *A);
  if (A == NULL) return (NULL);

  A->data = NULL;
  /* Allocate memory for three bands with blocks of size BSIZE*BSIZE each) plus workspace of size BSIZE*BSIZE */
  A->ldata = 3 * NB * blocksize * blocksize;
  A->data = (realtype *) malloc( A->ldata * sizeof(realtype));
  if (A->data == NULL) {
    free(A); A = NULL;
    return(NULL);
  }

  A->cols = (realtype **) malloc(3 * NB * sizeof(realtype *));
  /* cols[0] points to lower band */
  A->cols[0] = A->data;
  for (i=1; i<3*NB; ++i)
    A->cols[i] = A->cols[i-1] + blocksize*blocksize;
  /* cols[NB] points to main band */
  /* cols[2*NB] points to upper band */

  A->M = blocksize;
  A->N = NB;

  A->type = SUNDIALS_BTRIDIAG;

  return(A);
}

realtype **newBTridiagMat(long int nb, long int blocksize)
{
  realtype **a;
  long int i;

  if (nb <= 0) return(NULL);
  if (blocksize <= 0) return(NULL);

  a = NULL;
  a = (realtype **) malloc(3 * nb * sizeof(realtype *));
  if (a == NULL) return(NULL);

  a[0] = NULL;
  /* a[0] points to the start of the lower diagonal and the total data array */
  a[0] = (realtype *) malloc( 3 * nb * blocksize * blocksize * sizeof(realtype));
  if (a[0] == NULL) {
    free(a); a = NULL;
    return(NULL);
  }

  for (i=1; i<3*nb; ++i)
    a[i] = a[i-1] + blocksize*blocksize;
  /* a[nb] points to the start of the main diagonal */
  /* a[2*nb] points to the start of the upper diagonal */

  return(a);
}

void DestroyMat(DlsMat A)
{
  free(A->data);  A->data = NULL;
  free(A->cols);
  free(A); A = NULL;
}

void destroyMat(realtype **a)
{
  free(a[0]); a[0] = NULL;
  free(a); a = NULL;
}

int *NewIntArray(int N)
{
  int *vec;

  if (N <= 0) return(NULL);

  vec = NULL;
  vec = (int *) malloc(N * sizeof(int));

  return(vec);
}

int *newIntArray(int n)
{
  int *v;

  if (n <= 0) return(NULL);

  v = NULL;
  v = (int *) malloc(n * sizeof(int));

  return(v);
}

long int *NewLintArray(long int N)
{
  long int *vec;

  if (N <= 0) return(NULL);

  vec = NULL;
  vec = (long int *) malloc(N * sizeof(long int));

  return(vec);
}

long int *newLintArray(long int n)
{
  long int *v;

  if (n <= 0) return(NULL);

  v = NULL;
  v = (long int *) malloc(n * sizeof(long int));

  return(v);
}

realtype *NewRealArray(long int N)
{
  realtype *vec;

  if (N <= 0) return(NULL);

  vec = NULL;
  vec = (realtype *) malloc(N * sizeof(realtype));

  return(vec);
}

realtype *newRealArray(long int m)
{
  realtype *v;

  if (m <= 0) return(NULL);

  v = NULL;
  v = (realtype *) malloc(m * sizeof(realtype));

  return(v);
}

void DestroyArray(void *V)
{ 
  free(V); 
  V = NULL;
}

void destroyArray(void *v)
{
  free(v); 
  v = NULL;
}


void AddIdentity(DlsMat A)
{
  long int i,j;
  realtype ** M;

  switch (A->type) {

  case SUNDIALS_DENSE:
    for (i=0; i<A->N; i++) A->cols[i][i] += ONE;
    break;

  case SUNDIALS_BAND:
    for (i=0; i<A->M; i++) A->cols[i][A->s_mu] += ONE;
    break;

  case SUNDIALS_BTRIDIAG:
    /* special cases implementation, just to demonstrate */
    switch (A->M) {
      case 1 :
        M = A->cols + A->N;
        for (i=0; i<A->N; i++)
          M[i][0] += ONE;
        break;

      case 2 :
        M = A->cols + A->N;
        for (i=0; i<A->N; i++) {
          M[i][0] += ONE;
          M[i][3] += ONE;
        }
        break;

      default :
        /* generic implementation for M >= 3 */
        M = A->cols + A->N;
        for (i=0; i<A->N; i++)              /* i index of main diagonal block */
          for (j=0; j<A->M; j++)            /* j column in block */
            M[i][j*A->M + j] += ONE;        /* A->cols[1] points to main diagonal */
        break;
    } 
    break;

  }

}


void SetToZero(DlsMat A)
{
  long int i, j, colSize;
  realtype *col_j;

  switch (A->type) {

  case SUNDIALS_DENSE:
    
    for (j=0; j<A->N; j++) {
      col_j = A->cols[j];
      for (i=0; i<A->M; i++)
        col_j[i] = ZERO;
    }

    break;

  case SUNDIALS_BAND:

    colSize = A->mu + A->ml + 1;
    for (j=0; j<A->M; j++) {
      col_j = A->cols[j] + A->s_mu - A->mu;
      for (i=0; i<colSize; i++)
        col_j[i] = ZERO;
    }

    break;


  case SUNDIALS_BTRIDIAG:
    for (i=0; i<A->ldata; ++i) 
      A->data[i] = ZERO;

    break;

  } /* switch */
}


void PrintMat(DlsMat A)
{
  long int i, j, start, finish;
  realtype **a;
  realtype * block;

  switch (A->type) {

  case SUNDIALS_DENSE:

    printf("\n");
    for (i=0; i < A->M; i++) {
      for (j=0; j < A->N; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        printf("%12Lg  ", DENSE_ELEM(A,i,j));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        printf("%12g  ", DENSE_ELEM(A,i,j));
#else
        printf("%12g  ", DENSE_ELEM(A,i,j));
#endif
      }
      printf("\n");
    }
    printf("\n");
    
    break;

  case SUNDIALS_BAND:

    a = A->cols;
    printf("\n");
    for (i=0; i < A->N; i++) {
      start = SUNMAX(0,i-A->ml);
      finish = SUNMIN(A->N-1,i+A->mu);
      for (j=0; j < start; j++) printf("%12s  ","");
      for (j=start; j <= finish; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        printf("%12Lg  ", a[j][i-j+A->s_mu]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        printf("%12g  ", a[j][i-j+A->s_mu]);
#else
        printf("%12g  ", a[j][i-j+A->s_mu]);
#endif
      }
      printf("\n");
    }
    printf("\n");
    
    break;


  case SUNDIALS_BTRIDIAG:

    printf("\n");
    for (i=0; i < A->N*A->M; i++) {		/* Loop over all rows */
      /* print leading spaces for all blocks prior to this current row */
      start = SUNMAX(0, ((i / A->M) -1)*A->M);
      finish = SUNMIN(A->N*A->M, ((i / A->M) + 2)*A->M);
      /* special case, last block-row, only print leading spaces up to column */
      if (i / A->M == A->N - 1) {
        for (j=0; j < start - A->M; j++) printf("%12s  ","");
        /* print elements of U_N */
        block = BTRIDIAG_UPPER(A, A->N-1);
        for (j=0; j < A->M; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
          printf("%12Lg  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#elif defined(SUNDIALS_DOUBLE_PRECISION)
          printf("%12g  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#else
          printf("%12g  ",  BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#endif
        }
      }
      else
        for (j=0; j < start; j++) printf("%12s  ","");
      for (j=start; j < finish; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        printf("%12Lg  ", BTRIDIAG_ELEM(A,i,j));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        block = BTRIDIAG_BLOCK(A,i,j); 
        printf("%12g  ", BTRIDIAG_ELEM(A,i,j));
#else
        printf("%12g  ", BTRIDIAG_ELEM(A,i,j));
#endif
      }
      /* special case, first block-row, print third block */
      if (i / A->M == 0) {
        /* print elements of L_1 */
        block = BTRIDIAG_LOWER(A, 0);
        for (j=0; j < A->M; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
          printf("%12Lg  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#elif defined(SUNDIALS_DOUBLE_PRECISION)
          printf("%12g  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#else
          printf("%12g  ",  BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#endif
        }
      }
      printf("\n");
    }
    printf("\n");
    
    break;
  }

}


void SaveMat(DlsMat A, const char* const fname)
{
  long int i, j, start, finish;
  realtype **a;
  realtype * block;
  FILE * f;
  
  f = fopen(fname, "w");

  switch (A->type) {

  case SUNDIALS_DENSE:

    fprintf(f,"\n");
    for (i=0; i < A->M; i++) {
      for (j=0; j < A->N; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        fprintf(f,"%12Lg  ", DENSE_ELEM(A,i,j));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        fprintf(f,"%12g  ", DENSE_ELEM(A,i,j));
#else
        fprintf(f,"%12g  ", DENSE_ELEM(A,i,j));
#endif
      }
      fprintf(f,"\n");
    }
    fprintf(f,"\n");
    
    break;


  case SUNDIALS_BAND:

    a = A->cols;
    fprintf(f,"\n");
    for (i=0; i < A->N; i++) {
      start = SUNMAX(0,i-A->ml);
      finish = SUNMIN(A->N-1,i+A->mu);
      for (j=0; j < start; j++) fprintf(f,"%12s  ","0");
      for (j=start; j <= finish; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        fprintf(f,"%12Lg  ", a[j][i-j+A->s_mu]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        fprintf(f,"%12g  ", a[j][i-j+A->s_mu]);
#else
        fprintf(f,"%12g  ", a[j][i-j+A->s_mu]);
#endif
      }
      fprintf(f,"\n");
    }
    fprintf(f,"\n");
    
    break;


  case SUNDIALS_BTRIDIAG:

    fprintf(f,"\n");
    for (i=0; i < A->N*A->M; i++) {		/* Loop over all rows */
      /* print leading spaces for all blocks prior to this current row */
      start = SUNMAX(0, ((i / A->M) -1)*A->M);
      finish = SUNMIN(A->N*A->M, ((i / A->M) + 2)*A->M);
      /* special case, last block-row, only print leading spaces up to column */
      if (i / A->M == A->N - 1) {
        for (j=0; j < start - A->M; j++) fprintf(f,"%12s  ","0");
        /* print elements of U_N */
        block = BTRIDIAG_UPPER(A, A->N-1);
        for (j=0; j < A->M; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
          fprintf(f,"%12Lg  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#elif defined(SUNDIALS_DOUBLE_PRECISION)
          fprintf(f,"%12g  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#else
          fprintf(f,"%12g  ",  BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#endif
        }
      }
      else
        for (j=0; j < start; j++) fprintf(f,"%12s  ","0");
      for (j=start; j < finish; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
        fprintf(f,"%12Lg  ", BTRIDIAG_ELEM(A,i,j));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
        block = BTRIDIAG_BLOCK(A,i,j); 
        fprintf(f,"%12g  ", BTRIDIAG_ELEM(A,i,j));
#else
        fprintf(f,"%12g  ", BTRIDIAG_ELEM(A,i,j));
#endif
      }
      /* special case, first block-row, print third block */
      if (i / A->M == 0) {
        /* print elements of L_1 */
        block = BTRIDIAG_LOWER(A, 0);
        for (j=0; j < A->M; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
          fprintf(f,"%12Lg  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#elif defined(SUNDIALS_DOUBLE_PRECISION)
          fprintf(f,"%12g  ", BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#else
          fprintf(f,"%12g  ",  BTRIDIAG_BLOCK_ELEM(A, block, i % A->M, j % A->M) );
#endif
        }
      }
      fprintf(f,"\n");
    }
    fprintf(f,"\n");
    
    break;
  }
  fclose(f);
}
