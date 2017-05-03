CC = g++
CPPFLAGS =  -fopenmp -ansi -pedantic -O3 -std=c++0x

objects = AOF.o Tools.o testutils.o PrefixTree.o PigeonHole.o

default: $(objects)
	$(CC) $(CPPFLAGS) -o AOF $(objects)
	$(CC) gen_test.cpp -o gen
	$(CC) converter_fasta_to_plain.cpp -o converter
	$(CC) reverse_complement.c -o reverse_complement

clean:
	rm -f core *.exe *.o *~ Apsp test.* 



