CPPFLAGS = -ansi -Wall -g

stub: *.h *.cpp
	g++ stub.cpp slVector.cpp -lg -o stub

clean:
	rm -f *.o *.out

test:
	./stub house.jpg out.jpg 312 384
