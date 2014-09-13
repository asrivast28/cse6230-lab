.DEFAULT_GOAL := all

EXEEXT =
TARGETS = listrank$(EXEEXT)

CHDRS = timer.h
CSRCS = $(CHDRS:.h=.c)
COBJS = $(CSRCS:.c=.o)

CXXHDRS = list.hh listrank.hh listrank-par.hh
CXXSRCS = driver.cc $(CXXHDRS:.hh=.cc)
CXXOBJS = $(CXXSRCS:.cc=.o)

#CUDAHDRS += listrank-gpu.hh
#CUDASRCS += $(CUDAHDRS:.hh=.cu)

CC = icc
CFLAGS = -O3 -g

CXX = icpc
CXXFLAGS = -O3 -g

CUDAC = nvcc
CUDAFLAGS =

LDFLAGS =

all: $(TARGETS)

listrank$(EXEEXT): $(CXXHDRS) $(CXXOBJS) $(COBJS) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJS) $(COBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cu
	$(CUDAC) $(CUDACFLAGS) -o $@ -c $<

clean:
	rm -f *~ core $(TARGETS)

# eof
