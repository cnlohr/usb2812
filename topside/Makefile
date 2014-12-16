all : newtop

newtop : newtop.o
	g++ -o $@ $^ -lusb-1.0

clean :
	rm -rf *.o *~ newtop
