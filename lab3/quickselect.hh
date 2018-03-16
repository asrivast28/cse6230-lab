// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file quickselect.hh
 *
 *  \author Nicolas Devillard (1998), with C++ adaptions.
 *
 *  \brief This Quickselect routine is based on the algorithm
 *  described in "Numerical recipes in C", Second Edition, Cambridge
 *  University Press, 1992, Section 8.5, ISBN 0-521-43108-5 This code
 *  by Nicolas Devillard - 1998. Public domain. See also:
 *  http://ndevilla.free.fr/median/median/
 */

#if !defined (INC_QUICKSELECT_HH)
#define INC_QUICKSELECT_HH //!< quickselect.hh included

#include <cassert>
#include <cstdlib>
#include <algorithm>

template <typename ELEM_T>
ELEM_T
selectMedian (ELEM_T* arr, size_t n)
{
  size_t low, high;
  size_t median;
  size_t middle, ll, hh;

  low = 0 ; high = n-1 ; median = (low + high) / 2;
  for (;;) {
    if (high <= low) /* One element only */
      break;
    
    if (high == low + 1) {  /* Two elements only */
      if (arr[low] > arr[high])
        std::swap (arr[low], arr[high]) ;
      break;
    }
    
    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    std::swap (arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       std::swap (arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     std::swap (arr[middle], arr[low]) ;
    
    /* Swap low item (now in position middle) into position (low+1) */
    std::swap (arr[middle], arr[low+1]) ;
    
    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
      do ll++; while (arr[low] > arr[ll]) ;
      do hh--; while (arr[hh]  > arr[low]) ;
      
      if (hh < ll)
        break;
      
      std::swap (arr[ll], arr[hh]) ;
    }
    
    /* Swap middle item (in position low) back into correct position */
    std::swap (arr[low], arr[hh]) ;
    
    /* Re-set active partition */
    if (hh <= median)
      low = ll;
    if (hh >= median)
      high = hh - 1;
  }

  assert (median < n);
  return arr[median];
}

#endif //!< INC_QUICKSELECT_HH

// eof
