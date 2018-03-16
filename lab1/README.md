My implementation of partition in parallel loosely follows one of the approaches
described in the paper titled "Parallel Partition Revisited" by Frias et al.

My approach consists of following steps:

1) Divide the array into blocks and then divide all the blocks into
   'left' and 'right' blocks. Assign one 'left' and one 'right' block to each
   thread. Each thread will then scan its blocks and complete scanning of
   at least one block.

2) Accumulate all the unscanned blocks in step 1) near the middle of the array
   and then call the original scanning routine on this accumulated section.

3) Partition sequentially when the element count reaches below a certain threshold.
