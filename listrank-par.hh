// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-par.hh
 *  \brief Interface for a parallel list ranker.
 */

#if !defined (INC_LISTRANK_PAR_HH)
#define INC_LISTRANK_PAR_HH //!< listrank-par.hh included

#include "listrank.hh"

#if defined (__cplusplus)
extern "C" {
#endif

  /** Returns a short string naming the implementation */
  const char* getImplName__par (void);  

  /**
   *  Opaque, implementation-dependent data type for storing a parallel
   *  list ranking data structure.
   */
  typedef struct ParRankedList_t__ ParRankedList_t;

  /** Returns a new data structure for testing rankList__par(). */
  ParRankedList_t* setupRanks__par (size_t n, const index_t* Next);

  /** A parallel implementation of rankList__seq(); see 'listrank.hh' */
  void computeListRanks__par (ParRankedList_t* L);

  /** Sets up a data structure for testing rankList__par(). */
  const rank_t* getRanks__par (const ParRankedList_t* L);

  /**
   *  Frees parallel list rank data structure, L.
   *
   *  \note After calling this routine, any pointer returned via a
   *  call to getRanks__par(L) is _also_ invalid.
   */
  void releaseRanks__par (ParRankedList_t* L);

#if defined (__cplusplus)
} // extern "C"
#endif
#endif // !defined (INC_LISTRANK_PAR_HH)

// eof
