#include <iostream>
#include <cstdlib>
#include <stdio.h>

#include "Tools.h"

#define SEP '\n'
#define BUFFER 10

using namespace std;


int main(int argc, char *argv[]){

	uchar *text;
	std::ifstream::pos_type posSize;
	std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
	std::ofstream fileout(argv[2] , std::ofstream::out);
	

	if (argc < 3){
		cerr << "To run : reverse_complement inputfile outputfile" << endl;
		return 0;
	}

	if (file.is_open())
	{
		posSize = file.tellg();
		ulong size = posSize;
		
		char *memblock = new char[size / BUFFER + 1];
		char *memblockout = new char[2 * (size / BUFFER + 1)];
		char *read = new char[size / 4];
		char *reverse = new char[size / 4];

		int read_counter = 0;
		long outcounter = 0;


		for (int i = 0; i < BUFFER; i++){
			file.seekg(i*(size / BUFFER), std::ios::beg);
			file.read(memblock, size / BUFFER);
			
			int z = 0;

			outcounter = 0;
			for (int z = 0; z < size / BUFFER;z++){
				read[read_counter] = memblock[z];
				if (memblock[z] == SEPERATOR){
					read[read_counter] = '\0';
					/*
					memcpy(&memblockout[outcounter], read, read_counter);
					outcounter += read_counter;
					memblockout[outcounter++] = '\n';*/
					fileout << read << endl;
					

					for (int g = 0; g < (read_counter)/2; g++){
						char temp = reverse[g];
						reverse[g] = reverse[read_counter - 1-  g];
						reverse[read_counter - 1 - g] = temp;
					}
					reverse[read_counter] = '\0';
					
					/*
					memcpy(&memblockout[outcounter], reverse, read_counter);
					outcounter += read_counter;
					memblockout[outcounter++] = '\n';
					*/
					fileout << reverse << endl;
					read_counter = 0;
				}
				else if (memblock[z] == 'A'){
					reverse[read_counter++] = 'T';
				}
				else if (memblock[z] == 'C'){
					reverse[read_counter++] = 'G';
				}
				else if (memblock[z] == 'G'){
					reverse[read_counter++] = 'C';
				}
				else if (memblock[z] == 'T'){
					reverse[read_counter++] = 'A';

				}
			}
			
			
			
		}


		if (size%BUFFER > 0){
			file.seekg(BUFFER*(size / BUFFER), std::ios::beg);
			file.read(memblock, size % BUFFER);

			for (int z = 0; z < size % BUFFER; z++){
				read[read_counter] = memblock[z];
				if (memblock[z] == SEPERATOR){
					read[read_counter] = '\0';

					/*
					memcpy(&memblockout[outcounter], read, read_counter);
					outcounter += read_counter;
					memblockout[outcounter++] = '\n';
					*/
					fileout << read << endl;
					for (int g = 0; g < read_counter / 2; g++){
						char temp = reverse[g];
						reverse[g] = reverse[read_counter - 1 - g];
						reverse[read_counter - 1 - g] = temp;
					}
					reverse[read_counter] = '\0';
					
					/*
					memcpy(&memblockout[outcounter], reverse, read_counter);
					outcounter += read_counter;
					memblockout[outcounter++] = '\n';
					*/

					fileout << reverse << endl;
					
					read_counter = 0;
				}
				else if (memblock[z] == 'A'){
					reverse[read_counter++] = 'T';
				}
				else if (memblock[z] == 'C'){
					reverse[read_counter++] = 'G';
				}
				else if (memblock[z] == 'G'){
					reverse[read_counter++] = 'C';
				}
				else if (memblock[z] == 'T'){
					reverse[read_counter++] = 'A';

				}
			}
			
		}

	}
	else {
		cerr << argv[1] << " could not be opened.\n";
		return 0;
	}

	file.close();
	//fileout.close();
	return 0;
}
