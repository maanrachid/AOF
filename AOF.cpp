
#include <iostream>

#include <omp.h>
#include "stdlib.h"
#include "Tools.h"
#include "testutils.hpp"
#include "PrefixTree.h"
#include <stdio.h>
#include <string.h>
#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <math.h>
#include "PigeonHole.h"






using namespace std;

void displayhelp();
void display_choices(char *filename, char *output, int threads,int min,int mismatches,bool hamming);




int main(int argc,char *argv[])
{

  char *filename;

  char *output=new char[2];
  char *sorting = new char[2];
  int mismatches,processors,minlength;
  bool hamming;

  if (argc<2){
	  cerr << "wrong number of arguments, run Apsp to get a help" << endl;
	  displayhelp();
	  return 0;
  }

  // Default values 
 
  processors = omp_get_num_procs(); // use all threads
  minlength = 1; // minimal accepted length
  output[0] = '2';  // output results
  
  filename = argv[1];
  mismatches = 0;  // exact match
  hamming = true;  // useful only with approximate match
  

  for(int i=2;i<argc;i++){
    if (argv[i][0]=='-'){
      if (argv[i][1]=='p')
	processors = atoi(argv[i+1]);
      else if (argv[i][1]=='m')
	minlength=atoi(argv[i+1]);
      else if (argv[i][1]=='h')
	mismatches=atoi(argv[i+1]);
	  else if (argv[i][1] == 'o')
		  output = argv[i + 1];
	  else if (argv[i][1] == 't')
		  hamming = (bool)atoi(argv[i + 1]);
	
	  
    }
  }

  display_choices(filename,output,processors,minlength,mismatches,hamming);

  
  if (mismatches==0)
	  APSP_PrefixTree(filename, output, processors, minlength);
  else
	  AAPSP_PigeonHole_Approximate_Match(filename, output, processors, minlength, mismatches, hamming);

  return 0;
}




void displayhelp(){
  cerr<<"*******************************************************"<<endl;
  cerr<<"This program has one parameter and several optional parameters:"<<endl;
  cerr<<"AOF filename [mismatches] [number_of_processors] [output] [minimum_length] [type]"<<endl;
  cerr<<"filename\t is the name of the data file."<<endl;
  cerr<<"-h\tnumber of mismatches. Default value is 3."<<endl;
  cerr<<"-p\tnumber of threads. Default value is the maximum."<<endl;
  cerr << "-o\toutput:\t 2 (default) for producing all overlaps, 1 for only maximum overlaps, 0 for no output." << endl;
//in an array.\n\t 0 for executing without an output.\n\t 2 for outputing all overlaps."<<endl;
  cerr<<"-m\tminimum overlap length for matching, 1 is the default value."<<endl;
  cerr << "-t\tType of matching: 1 (hamming) is the default, 0 (Edit distance)." << endl;
  cerr<<"*******************************************************"<<endl;
}


void display_choices(char *filename, char *output, int threads,int min,int mismatches,bool hamming){

  cerr<<"These are your choices:"<<endl;
  cerr<<"File name:"<<filename<<endl;
  cerr<<"Output:";
  if (output[0]=='0')
    cerr<<" No Output" <<endl;
  else if (output[0]=='1')
    cerr<<" Maximum suffix-prefix matches are shown in two dimentional array."<<endl;
  else if (output[0] == '2')
    cerr<<" All overlaps are shown. Alignments can be shown if edit distance is used (-t 1). "<<endl;


  cerr<<"Number of threads:"<<threads<<endl;
  cerr<<"Minimum Match Length:"<<min<<endl;
  if (mismatches == 0)
	  cerr << "Exact Matching is requested. Prefix tree is used.\n";
  else{
	  cerr << "Number of Mistmatches:" << mismatches << endl;;
	  if (hamming)
		  cerr << "Hamming distance is used for approximate matching." << endl;
	  else
		  cerr << "Edit distance is used for approximate matching." << endl;

  }
}




