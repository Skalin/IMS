CPP=g++
CPFLAGS= -std=c++11 -Wall
CPPSIMLIB= -lm -lsimlib

all:	 clean ims clean-obj

ims:
	$(CPP) $(CPFLAGS) main.cpp -o ims $(CPPSIMLIB)

run:
	./ims

clean-obj:
	rm -rf *.o

clean:
	rm -rf ims
