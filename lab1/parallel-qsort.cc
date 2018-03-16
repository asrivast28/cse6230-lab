/**
 *  \file parallel-qsort.cc
 *
 *  \brief Implement your parallel quicksort algorithm in this file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "sort.hh"
#include <algorithm>
#include <list>
#include <numeric>

#include <cilk/reducer_list.h>

enum Scanned
{
  Left,
  Right,
  Both
};

/**
 *  Given a pivot value, this function scans the left and right blocks,
 *  defined by the input parameters, of the given input array and tries
 *  to swap elements across the pivot until one or both the blocks are
 *  exhausted. It returns an enum value corresponding to whether Left,
 *  Right, or Both blocks were scanned.
 */
Scanned scanBlocks (const keytype pivot, keytype* A, const int leftStart, const int leftEnd, const int rightStart, const int rightEnd)
{
  int leftIndex = leftStart;
  int rightIndex = rightStart;

  while ((leftIndex < leftEnd) && (rightIndex < rightEnd)) {
    while ((leftIndex < leftEnd) && (A[leftIndex] <= pivot)) {
      ++leftIndex;
    }
    while ((rightIndex < rightEnd) && (A[rightIndex] > pivot)) {
      ++rightIndex;
    }
    if ((leftIndex == leftEnd) || (rightIndex == rightEnd)) {
      break;
    }
    std::swap(A[leftIndex++], A[rightIndex++]);
  }
  if ((leftIndex == leftEnd) && (rightIndex == rightEnd)) {
    return Both;
  }
  else if (leftIndex == leftEnd) {
    return Left;
  }
  else {
    return Right;
  }
}

/**
 *  This function moves the blocks which were not completely scanned,
 *  closer to the middle of the given array so that all the unscanned
 *  data can be passed to a recursive call.
 */
int swapBlocks (keytype* A, const int blockSize, const int startIndex, const std::list<int>& remaining, const int jump)
{
  std::list<int>::const_iterator head = remaining.begin();
  std::list<int>::const_iterator tail = remaining.end();
  int index = startIndex;
  while (head != tail) {
    if (*head != index) { 
      int firstIndex = *head * blockSize;
      int secondIndex = index * blockSize;
      for (int i = 0; i < blockSize; ++i) {
        std::swap(A[firstIndex++], A[secondIndex++]);
      }
    }
    index += jump;
    ++head;
  }
  return index - jump;
}

/**
 *  Given a pivot value, this routine partitions a given input array
 *  into two sets: the set A_le, which consists of all elements less
 *  than or equal to the pivot, and the set A_gt, which consists of
 *  all elements strictly greater than the pivot.
 *
 *  This routine overwrites the original input array with the
 *  partitioned output. It also returns the index n_le such that
 *  (A[0:(k-1)] == A_le) and (A[k:(N-1)] == A_gt).
 */
int partition (keytype pivot, int N, keytype* A)
{
  int k = 0;
  for (int i = 0; i < N; ++i) {
    /* Invariant:
     * - A[0:(k-1)] <= pivot; and
     * - A[k:(i-1)] > pivot
     */
    const int ai = A[i];
    if (ai <= pivot) {
      /* Swap A[i] and A[k] */
      int ak = A[k];
      A[k++] = ai;
      A[i] = ak;
    }
  }
  return k;
}

/**
 *  This functions partitions the input array about the given pivot
 *  using parallel recursive calls.
 */
int parallelPartition (keytype pivot, int N, keytype* A, const int G)
{
  // Partition in serial if the array size is below a certain threshold.
  if (N <= G) {
    return partition (pivot, N, A);
  }

  /**
   *  Use the threshold value as block size and divide the array into
   *  blocks of equal length. These blocks will be assigned to threads
   *  for scanning.
   */
  const int blockSize = G;
  int totalBlockCount = N / blockSize;
  int leftBlockCount = (totalBlockCount / 2) + ((totalBlockCount % 2) ? 1 : 0);
  int rightBlockCount = totalBlockCount - leftBlockCount;
  int minBlockCount = std::min(leftBlockCount, rightBlockCount);

  // These lists store indices of the blocks which were not scanned completely.
  cilk::reducer_list_append<int> leftRemainingReducer;
  cilk::reducer_list_append<int> rightRemainingReducer;

  // Each loop iteration will compare a block from left side with a block from right side.
  _Cilk_for (int i = 0; i < minBlockCount; ++i) {
    int leftStart = i * blockSize;
    int rightStart = (leftBlockCount + i) * blockSize;
    Scanned completed = scanBlocks (pivot, A, leftStart, leftStart + blockSize, rightStart, rightStart + blockSize);
    if (completed == Right) {
      leftRemainingReducer.push_back(i);
    }
    else if (completed == Left) {
      rightRemainingReducer.push_back(i + leftBlockCount);
    }
  }

  std::list<int> leftRemaining(leftRemainingReducer.get_value());
  std::list<int> rightRemaining(rightRemainingReducer.get_value());
  if (leftBlockCount > minBlockCount) {
    leftRemaining.push_back(leftBlockCount - 1);
  }
  _Cilk_spawn rightRemaining.sort();
  leftRemaining.sort();
  leftRemaining.reverse();
  _Cilk_sync;

  int n_le = -1;
  if (leftRemaining.empty() && rightRemaining.empty()) {
    n_le = leftBlockCount * blockSize;
  }
  else {
    // Move all the unscanned blocks to middle of the array.
    _Cilk_spawn swapBlocks (A, blockSize, leftBlockCount, rightRemaining, 1);
    int lIndex = swapBlocks (A, blockSize, leftBlockCount - 1, leftRemaining, -1);
    _Cilk_sync;

    // Call this routine again to partition unscanned elements.
    n_le = lIndex * blockSize;
    n_le += parallelPartition (pivot, (leftRemaining.size() + rightRemaining.size()) * blockSize, &A[n_le], G); 
  }
  // This takes care of spillover elements.
  if ((N % blockSize) != 0) {
    for (int i = (totalBlockCount * blockSize); i < N; ++i) {
      if (A[i] <= pivot) {
        std::swap(A[n_le++], A[i]);
      }
    }
  }
  return n_le;
}

void
quickSort (int N, keytype* A)
{
  const int G = 2048; /* base case size, a tuning parameter */
  if (N < G)
    sequentialSort (N, A);
  else {
    // Choose pivot at random
    keytype pivot = A[rand () % N];

    // Partition around the pivot. Upon completion, n_less, n_equal,
    // and n_greater should each be the number of keys less than,
    // equal to, or greater than the pivot, respectively. Moreover, the array
    int n_le = parallelPartition (pivot, N, A, G);
    _Cilk_spawn quickSort (n_le, A);
    quickSort (N-n_le, A + n_le);
    _Cilk_sync;
  }
}

void
parallelSort (int N, keytype* A)
{
  quickSort (N, A);
}

/* eof */
