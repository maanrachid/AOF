#ifndef _PigeonHole_
#define _PigeonHole_

#define BUFFER 10
#include <string>
using namespace std;

struct dicLink{
	int read;
	struct dicLink *link;
};



void AAPSP_PigeonHole_Approximate_Match(char *filename, char *output, int threads, int min, int mismatches,bool hamming);

bool compare(int start, string str, int r1, int r2, int sizeToCompare, uchar *text, int *mismatches,bool hamming, 
					bool backtra, string& v, string& w,bool full_prefix);



int String_Alignment(string v, string w, int indelp, int match, int mismatchp, int *curmismatches, string& str1, string& str2,bool backtra,bool full_prefix);

void backtr(int** backtrack, string v, string w, int i, int j, string& str1, string& str2);



#endif