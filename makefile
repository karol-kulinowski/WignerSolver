# Makefile
# Author: KK

CXXFLAGS = -std=c++11 -g -Ofast\
	-fopenmp -Wpedantic -Wall -Wextra -Werror\
	-Wdisabled-optimization\
	-Wlogical-op\
	-Wmissing-declarations\
	-Wmissing-include-dirs\
	-Wredundant-decls\
	-Wshadow\
	-Wswitch-default\
	-Wsign-conversion\
	-Wfloat-conversion
CXX = g++ # icpc
OBJS = src/WignerFunction.o \
	src/wfIO.o src/wignerTools.o \
	src/solveWignerPoisson.o \
	src/poisson1D.o \
	main.o
LDLIBS = -larmadillo -lsuperlu -lopenblas -llapack
#-lblas <-> -lopenblas
#-lsuperlu -larmadillo -lopenblas -lm -fopenmp #-m64 -I${MKLROOT}/include # -mkl
#-lsuperlu -L/home/karol/intel/compilers_and_libraries_2019.0.117/linux/mkl/lib -llapack -L/opt/OpenBLAS/lib/ -lm  -lopenblas -lm
# -lm -- math library

run.out: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f $(OBJS) run.out
