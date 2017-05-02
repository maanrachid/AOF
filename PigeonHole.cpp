
#include <iostream>

#include <omp.h>
#include <string>
#include "stdlib.h"
#include "Tools.h"
#include "testutils.hpp"
#include <stdio.h>
#include <string.h>
#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <math.h>
#include "PigeonHole.h"

using namespace std;

#define FailVal -10000

ulong N;
uint K;
ulong *startS;



void AAPSP_PigeonHole_Approximate_Match(char *filename, char *output, int threads, int min, int mismatches, bool hamming)
{
	
	// N is the total length of the all strings. we have k strings.   
	//uint *Sorted= new uint[MAX_K];
	uchar *text;
	std::ifstream::pos_type posSize;
	std::ifstream file((char *)filename, std::ios::in | std::ios::binary | std::ios::ate);
	ulong counter = 0, bitnum = 1, reminder = 0, counterk = 0, pos = 0;
	// counterk is to count strings, counter to count the size of text 
	//(without the seperators), pos to get the current pos in final array.
	startS = new ulong[MAX_K];
	int **A = NULL;
	struct stack_node **matched = new stack_node*[MAX_K];    /* used for matched strings in the modified tree */

	startS[0] = 0;

	unordered_map<string, struct dicLink *> * index = new unordered_map<string, struct dicLink *>[mismatches + 1];
	int piecesize = min / (mismatches + 1);
	cerr << "Sigment Size:" << piecesize << endl;

	
	if (file.is_open())
	{
		posSize = file.tellg();
		ulong size = posSize;
		if (MAX_N != 0 && size > MAX_N)
			size = MAX_N;
		char *memblock = new char[size / BUFFER + 1];
		text = new uchar[size / 4];
		reminder = size%BUFFER;

		string str1 = "";
		int m = 0;

		for (int i = 0; i<BUFFER; i++){
			file.seekg(i*(size / BUFFER), std::ios::beg);
			file.read(memblock, size / BUFFER);
			for (ulong z = 0; z<size / BUFFER; z++){
				if (memblock[z] != SEPERATOR){
					//cerr <<"encoding:"<<memblock[z]<<" pos:"<<pos<<endl;
					encode(text, pos, bitnum, memblock[z]);
					if (memblock[z] == 'A' || memblock[z] == 'C' || memblock[z] == 'G' || memblock[z] == 'T') {
						counter++;
						bitnum += 2;
						//cerr<<"bitnum now "<<bitnum<<endl;
						if (bitnum == 9) {
							bitnum = 1;
							pos++;
						}

						if (m<min){
							m++;
							str1 += memblock[z];
							if (m == min) {
								for (int ii = 0; ii<(mismatches + 1); ii++){
									string part = str1.substr(ii*piecesize, piecesize);
									//cout << part <<endl;
									struct dicLink *dl = new dicLink();
									dl->read = counterk;

									if (index[ii].find(part) != index[ii].end()){
										struct dicLink *temp = index[ii][part];
										index[ii][part] = dl;
										dl->link = temp;
									}
									else {
										dl->link = NULL;
										index[ii][part] = dl;
									}

								}

							}
						}
					}
				}
				else {
					startS[++counterk] = counter;
					//Sorted[counterk-1]=counterk-1;
					matched[counterk - 1] = NULL;
					m = 0;
					str1 = "";
				}
			}
		}

		//cerr <<"reminder:"<<reminder<<endl;
		if (reminder>0){
			file.seekg(BUFFER*(size / BUFFER), std::ios::beg);
			file.read(memblock, reminder);
			for (ulong z = 0; z<reminder; z++){
				if (memblock[z] != SEPERATOR){
					encode(text, pos, bitnum, memblock[z]);
					if (memblock[z] == 'A' || memblock[z] == 'C' || memblock[z] == 'G' || memblock[z] == 'T') {
						counter++;
						bitnum += 2;
						if (bitnum == 9) {
							bitnum = 1;
							pos++;
						}
					}
				}
				else {
					startS[++counterk] = counter;
					//Sorted[counterk-1]=counterk-1;
					matched[counterk - 1] = NULL;
				}
			}
		}

		startS[counterk] = counter;
		//Sorted[counterk]=counterk;
		matched[counterk] = NULL;

		N = counter;
		file.close();
		cerr << "Size of File:" << size << endl;
		cerr << "Number of Strings: K " << counterk << endl;
		cerr << "Size of strings:" << counter << endl;
		cerr << "Type of Approximate Matching :";
		if (hamming)
			cerr << " Hamming distance\n" << endl;
		else
			cerr << " Edit distance\n" << endl;
		K = counterk;
		if (output[0] == '1'){
			A = new int*[K];
			
			for (int z = 0; z < K; z++){
				A[z] = new int[K];
				memset(A[z], 0, sizeof(int)*K);
			}
		}
	}

	omp_set_num_threads(threads);


	double starttime, endtime;
	starttime = omp_get_wtime();
	cerr << "Finding Approximate Overlaps: " << endl;

#pragma omp parallel for 
	for (ulong i = 0; i<counterk; i++){
		ulong b = startS[i];
		ulong next = startS[i + 1];
		string s = "";
		for (int j = b; j<next; j++){
			s += decode(text, j);
		}


		//cerr << i<< " "<< (int)(s.length())-min+1<<endl;
		int len = s.length();


		for (int l = 0; l<len - min + 1; l++){ // check all suffixes in string i

			string str1 = s.substr(l, len - l);
			unordered_map<int, int> *printed = new unordered_map<int, int>();

			//cout<<"string :"<<str1<<endl;
			for (int z = 0; z <= mismatches; z++){
				string parts = str1.substr(z*piecesize, piecesize);
				//cout<<parts<<endl;
				unordered_map<string, struct dicLink *>::iterator itr = index[z].find(parts);
				if (itr != index[z].end()){
					//cout << "found match " <<parts<<"from "<<str1<<" in index " <<z <<endl;
					struct dicLink *ptr = itr->second;
					list<int> matchlist;
					while (ptr != NULL){
						int v = mismatches;
						int r = ptr->read;
						bool result = false;

						string before_prefix = "";string after_prefix = "";
						string before_suffix = "";string after_suffix = "";

						if (r == i)
						{
							ptr = ptr->link; continue;
						}
						if (z == 0)
							// if this is a first part, compare until reaching the end of the suffix
							// str1 is a suffix and r is a prefix
							result = compare(piecesize, str1, i, r, str1.length() - piecesize, text, &v,
														hamming,true,after_prefix,after_suffix, true);
						else {
							result = compare(0, str1, i, r, z*piecesize, text, &v,
														hamming,true,before_prefix,before_suffix,false);
							if (v >= 0)
								result = compare((z + 1)*piecesize, str1, i, r, str1.length() - piecesize*(z + 1), text, &v,
														hamming,true, after_prefix, after_suffix,true);
							else
								result = false;
						}

						if (result && i != r){
							if (output[0] == '1'){
								if (A[i][r] < str1.length())
									A[i][r] = str1.length();
							} else if (output[0] == '2' && printed->find(r) == printed->end()){
								printf("%d %d-> %d\n", i, r, str1.length());
								(*printed)[r] = 1;
								if (!hamming){
									cout << before_suffix << parts << after_suffix << endl;
									cout << before_prefix << parts << after_prefix << endl;;
								}
							}
							//ss>> h;
							//matchinglist.push_back(h);
							//return;
						}

						ptr = ptr->link;
					}
				}
			}

			printed->clear();
			delete printed;

		}
	}

	if (output[0] == '1')
		printArr(A,K);

	endtime = omp_get_wtime();
	cerr << "User Time for finding approximate overlaps: "
		<< endtime - starttime << endl;


	delete[] startS;

}


