# CSE 6230, Fall 2014, Lab 5: Tuning GPU reduction #

For instructions, see: https://bitbucket.org/gtcse6230fa14/lab5/wiki/Home

This lab is due before class on Thursday, October 9, 2014.

Implementation
===============

The following implementations implement the cases discussed in class in an
iterative manner. All the implementations are expected to handle non power-of-2
input sizes.

Version 0: Naive
-----------------
The implementation was provided and was used as base for other implementations.

Version 1: Non-divergent threading
-----------------------------------
My implementation for non-divergent kernel borrows almost all of the code
from naive implementation and changes just the target of the reduction
in the loop for preventing the divergence of threads observed in the
naive implementation.
Typical observed performance was ~12 GB/s.

Version 2: Sequential addressing
---------------------------------
The implementation for sequential addressing keeps the buffer initialization
and final result assignment part from the previous versions but changes the
looping structure so that the redution sources are from different halves of
the active array, while also maintaining non-divergence implemented in the
previous version.
Typical observed performance was ~17 GB/s.

Version 3: First add during load
---------------------------------
As the name suggests, the only change in this version from the previous version
is that it does the first reduction during load, thus halving the number of
threads. Rest of the implementation is identical to the previous version.
Typical observed performance was ~30 GB/s.

Version 4: Unroll last warp
----------------------------
Instead of looping all the way down to 0, this implementation loops down only
to 32 and then does reduction for the whole warp in a separate function, where it
does not need any __syncthreads() calls.
Typical observed performance was ~49 GB/s.

Version 5: Unroll entire loop
------------------------------
Instead of a for-loop, the reduce kernel was modified to reduce depending on the
block size and multiple conditional statements were written for handling different
common power-of-2 block sizes.
Typical observed performance was ~49 GB/s.

Version 6: Multiple adds during load
-------------------------------------
Instead of doing just the first add during load, as in Version 3, modified the code
to do multiple adds during the load.
Typical observed performance was ~106 GB/s.
