// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-mt.hh
 *  \brief Interface for a multithreaded list ranking kernel.
 */

#if !defined (INC_LISTRANK_MT_HH)
#define INC_LISTRANK_MT_HH //!< listrank-mt.hh included

#include "listrank.hh"

/** Multithreaded implementation of rankList__seq (); see 'listrank.hh' */
extern "C" void rankList__mt (int n, rank_t* Rank, const index_t* Next);

#endif // !defined (INC_LISTRANK_MT_HH)

// eof
