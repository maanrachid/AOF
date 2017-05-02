#include <iostream>

#include <omp.h>
#include "stdlib.h"
#include "Tools.h"
#include "testutils.hpp"
#include "Ctree.h"
#include <stdio.h>
#include <string.h>
#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <math.h>
#include "PigeonHole.h"

using namespace std;


int main(){
  string v,w;
  cout << "Enter string v :";
  cin >> v;
  cout << "Enter string w :";
  cin >> w;
  cout << String_Alignment(v,w,1,0,1,1)<<endl;;
  return 0;

}
