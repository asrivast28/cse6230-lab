===================================================
CSE 6230, Fall 2014: Lab 4: Broadcast algorithms (DUE: Th Oct 2)
===================================================

* Info on the Jinx cluster: http://support.cc.gatech.edu/facilities/instructional-labs/jinx-cluster
* MPI PDF slides: http://vuduc.org/cse6230/slides/cse6230-fa14--06-mpi.pdf

In this hands-on lab, you will use Message Passing Interface (MPI) point-to-point operations to implement the two broadcast algorithms we discussed in the previous class. The first of these uses a tree-based approach, which is latency-optimal; the second uses a scatter + all-gather approach, which is bandwidth-optimal (to within a constant factor).

    Recall that MPI provides a portable interface for parallel tasks running on different nodes of a cluster or supercomputer to send messages to each other.

As usual, you may work in teams of up to two people per team. Be sure to include a README in your submitted repository that says with whom you worked, and which answers the questions below. Each team member should submit his or her own repo, but you may submit identical codes and README files. And per the usual class policy, late assignments will not be accepted.


Part 0a: Setting up MPI for your environment
==================================

Start by logging into Jinx and setting up your environment so that the code we are providing will compile and run. In particular, this lab's experiments rely on a particular version of the *GCC* compiler as well as the *MPI* communication library discussed in the previous class.

Set up your shell environment for compiling and running MPI programs. Add the following lines to your ``.bashrc`` file in your home directory. (Go ahead and create this file if it does not already exist.)

.. sourcecode :: bash

    # Add these lines to ~/.bashrc, if not there already:
    if [ -f /etc/bashrc ]; then
      source /etc/bashrc
    fi

    # Add this line for MPI (+ gcc 4.7.2) support
    source /nethome/rvuduc3/local/jinx/setup-mpi.sh


You may also need to add the following line to ``.bash_profile``, also in your home directory. (Again, create this file if it does not already exist.)

.. sourcecode :: bash

    # Add these lines to ~/.bash_profile, if not there already:
    if [ -f ~/.bashrc ] ; then
      source ~/.bashrc
    fi

Once you've made these changes, log out and log back in for this setup to take effect. If it worked, then the command

.. sourcecode :: bash

    which mpirun


