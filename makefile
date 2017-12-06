all:	 clean ims clean-obj

ims: main.cpp
	g++ -std=c++11 -Wall -m64 -O2 -g -lm -lsimlib -o ims

run:
	./ims

clean-obj:
	rm -rf *.o

clean:
	rm -rf ims
