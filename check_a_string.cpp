#include <iostream>
#include <cstdlib>
#include <stdio.h>

#define SEP '\n'

using namespace std;



int main(int argc, char *argv[]){

  FILE *f1,*f2;
  f1= fopen(argv[1],"r");

  int i; int countk=0;
  bool startprinting=false;
  

  do {
    if (countk==atoi(argv[2])){
      startprinting=true;
    }

    i=fgetc(f1);
    if (startprinting) putchar(i);
    if (i==SEP) countk++;
    if (startprinting && i==SEP){ return 0;}
  } while (i!=EOF);


  return 0;
}