// str is a suffix in r1 and r2 is a read with the prefix. 
bool compare(int start, string str, int r1, int r2, int sizeToCompare, uchar *text, int *mismatches,bool hamming
	, bool backtra, string& v, string& w, bool full_prefix){
	int i;
	
	if (hamming){
		for (i = start; i < start + sizeToCompare; i++){
			if (startS[r2] + i >= startS[r2 + 1]){ // added for cases when a suffix is bigger than a string
				return false;
			}
			if ((str[i] != decode(text, startS[r2] + i)) /*|| (startS[r2]+i>=startS[r2+1])*/){
				(*mismatches)--;
				if ((*mismatches) == -1) return false;
			}
		}
	}
	else {
		string tprefix="";
		string tsuffix = "";
		int limit;
		if (!full_prefix) limit = start + sizeToCompare; else limit = startS[r2 + 1] - startS[r2];
		for (i = start; i < limit; i++){
			
			if (startS[r2] + i >= startS[r2 + 1]) // added for cases when a suffix is bigger than a string
				break;
			tprefix += decode(text, startS[r2] + i);
		}
		tsuffix = str.substr(start, sizeToCompare);
		
		/*
		if (tsuffix.length() < 10 && tsuffix=="TGTTTCACT"){
			cerr << "prefix:" << tprefix << "\n" << "suffix:" << tsuffix << endl;
			cerr << tprefix.length() << " " << tsuffix.length() << endl;
			cerr << "mismatches before:" << *mismatches << endl;
		}*/
		int align = String_Alignment(tprefix, tsuffix, 1, 0, 1, mismatches, v,w,backtra,full_prefix);
		*mismatches +=align;
		//if (tsuffix.length()<10 && tsuffix=="TGTTTCACT") cerr << "Align:"<< align << " mismatches:"<<*mismatches<< endl;
		if ((*mismatches) <= -1) return false;

	}
	return true;
}


