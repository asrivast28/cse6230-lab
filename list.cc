// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file list.cc
 *  \brief Implements the list.hh interface.
 */

/* ====================================================================== */

index_t *
duplicate (int n, const index_t* A)
{
  index_t* B = new index_t[n]; assert (B);
  memcpy (B, A, n * sizeof (index_t));
  return B;
}

/* ====================================================================== */

index_t *
createRandomList (int n)
{
  // Create an initial linked list where each node i points to i+1.
  index_t* Init = new index_t[n]; assert (Init);
  for (int i = 0; i < n; ++i)
    Init[i] = i + 1;
  Init[n-1] = NIL; // "NULL" pointer

  // Remap node i > 0 to position AddrMap[i].
  index_t* AddrMap = new index_t[n]; assert (AddrMap);
  for (int i = 0; i < n; ++i)
    AddrMap[i] = i;
  shuffle (n-1, AddrMap+1);

  // Create final list
  index_t* Next = new index_t[n]; assert (Next);
  for (int i = 0; i < n; ++i)
    Next[AddrMap[i]] = Init[i] > 0 ? AddrMap[Init[i]] : NIL;

  delete[] AddrMap;
  delete[] Init;
  return Next;
}

/* ====================================================================== */

/** Generates a uniform random permutation of an array */
static void
shuffle (int n, index_t* A)
{
  // Implements the Fisher-Yates (Knuth) algorithm:
  // http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
  for (int i = 0; i < (n-1); ++i)
    swap (A[i], A[i+1+(lrand48 () % (n-i-1))]);
}

void
printList (const string& tag, const rank_t* Rank, const index_t* Next,
           index_t head, index_t truncate)
{
  index_t count = 0;

  cerr << "=== " << tag << " ===" << endl;
  index_t cur_node;

  cerr << "  Rank: [";
  cur_node = head;
  count = 0;
  if (cur_node == NIL)
    cerr << " (empty)";
  else
    do {
      if (!truncate || ++count <= truncate) {
	cerr << ' ' << Rank[cur_node];
	cur_node = Next[cur_node];
      } else {
	cerr << " ...";
	break;
      }
    } while (cur_node != NIL);
  cerr << " ]" << endl;

  cerr << "  Next: [";
  cur_node = head;
  count = 0;
  if (cur_node == NIL)
    cerr << " (empty)";
  else
    do {
      if (!truncate || ++count <= truncate) {
	cerr << ' ' << Next[cur_node];
	cur_node = Next[cur_node];
      } else {
	cerr << " ...";
	break;
      }
    } while (cur_node != NIL);
  cerr << " ]" << endl;
}

// eof
