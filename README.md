CSE 6230, Fall 2014: Lab 3 -- CUDA
==================================

> Due: Sep 23, 2014 @ 4:35pm (just before class)

For instructions, see: https://bitbucket.org/gtcse6230fa14/lab3/wiki/Home
==========================================================================


Implementation
==============

Algorithm
----------
I used Wyllie's algorithm for parallel list ranking in both Cilk Plus as well
as CUDA implementation.

Cilk Plus
----------
For Cilk Plus implementation, I created two arrays each for storing next indices
and rank. I copied the original next indices in one of the next arrays during the
setup process.

During compute part,  I first initialized one of the rank arrays and then
started computations as per Wyllie's algorithm over log2(N) iterations. After
each iteration, I switched the rank and next pointers. The current rank array
contains computed ranks at the end of the for loop.

My implementation gives an effective bandwidth of about ~0.13 GB/s.


CUDA
-----
For CUDA implementation, I followed an approach similar to the approach for 
Cilk Plus implementation. I allocated two next and rank arrays on the device
and used them for storing results of ranks computed in each step and used
pointer switching.

I launched kernel for initializing ranks and then a further log2(N) kernel
launches for computing rank, step by step, as per Wyllie's algorithm.

I also called cudaDeviceSynchronize() before exiting computeListRanks__par
in order to ensure that all the kernels have finished executing.

My implementation gives an effective bandwith of about ~0.31 GB/s.
