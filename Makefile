.DEFAULT_GOAL := all

EXEEXT =
TARGETS =

TARGETS =listrank-cilk$(EXEEXT)
TARGETS += listrank-cuda$(EXEEXT)

CHDRS = timer.h
CSRCS = $(CHDRS:.h=.c)
COBJS = $(CSRCS:.c=.o)

CXXHDRS = list.hh listrank.hh
CXXSRCS = driver.cc $(CXXHDRS:.hh=.cc)
CXXOBJS = $(CXXSRCS:.cc=.o)

#CUDAHDRS += listrank-gpu.hh
#CUDASRCS += $(CUDAHDRS:.hh=.cu)

CC = icc
CFLAGS = -O3 -g

CXX = icpc
CXXFLAGS = -O3 -g

CUDAROOT = /opt/cuda-4.2/cuda
CUDAC = $(CUDAROOT)/bin/nvcc
CUDAFLAGS = -O3 -arch=sm_20
CUDALDFLAGS = -L$(CUDAROOT)/lib64 -lcudart

LDFLAGS =

all: $(TARGETS)

listrank-cilk$(EXEEXT): $(CXXHDRS) $(CXXOBJS) $(COBJS) Makefile \
	                listrank-par.hh listrank-cilk.cc
	$(CXX) $(CXXFLAGS) -o $@ listrank-cilk.cc $(CXXOBJS) $(COBJS) $(LDFLAGS)

listrank-cuda$(EXEEXT): $(CXXHDRS) $(CXXOBJS) $(COBJS) Makefile \
	                listrank-par.hh listrank-cuda.cu
	$(CUDAC) $(CUDAFLAGS) -o listrank-cuda.o -c listrank-cuda.cu
	$(CXX) $(CXXFLAGS) -o $@ listrank-cuda.o $(CXXOBJS) $(COBJS) \
		$(LDFLAGS) $(CUDALDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cu
	$(CUDAC) $(CUDACFLAGS) -o $@ -c $<

clean:
	rm -f *~ core $(TARGETS)

# eof