int String_Alignment(string v, string w, int indelp, int match, int mismatchp, int *curmismatches, string& str1, string &str2, bool backtra,bool full_prefix)
{

	int **A = new int*[v.length() + 1];
	int **backtrace = new int*[v.length() + 1];

	for (int i = 0; i <= v.length(); i++)
	{
		A[i] = new int[w.length() + 1];
		backtrace[i] = new int[w.length() + 1];
	}

	A[0][0] = 0;
	for (int i = 1; i <= v.length(); i++)
	{
		A[i][0] = A[i-1][0]-indelp;
		backtrace[i][0] = 1;
	}

	for (int i = 1; i <= w.length(); i++)
	{
		A[0][i] = A[0][i - 1] - indelp;
		backtrace[0][i] = 2;

	}


	int max_max=FailVal;
	int choosen_i = v.length();
	for (int i = 1; i <= v.length(); i++)
	{
		for (int j = 1; j <= w.length(); j++)
		{
			int max = 0; int choice = 0;
			if (A[i - 1][j] >= A[i][j - 1])
			{
				max = A[i - 1][j] - indelp;
				choice = 1;
			}
			else
			{
				choice = 2;
				max = A[i][j - 1] - indelp;
			}

			int comp = 0;
			if (v[i - 1] == w[j - 1]) comp = match; else comp = -1 * mismatchp;


			if (max < A[i - 1][j - 1] + comp)
			{
				choice = 3;
				max = A[i - 1][j - 1] + comp;
			}

			A[i][j] = max;

			
			
			backtrace[i][j] = choice;
			
			// we check if the prefix >= suffix 
			// with mismatch =1 and gap = 1 and match =1 it is not possible to get 
			// a match without reaching an equal size, 

			if (j==w.length()){
				if (A[i][j]>max_max){
					choosen_i = i;
					max_max = A[i][j];
				}
				if (max + (*curmismatches)<=-1 && i>=w.length())
					goto label1;
			}

		}

	}


	
label1:


	int u = A[choosen_i][w.length()];

	if (backtra)
	{
		
		backtr(backtrace, v, w, choosen_i, w.length(), str1, str2);
		
	}

	for (int i = 0; i <= v.length(); i++)
	{
		delete(A[i]);
		delete(backtrace[i]);
	}
	delete(A); delete(backtrace);


	return u;


}

void backtr(int** backtrack, string v, string w, int i, int j, string& str1, string& str2)
{

	if (i == 0 && j == 0)
	{
		return;
	}


	if (backtrack[i][j] == 1)
	{
		str1 = v[i - 1] + str1;
		str2 = "-" + str2;
		backtr(backtrack, v, w, i - 1, j, str1, str2);
	}
	else if (backtrack[i][j] == 2)
	{
		str1 = "-" + str1;
		str2 = w[j - 1] + str2;
		backtr(backtrack, v, w, i, j - 1, str1, str2);
	}
	else
	{
		str1 = v[i - 1] + str1;
		str2 = w[j - 1] + str2;
		backtr(backtrack, v, w, i - 1, j - 1, str1, str2);


	}

}