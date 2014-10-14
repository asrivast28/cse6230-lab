/**
 *  \file mm-cuda.cu
 *  \brief CUBLAS-based implementation of the local matrix multiply.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cuda_runtime.h>
#include "cublas_v2.h"

static int isInit__ = 0; /*!< Set to '1' when handle is initialized, '-1' on error */
static cublasHandle_t handle__;

extern void sgemm_ (const char* opA, const char* opB,
		    const int* M, const int* N, const int* K,
		    const float* alpha, const float* A, const int* lda,
		    const float* B, const int* ldb,
		    const float* beta, float* C, const int* ldc);

static void mm_init (void);
static void assert_status (cublasStatus_t stat);

/* ======================================================================
 * Modify the three routines in this section (mm_local, mm_create, and
 * mm_free) to improve the efficiency of the GPU implementation.
 */

extern "C"
void
mm_local (const int m, const int n, const int k,
	  const float* A_cpu_pinned, const int lda,
	  const float* B_cpu_pinned, const int ldb,
	  float* C_cpu_pinned, const int ldc)
{
  mm_init ();
  assert (A_cpu_pinned || m <= 0 || k <= 0); assert (lda >= m);
  assert (B_cpu_pinned || k <= 0 || n <= 0); assert (ldb >= k);
  assert (C_cpu_pinned || m <= 0 || n <= 0); assert (ldc >= m);

  float* A_gpu;
  cudaMalloc ((void **)&A_gpu, m * k * sizeof (float)); assert (A_gpu);
  float* B_gpu;
  cudaMalloc ((void **)&B_gpu, k * n * sizeof (float)); assert (B_gpu);
  float* C_gpu;
  cudaMalloc ((void **)&C_gpu, m * n * sizeof (float)); assert (C_gpu);
  cudaMemcpy (A_gpu, A_cpu_pinned, m * k * sizeof (float), cudaMemcpyHostToDevice);
  cudaMemcpy (B_gpu, B_cpu_pinned, k * n * sizeof (float), cudaMemcpyHostToDevice);
  cudaMemcpy (C_gpu, C_cpu_pinned, m * n * sizeof (float), cudaMemcpyHostToDevice);

  const float ONE = 1.0;
  cublasStatus_t stat = cublasSgemm (handle__, CUBLAS_OP_N, CUBLAS_OP_N,
				     m, n, k,
				     &ONE, A_gpu, lda, B_gpu, ldb,
				     &ONE, C_gpu, ldc);
  assert_status (stat);

  cudaMemcpy (C_cpu_pinned, C_gpu, m * n * sizeof (float), cudaMemcpyDeviceToHost);
  cudaFree (C_gpu);
  cudaFree (B_gpu);
  cudaFree (A_gpu);
}

extern "C"
float *
mm_create (int m, int n)
{
  float* A_cpu_pinned = NULL;
  cudaHostAlloc ((void **)&A_cpu_pinned, m * n * sizeof(float), cudaHostAllocMapped | cudaHostAllocPortable);
  assert (A_cpu_pinned);
  return A_cpu_pinned;
}

extern "C"
void
mm_free (float* A_cpu_pinned)
{
  if (A_cpu_pinned) cudaFreeHost (A_cpu_pinned);
}

/* ====================================================================== */

static
void
mm_init (void)
{
  if (!isInit__) {
    cublasStatus_t stat = cublasCreate (&handle__);
    if (stat != CUBLAS_STATUS_SUCCESS) {
      fprintf (stderr, "*** CUBLAS initialization failure!\n");
      exit (-1);
    }
    isInit__ = 1;
  }
}

static
void
assert_status (cublasStatus_t stat)
{
  if (stat != CUBLAS_STATUS_SUCCESS) {
    switch (stat) {
    case CUBLAS_STATUS_NOT_INITIALIZED:
      fprintf (stderr, "CUBLAS_STATUS_NOT_INITIALIZED\n");
      break;
    case CUBLAS_STATUS_INVALID_VALUE:
      fprintf (stderr, "CUBLAS_STATUS_INVALID_VALUE\n");
      break;
    case CUBLAS_STATUS_ARCH_MISMATCH:
      fprintf (stderr, "CUBLAS_STATUS_ARCH_MISMATCH\n");
      break;
    case CUBLAS_STATUS_EXECUTION_FAILED:
      fprintf (stderr, "CUBLAS_STATUS_EXECUTION_FAILED\n");
      break;
    default:
      fprintf (stderr, "(unknown error)\n");
      break;
    }
  }
}

/* eof */