should return the string,
``/nethome/rvuduc3/local/jinx/openmpi/1.6.2--gcc4.7.2/bin/mpirun``.

    For future reference, we are using version 1.6.2 of OpenMPI_, not to be confused with OpenMP_. The other major open-source MPI implementation is MPICH_. Many of the major cluster, supercomputer, and networking vendors [#]_ provide their own implementations of MPI as well.

.. _OpenMPI: http://www.open-mpi.org
.. _OpenMP: http://openmp.org
.. _MPICH: http://www.mpich.org
.. [#] Examples of vendors include Intel, IBM, Cray, and Mellanox, among others.

Part 0b: Obtaining this week's code
===========================

Use the same fork-checkout procedure from Lab 1. [#]_ The repo you want is ``gtcse6230fa14/lab4``. As a reminder, the basic steps to get started are:

.. [#] https://bitbucket.org/gtcse6230fa14/lab1/wiki/Home

1. Log into your Bitbucket account.

2. Fork the code for this week's lab into your account. The URL is https://bitbucket.org/gtcse6230fa14/lab4.git . Be sure to rename your repo, appending your Bitbucket ID. Also mark your repo as "Private" if you do not want the world to see your commits.

3. Check out your forked repo on Jinx. Assuming your Bitbucket login is MyBbLogin and assuming that you gave your forked repo the same name (``lab4-MyBbLogin``), you would on Jinx use the command:

.. sourcecode :: bash

    git clone https://MyBbLogin@bitbucket.org/MyBbLogin/lab4-MyBbLogin.git


(Alternatively, if you figured out how to do password-less checkouts using ssh keys, you might use the alternative checkout style, ``git clone git@bitbucket.org:MyBbLogin/lab4--MyBbLogin.git``.)

If it worked, you'll have a ``lab4-MyBbLogin`` subdirectory that you can start editing.

The code we have provided includes these files:

* ``serial.c``: An implementation of a "naive" broadcast algorithm.
* ``tree.c``: A latency-optimal implementation of a minimum spanning tree-based algorithm.
* ``bigvec.c``: A partial implementation of the scatter + all-gather algorithm, which you will complete and compare against the tree-based algorithm.
* A bunch of other files, described below as needed.


Part 1: Creating a communication model for point-to-point operations
====================================================

Your goal is to devise, model, and implement algorithms for *broadcasting* the data from one MPI process to all others, using _only_ the MPI point-to-point communication operations, such as ``MPI_Send``, ``MPI_Recv``, or their non-blocking counterparts, ``MPI_Isend``, ``MPI_Irecv``, and ``MPI_Wait``.

The file ``serial.c`` implements a "naive" broadcast algorithm. The data resides initially on MPI rank 0. This rank sends its data to each of the other processes, one by one. Inspect `serial.c` and verify that the `bcast()` routine implemented therein implements such a scheme.

We ran this benchmark on the Jinx cluster. The results appear in the graph below. More specifically, we show the average time per broadcast (y-axis) as a function of the size of the data size (x-axis).

|Figure1|

.. |Figure1| image:: https://bytebucket.org/gtcse6230fa14/lab4-testing3/raw/f85f7ede438afb0154cff5849600a5e3aba3915a/serial.png?token=07cfeda50de6818b68deae2068472e1fd25e01df

Figure 1: Performance of the naive algorithm on Jinx. Note that the x-axis is on a :math:`\log_2` scale, the y-axis on a :math:`\log_{10}` scale.

**Question 1:** Assuming that the time $T(n)$ to send a message of size $n$ is $T_{\mbox{msg}}(n) = \alpha + \frac{n}{\beta}$, use these data to estimate $\alpha$ and $\beta$. You may either eyeball the plot or use the raw data used to generate these plots, which appear at the following links for [two processors](./serial-2.dat), [four processors](./serial-4.dat), and [eight   processors](./serial-8.dat). Explain how you derived your estimate and report $\alpha$ in microseconds and $\beta$ in megabytes ($10^6$ bytes) per second. (Note that the plot shows message sizes along the x-axis in bytes, [kibibytes](http://en.wikipedia.org/wiki/Kibibyte), and [mebibytes](http://en.wikipedia.org/wiki/Mebibyte).)

> The raw data is stored in a tab-delimited files. Each file has four columns. The first column is the number of MPI processes (ranks). The second column is the size of the data being broadcast, in Bytes. The third column is the average time to perform the broadcast, in seconds. The fourth column is the number of timing trials used to compute the average. (You can basically ignore the last column.)


Part 2: Modeling the "small" and "large" vector broadcast algorithms
===================================================

Recall from the last class that we described two algorithms for
broadcast. The first algorithm is a *minimum spanning tree*
approach. We argued this method is good for *small* messages because
it is optimal with respect to latency (number of messages). The other
uses a two-stage "scatter" plus "all-gather" technique, which we
argued was good for large messages ("big vectors") because it trades a
higher latency component for a nearly-optimal bandwidth component.

Look at the `bcast()` code in `tree.c`. Convince yourself that it
implements a minimum spanning tree algorithm like the one described in
class. Note that it assumes a power-of-two number of processes and,
furthermore, that the number of processes divides the number of data
elements.

**Question 2:** Using your model of message time from Question 1,
  write down a model for the tree-based algorithm.




Running the tree-based algorithm
================================

Let's see how accurately the model from Question 2 predicts actual
execution time by running the tree-based algorithm. We have provided a
`Makefile` to simplify compiling and running this code (and the others
we will use in this lab).

To compile, simply execute:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If it succeeds, it will produce the binary, `tree`.

We have provided `Makefile` rules that make it easy to run this code
and generate a performance plot like that shown in Figure 1 (above).
The command,

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make pbs ALG=tree P=2 ; qstat -a
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

will submit a batch job that runs the `tree` program with two nodes
(`P=2`) and various message sizes. Go ahead and run this command now;
the "`qstat -a`" part peeks at the job queue to verify the job is in
the queue. (Repeat "`qstat -a` to monitor the progress of this job, as
in previous labs.) Once it begins running, it should complete in about
a minute and generate a file called, `tree-2.dat`, which will have the
same format as the `serial-?.dat` files above. Repeat this command
with `P=4` and `P=8` to collect data for the tree-based algorithm
running with four and eight nodes, respectively.

Once you've collected this data, you can generate a plot by executing
the command,

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make plot-alg ALG=tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will run a *gnuplot* script to plot the `tree-2.dat`,
`tree-4.dat`, and `tree-8.dat` data. If successful, it will create a
file called `tree.png`. You can download or use `display` as in the
[previous
lab](http://stumptown.cc.gt.atl.ga.us/cse6230-hpcta-fa12/lab8/) to see
it.

**Question 3:** Compare these data to your model from Question 2. How
  well do they agree?




Implementing the "scatter + all-gather" approach for "big vectors"
==================================================================

Take a look at `bigvec.c`, which is a partial implementation of the
algorithm designed for large vectors. In particular, we've provided
the "scatter" step; you will need to complete the "all-gather" step.

**Question 4:** Complete the "all-gather" code, using only
  point-to-point MPI operations (either blocking or non-blocking, as
  you wish). See the notes below on compiling and testing your
  code. You may make the same assumptions as we do in the tree-based
  algorithm: power-of-two number of processes and the number of
  processes evenly dividing the message length. Once you've gotten it
  working, be sure to upload your final implementation onto T-Square.

To compile your implementation, you can use the command,

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make bigvec
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will generate the executable, `bigvec`. You may choose to test it
interactively on a single node; once you've gotten it debugged,
perform a timing run on eight nodes using the command,

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make pbs ALG=bigvec P=8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When this run succeeds, it will create a data file called,
`bigvec-8.dat`.

**Question 5:** Plot the `serial-8.dat`, `tree-8.dat`, and
  `bigvec-8.dat` together. (See below for a `make` rule we have
  provided that uses *gnuplot* script to do it; alternatively, use any
  plotting software you wish.) You should see that the tree-based
  method is faster for "small" messages, whereas the scatter+allgather
  method if faster for "large" messages. What is the cross-over point
  (message size) between the tree and scatter+allgather methods?

> We have provided a *gnuplot* script that can generate this plot. Use
  `make` to run it by issuing the following command:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.bash}
  make plot-all P=8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



# Welcome

Welcome to your wiki! This is the default page we've installed for your convenience. Go ahead and edit it.

## Wiki features

This wiki uses the [Markdown](http://daringfireball.net/projects/markdown/) syntax.

The wiki itself is actually a git repository, which means you can clone it, edit it locally/offline, add images or any other file type, and push it back to us. It will be live immediately.

Go ahead and try:

```
$ git clone https://gtcse6230fa14@bitbucket.org/gtcse6230fa14/lab4-testing3.git/wiki
```

Wiki pages are normal files, with the .md extension. You can edit them locally, as well as creating new ones.

## Syntax highlighting


You can also highlight snippets of text (we use the excellent [Pygments][] library).

[Pygments]: http://pygments.org/


Here's an example of some Python code:

```
#!python

def wiki_rocks(text):
    formatter = lambda t: "funky"+t
    return formatter(text)
```


You can check out the source of this page to see how that's done, and make sure to bookmark [the vast library of Pygment lexers][lexers], we accept the 'short name' or the 'mimetype' of anything in there.
[lexers]: http://pygments.org/docs/lexers/


Have fun!
