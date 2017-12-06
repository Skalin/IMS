all:	 clean ims clean-obj

ims: main.cpp
	g++ -std=c++11 -Wall -m64 -O2  -g   -I /home/skalin/Programming/simlib/src    main.cpp /home/skalin/Programming/simlib/src/simlib.so /home/skalin/Programming/simlib/src/simlib.h   -o ims

clean-obj:
	rm -rf *.o

clean:
	rm -rf ims
