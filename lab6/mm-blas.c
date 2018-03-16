/**
 *  \file mm-blas.c
 *  \brief BLAS-based implementation of the local matrix multiply.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern void sgemm_ (const char* opA, const char* opB,
		    const int* M, const int* N, const int* K,
		    const float* alpha, const float* A, const int* lda,
		    const float* B, const int* ldb,
		    const float* beta, float* C, const int* ldc);
void
mm_local (const int m, const int n, const int k,
	  const float* A, const int lda,
	  const float* B, const int ldb,
	  float* C, const int ldc)
{
  assert (A || m <= 0 || k <= 0); assert (lda >= m);
  assert (B || k <= 0 || n <= 0); assert (ldb >= k);
  assert (C || m <= 0 || n <= 0); assert (ldc >= m);

  const float ONE = 1.0;
  sgemm_ ("N", "N", &m, &n, &k, &ONE, A, &lda, B, &ldb, &ONE, C, &ldc);
}

float *
mm_create (int m, int n)
{
  float* A = (float *)malloc (m * n * sizeof (float));
  assert (A);
  return A;
}

void
mm_free (float* A)
{
  if (A) free (A);
}

/* eof */
