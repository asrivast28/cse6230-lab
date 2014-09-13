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
  index_t* Next_device;
  rank_t* Rank_device;
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
  CUDA_CHECK_ERROR (cudaMalloc (&(L->Next_device), n * sizeof (index_t)));
  CUDA_CHECK_ERROR (cudaMalloc (&(L->Rank_device), n * sizeof (rank_t)));

  // Copy CPU buffer contents to the GPU:
  CUDA_CHECK_ERROR (cudaMemcpy (L->Next_device, L->Next_host,
                                n * sizeof (index_t),
                                cudaMemcpyHostToDevice));
  CUDA_CHECK_ERROR (cudaMemcpy (L->Rank_device, L->Rank_host,
                                n * sizeof (rank_t),
                                cudaMemcpyHostToDevice));

  return L;
}

void releaseRanks__par (ParRankedList_t* L)
{
  if (L) {
    releaseRanksBuffer (L->Rank_host);

    // Free GPU buffers:
    CUDA_CHECK_ERROR (cudaFree (L->Next_device));
    CUDA_CHECK_ERROR (cudaFree (L->Rank_device));
  }
}

// ============================================================

const rank_t *
getRanks__par (const ParRankedList_t* L)
{
  // Copy GPU results back to the CPU:
  CUDA_CHECK_ERROR (cudaMemcpy (L->Rank_host, L->Rank_device,
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
computeListRanks__init (size_t n, const index_t* Next, rank_t* Rank)
{
  const unsigned int BID = blockIdx.y * gridDim.x + blockIdx.x; // block ID
  const unsigned int LID = threadIdx.x;  // local thread ID
  const int TPB = blockDim.x; // threads per block
  const unsigned int k = BID * TPB + LID;
  if (k < n)
    Rank[k] = (Next[k] == NIL) ? 0 : 1;
}

//#include "soln--cuda1.cu" // Instructor's solution: none for you!

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

  computeListRanks__init<<<GB, TB>>> (L->n, L->Next_device, L->Rank_device);

  //------------------------------------------------------------
  //
  // ... YOUR CODE GOES HERE ...
  //
  // (you may also modify the preceding code if you wish)
  //
  //#include "soln--cuda2.cu"  // Instructor's solution: none for you!
  //------------------------------------------------------------
}

// eof
