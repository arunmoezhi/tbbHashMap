TBB_INC=${TBBROOT}/include
TBB_LIB=${TBBROOT}/lib/intel64
TBBFLAGS=-I$(TBB_INC) -Wl,-rpath,$(TBB_LIB) -L$(TBB_LIB) -ltbb -ltbbmalloc_proxy -ltbbmalloc
GSL_INC=/opt/apps/intel13/gsl/1.15/include
GSL_LIB=/opt/apps/intel13/gsl/1.15/lib
GSLFLAGS=-I$(GSL_INC) -Wl,-rpath,$(GSL_LIB) -L$(GSL_LIB) -lgsl -lgslcblas
CC=icpc -g
CFLAGS= -O0 -lrt -lpthread -std=gnu++0x -march=native 
SRC= ./src/TestTbbHashMap.cpp
OBJ= ./bin/tbbHashMap.o
all: 
		$(CC) $(CFLAGS) $(TBBFLAGS) $(GSLFLAGS) -o $(OBJ) $(SRC)
clean:
	rm -rf ./bin/*.*
