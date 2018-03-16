// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-cuda.cu
 *
 *  \brief Implement the 'listrank-par.hh' interface using CUDA.
 */

#include <cassert>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "listrank-par.hh"

#include "cuda_utils.h"

#define cudaDeviceScheduleBlockingSync 0x04


using namespace std;

// ============================================================
const char *
getImplName__par (void)
{
  return "CUDA";
}

// ============================================================

struct ParRankedList_t__
{
  size_t n;

  // Buffers on the host (i.e., CPU)
  const index_t* Next_host;
  rank_t* Rank_host;

  // Buffers on the device (i.e., GPU)
  index_t* Next_device[2];
  rank_t* Rank_device[2];
};

// ============================================================

ParRankedList_t *
setupRanks__par (size_t n, const index_t* Next)
{
  ParRankedList_t* L = new ParRankedList_t;
  assert (L);

  L->n = n;
  L->Next_host = Next;
  L->Rank_host = createRanksBuffer (n);

  // Create buffers on the GPU:
  for (size_t i = 0; i < 2; ++i) {
    CUDA_CHECK_ERROR (cudaMalloc (&(L->Next_device[i]), n * sizeof (index_t)));
    CUDA_CHECK_ERROR (cudaMalloc (&(L->Rank_device[i]), n * sizeof (rank_t)));
  }


  // Copy CPU buffer contents to the GPU:
  CUDA_CHECK_ERROR (cudaMemcpy (L->Next_device[0], L->Next_host,
                                n * sizeof (index_t),
                                cudaMemcpyHostToDevice));
  return L;
}

void releaseRanks__par (ParRankedList_t* L)
{
  if (L) {
    releaseRanksBuffer (L->Rank_host);

    // Free GPU buffers:
    for (size_t i = 0; i < 2; ++i) {
      CUDA_CHECK_ERROR (cudaFree (L->Next_device[i]));
      CUDA_CHECK_ERROR (cudaFree (L->Rank_device[i]));
    }
    L->Next_host = NULL;
    L->Rank_host = NULL;
  }
}

// ============================================================

const rank_t *
getRanks__par (const ParRankedList_t* L)
{
  // Copy GPU results back to the CPU:
  CUDA_CHECK_ERROR (cudaMemcpy (L->Rank_host, L->Rank_device[0],
                                L->n * sizeof (rank_t),
                                cudaMemcpyDeviceToHost));
  return L->Rank_host;
}

// ============================================================

/**
 *  Implements a GPU kernel to initialize the rank buffer. By analogy,
 *  see the first for loop in both the sequential and Cilk Plus
 *  implementations.
 */
__global__ void
initListRanks__init (const size_t n, rank_t* Rank, const index_t* Next)
{
  const unsigned int BID = blockIdx.y * gridDim.x + blockIdx.x; // block ID
  const unsigned int LID = threadIdx.x;  // local thread ID
  const int TPB = blockDim.x; // threads per block
  const unsigned int k = BID * TPB + LID;

  if (k < n)
    Rank[k] = (Next[k] == NIL) ? 0 : 1;
}

__global__ void
computeListRanks__init (ParRankedList_t L)
{
  const unsigned int BID = blockIdx.y * gridDim.x + blockIdx.x; // block ID
  const unsigned int LID = threadIdx.x;  // local thread ID
  const int TPB = blockDim.x; // threads per block
  const unsigned int k = BID * TPB + LID;

  if (k >= L.n) {
    return;
  }

  index_t* N_cur = L.Next_device[0];
  index_t* N_next = L.Next_device[1];

  rank_t* R_cur = L.Rank_device[0];
  rank_t* R_next = L.Rank_device[1];

  if (N_cur[k] != NIL) {
    R_next[k] = R_cur[k] + R_cur[N_cur[k]];
    N_next[k] = N_cur[N_cur[k]];
  }
  else {
    R_next[k] = R_cur[k];
    N_next[k] = NIL;
  }
}

void
computeListRanks__par (ParRankedList_t* L)
{
  assert (L != NULL);
  if (L->n == 0) return; // empty pool

  static const int MAX_THREADS_PER_BLOCK = 1024;
  static const int GB_X = 16;
  int blocks = (L->n + MAX_THREADS_PER_BLOCK - 1) / MAX_THREADS_PER_BLOCK;
  dim3 GB (GB_X, (blocks + GB_X - 1) / GB_X, 1);
  dim3 TB (MAX_THREADS_PER_BLOCK, 1, 1);

  size_t maxIterations = static_cast<size_t>(ceil(log2(static_cast<double>(L->n)))); 

  initListRanks__init<<<GB, TB>>> (L->n, L->Rank_device[0], L->Next_device[0]);

  for (size_t it = 0; it < maxIterations; ++it) {
    computeListRanks__init<<<GB, TB>>> (*L);

    index_t* N_temp = L->Next_device[0];
    L->Next_device[0] = L->Next_device[1];
    L->Next_device[1] = N_temp;

    rank_t* R_temp = L->Rank_device[0];
    L->Rank_device[0] = L->Rank_device[1];
    L->Rank_device[1] = R_temp;
  }
  cudaDeviceSynchronize();
}

// eof
