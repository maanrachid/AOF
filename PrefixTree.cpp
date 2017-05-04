#include <iostream>
#include "PrefixTree.h"
#include <omp.h>
#define INIT_SHARE 500
#define BUFFER 10

using namespace std;
extern ulong N;
extern uint K;
extern ulong *startS;

void APSP_PrefixTree(char *filename, char *output, int threads, int min)
{
	char sorting[2];
	sorting[0] = '0';

	// N is the total length of the all strings. we have k strings.   
	uint *Sorted = new uint[MAX_K];
	uchar *text;
	std::ifstream::pos_type posSize;
	std::ifstream file((char *)filename, std::ios::in | std::ios::binary | std::ios::ate);
	ulong counter = 0, bitnum = 1, reminder = 0, counterk = 0, pos = 0;
	// counterk is to count strings, counter to count the size of text (without the seperators), pos to get the current pos in final array.
	startS = new ulong[MAX_K];
	struct stack_node **matched = new stack_node*[MAX_K];    /* used for matched strings in the modified tree */
	startS[0] = 0;


	if (file.is_open())
	{
		posSize = file.tellg();
		ulong size = posSize;
		
		char *memblock = new char[size / BUFFER + 1];
		text = new uchar[size / 4];
		reminder = size%BUFFER;

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
					}
				}
				else {
					startS[++counterk] = counter;
					Sorted[counterk - 1] = counterk - 1;
					matched[counterk - 1] = NULL;
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
					Sorted[counterk - 1] = counterk - 1;
					matched[counterk - 1] = NULL;
				}
			}
		}

		startS[counterk] = counter;
		Sorted[counterk] = counterk;
		matched[counterk] = NULL;

		N = counter;
		file.close();
		cerr << "Size of File:" << size << endl;
		cerr << "Number of Strings: K " << counterk << endl;
		cerr << "Size of strings:" << counter << endl;
		K = counterk;

		/* decoding code
		for(int i=0;i<counterk;i++){
		//cerr << "start" <<i<< ":"<<startS[i]<<endl;
		for(ulong z=startS[i];z<startS[i+1];z++){
		char v= decode(text,z);
		//cerr<<"decoding byte "<<bbyte<<" bit "<<bbit<<" "<<v <<endl;
		//cerr<<v;
		}
		//cerr <<SEPERATOR;
		}*/


	}



	/*
	for(uint u=0;u<K;u++)
	cerr<<"Sorted:"<<Sorted[u]<<endl; */
	struct tree_node *ptr;

	sdsl::stop_watch stopwatch;

	if (sorting[0] == '1') {
		stopwatch.start();
		msd(text, 0, K, 0, Sorted);
		stopwatch.stop();
		cerr << "User Time for Sorting: " << stopwatch.getUserTime() << " ms" << endl;
		//print(text);
		//print_Sorted(text,Sorted);
		stopwatch.start();
		cerr << "Creating tree After Sorting: " << endl;
		ptr = create_tree(text, K, N, startS, Sorted);

		stopwatch.stop();
		cerr << "User Time for Constructing tree: " << stopwatch.getUserTime() << " ms" << endl;

		//cerr<<"displaying tree "<<endl;
		//display_tree(ptr);
	}
	else {

		sdsl::stop_watch stopwatch;
		stopwatch.start();
		cerr << "Creating tree: " << endl;

		ptr = create_tree_modified(text, K, N, startS, matched);
		int counter1 = 0;
		traverse_tree_modified(ptr, Sorted, &counter1, matched);
		stopwatch.stop();
		cerr << "User Time for Constructing tree with no sorting required: " << stopwatch.getUserTime() << " ms" << endl;
		//cerr<<"displaying tree: "<<endl;
		//display_tree(ptr);
		//print(text);
		//print_Sorted(text,Sorted);
	}


	//  if (method[0]=='0'){
	find_all_pairs(ptr, text, N, K, startS, Sorted, atoi(output), threads, min);
	// }else {
	//find_all_pairs_modified(ptr,text,N,K,startS,Sorted,atoi(output));
	//}


	// Clean up    
	delete[] startS;

}




struct tree_node *create_tree(uchar *text,uint k,ulong n, ulong startp[],uint sorted[]){
  struct tree_node *root=create_node(0,0,0);
  for(uint i=0;i<k;i++){
    struct tree_node *curptr=root;
    struct tree_node *parent=root;
    long pos=1;long currpos=0;int choosen_child=1;
    long c= startp[sorted[i]];
    //cout<<"k:"<<i<<endl;
    while (1){
      if (curptr->pos>=pos) { //still in the same node
		  if (/*text[c]*/ decode(text,c)== decode(text, startp[sorted[curptr->from]]+currpos)  /*text[startp[sorted[curptr->from]]+currpos]*/) { 
		  //putchar(text[c]); 
		  //cout<<"startp[sorted[curptr->from]]+currpos"<<startp[curptr->from]+currpos<<endl;				
			  pos++;currpos++;c++;
			  if ( c==startp[sorted[i]+1]   /*text[c]==SEPERATOR*/) break;  /* No need to do anything since the range has been updated */
		  } else {  // Split 
			  //cout<<"enter splitting"<<endl;
			  struct tree_node *temp = create_node(pos-1,curptr->from,curptr->to); //new mother node
			  temp->ptr[get_index(decode(text,c)/*text[c]*/)]= create_node(find_pos(startp,sorted,i,k,n)-currpos,i,i);  //new node 
			  parent->ptr[choosen_child]=temp;
			  //cout<<"choosen_child:"<<choosen_child<<endl;
			  //cout<<"startp[sorted[curptr->from]]+currpos:"<<startp[sorted[curptr->from]]+currpos<<endl;
			  temp->ptr[get_index(decode(text,startp[sorted[curptr->from]]+currpos)  /*text[startp[sorted[curptr->from]]+currpos]*/)]=curptr;
			  curptr->pos= curptr->pos-pos;
			  remove_index(curptr,i);
			  break;
		 }
      } else if (curptr->pos<pos){  // moving to another node either old or new node
			if (curptr->ptr[get_index(decode(text,c) /*text[c]*/)]!=NULL){
			  //cout << "Processing string:" << sorted[i]<<endl;
			  choosen_child=get_index(decode(text,c) /*text[c]*/);
			  parent=curptr;
			  curptr= curptr->ptr[choosen_child];
			  pos=1; currpos++;c++;
			  updateNodeRange(curptr,i);
			  //display_node(curptr);
			  if (c==startp[sorted[i]+1]   /*text[c]==SEPERATOR*/) break;  /* No need to do anything since the range has been updated updated */					
			}else {
			  curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos(startp,sorted,i,k,n)-currpos,i,i);
			  //display_node(curptr->ptr[get_index(decode(text,c) /*text[c]*/)]);
			  break;
			}
       }
    }

  }
  return root;
}


/* Stage 1 from method 2: Creating the tree without sorting. no update for the interval is done.*/
struct tree_node *create_tree_modified(uchar *text,uint k,ulong n, ulong startp[],struct stack_node **stacks){
  struct tree_node *root=create_node(0,0,0);
  for(uint i=0;i<k;i++){
    struct tree_node *curptr=root;
    struct tree_node *parent=root;
    ulong pos=1;long currpos=0;int choosen_child=1;
    ulong c= startp[i];
    //cout<<"k:"<<i<<endl;
    while (1){
      if (curptr->pos>=pos) { //still in the same node
			if (decode(text,c) /*text[c]*/==  decode(text,startp[curptr->from]+currpos) && c!=startp[i+1]/* added here recently*/   /*text[startp[curptr->from]+currpos]*/) { 
				//putchar(text[c]); 
				//cout<<"in a node : startp[sorted[curptr->from]]+currpos"<<startp[curptr->from]+currpos<<endl;
				pos++;currpos++;c++;
					
			} else {  // Split 
				//cout<<"enter splitting"<<endl;
				struct tree_node *temp = create_node(pos-1,curptr->from,curptr->to); //new mother node
				if (c==startp[i+1])
					temp->ptr[0] = create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);  //new node for the seperator if end of string is reached
				else
					temp->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);  //new node 
				parent->ptr[choosen_child]=temp;
				//cout<<"choosen_child:"<<choosen_child<<endl;
				//cout<<"startp[sorted[curptr->from]]+currpos:"<<startp[sorted[curptr->from]]+currpos<<endl;
				temp->ptr[get_index(/*text[startp[curptr->from]+currpos]*/ decode(text,startp[curptr->from]+currpos))]=curptr;
				curptr->pos= curptr->pos-pos;
				//cout<<"curpos->"<<curptr->pos<<endl;
				remove_index(curptr,i);
				break;
			}
      }else if (curptr->pos<pos){  // moving to another node either old or new node
				if (curptr->ptr[get_index(decode(text,c)/*text[c]*/)]!=NULL &&  c!=startp[i+1]){
					 choosen_child=get_index(decode(text,c)/*text[c]*/);
					 parent=curptr;
					 curptr= curptr->ptr[choosen_child];
					 //cout << "move toward a child node"<<endl;
					 if (c==startp[i+1]   /*text[c]==SEPERATOR*/){  /* useless now */
						stacks[curptr->from]= create_stack_node(i,stacks[curptr->from]);
						//cout <<"matched 2 " <<curptr->from << " "<<i<<endl;
						break;
					 }
					 pos=1; currpos++;c++;
					
				}else {
				  if (/*text[c]==SEPERATOR*/ c==startp[i+1] && curptr->ptr[1]==NULL && curptr->ptr[2]==NULL && curptr->ptr[3]==NULL && curptr->ptr[4]==NULL){
					stacks[curptr->from]= create_stack_node(i,stacks[curptr->from]);
					//cout <<"create stack node" <<curptr->from << " "<<i<<endl;
					break;
				  }else if (c!=startp[i+1]  && curptr->ptr[1]==NULL && curptr->ptr[2]==NULL && curptr->ptr[3]==NULL && curptr->ptr[4]==NULL && curptr!=root){
					curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);
					curptr->ptr[0]= create_node(0,curptr->from,curptr->to);
					
					//cout<<"create 2 new nodes"<<endl;
					break;
				  }else {
					if (c!=startp[i+1]   /*text[c]==SEPERATOR*/){
						curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);
						//cout<<"create new nodes - not terminator"<<endl;
					}else {
						if (curptr->ptr[0]==NULL){
							curptr->ptr[0]= create_node(0,i,i);
							//cout<<"create new nodes - terminator "<<endl;
						}else {
							stacks[curptr->ptr[0]->from]= create_stack_node(i,stacks[curptr->ptr[0]->from]);
							//cout <<"create stack node 2" <<curptr->from << " "<<i<<endl;
						}
					}
					//display_node(curptr->ptr[get_index(text[c])]);
					
					break;
				  }
			    }
      }
    }

  }
  return root;
}

struct tree_node *create_node(int pos,int from,int to){
  struct tree_node*ptr = new struct tree_node();
  if (pos<0) pos=0;
  ptr->pos=pos;
  ptr->from=from;
  ptr->to=to;
  for(int i=0;i<5;i++)
    ptr->ptr[i]=NULL;
  return ptr;
}

void display_node(struct tree_node* p){
  cout<<"Pos:"<<p->pos<<endl;
  cout<<"From:"<<p->from<<endl;
  cout<<"To:"<<p->to<<endl;
}

void display_tree(struct tree_node* p){

  display_node(p);
  for(int i=0;i<5;i++){
    if (p->ptr[i]!=NULL){
      cout<<"Branch ";
      putchar(get_char(i));
      cout<<endl;
      display_tree(p->ptr[i]);
    }
  }

}


int get_index(uchar c){
  if (c==SEPERATOR)
    return 0;
  else if (c=='A')
    return 1;
  else if (c=='C')
    return 2;
  else if (c=='G')
    return 3;
  else if (c=='T')
    return 4;
  else
    return -1;
}

uchar get_char(int c){
  if (c==0)
    return SEPERATOR;
  else if (c==1)
    return 'A';
  else if (c==2)
    return 'C';
  else if (c==3)
    return 'G';
  else if (c==4)
    return 'T';
  else
    return SEPERATOR;
}

ulong find_pos(ulong startpos[],uint sorted[],int i,ulong k,ulong n){
  if (sorted[i]<k-1)
    return startpos[sorted[i]+1]-startpos[sorted[i]]-1;
  else
    return n-startpos[sorted[i]]-1;
}

ulong find_pos_modified(ulong startpos[],int i,ulong k,ulong n){
  if ((uint)i<k-1)
    return startpos[i+1]-startpos[i]-1;
  else
    return n-startpos[i]-1;
}


void updateNodeRange(struct tree_node *ptr,int i){
  if (ptr->from > i ) ptr->from =i;
  if (ptr->to < i) ptr->to=i;
}

void remove_index(struct tree_node *ptr,int i){
  if (ptr->to==i) ptr->to=i-1;
}



void find_all_pairs(struct tree_node *ptr,uchar *T,ulong N,ulong k,ulong startpos[],uint sorted[],int output
		    ,int threads,int min){
  int **A;
 

  if (output==1){
    A= new int*[k];
    for (uint z=0;z<k;z++)
      A[z]= new int[k];

    for (uint z=0;z<k;z++){
      for (uint z1=0;z1<k;z1++)
	A[z][z1]=0;
    }
  }

 
  // calculate the total load
  double load = 0;
  for(ulong z=0;z<k-1;z++){
    int len=startpos[z+1]-startpos[z]-1;
    //std::cerr<<"len:"<<len<<std::endl;
    load+=(len * (len+1)/2);
    //cerr<<load<<endl;
  }
  load += (N-startpos[k-1]) *(N-startpos[k-1]-1)/2; 
  double share_p = load/threads;
  //  cerr << "Processor Share:"<<share_p<<endl;
  // End calculating



  //Calculating P shares
  double share_temp=0;
  int *start_p = new int[threads];
  int *end_p = new int[threads];
  int p_counter=0;

  start_p[0]=0;
  for(ulong z=0;z<k;z++){
    ulong len=startpos[z+1]-startpos[z]-1;
    if ((share_temp + (len * (len+1)/2)) <=share_p ||
	share_temp==0.0){
      share_temp += (len * (len+1)/2);
      
    }
    else{
      //cerr << p_counter<<" "<<share_temp<<endl;
      end_p[p_counter++]=z-1;
      start_p[p_counter]=z;
      share_temp=(len * (len+1)/2);
      if (p_counter==threads-1) break;
    } 
  }


  end_p[p_counter++]=k-1;
  while (p_counter<=threads-1){  //make the rest idle
    
    start_p[p_counter]=k-1;
    end_p[p_counter]=k-1;
    p_counter++;
  }

  //start_p[p_counter]=k-1;
 
 
  
  

  // End Calculating...
  /*
  for(int z=0;z<threads;z++)
    cerr<<"Processor:"<<z<<" start:"<<start_p[z]<<" end:"<<end_p[z]<<endl;
  */



  //std::cerr<<"load:"<<load<<std::endl;


  double starttime,endtime;
  starttime = omp_get_wtime();

  if (threads==1)
     distribute_seq(threads,T,startpos,sorted,A,ptr,k,N,output,min);
 else
    distribute_k_1(threads,T,startpos,sorted,A,ptr,k,N,output,min);



  endtime = omp_get_wtime();  

  cerr<<"User Time for finding all pairs using brute force: "<<endtime-starttime<<endl;


  if (output==1){
	  printArr(A,k);
  }	
	
}





/* this function represents stage 2 for method 2 to construct the tree. 
   the purpose is to update all intervals for all nodes in the tree */
void traverse_tree_modified(struct tree_node *ptr,uint sorted [],int *counter,struct stack_node **stacks){
  bool first=false;
  for(int i=0;i<5;i++){
    if (ptr->ptr[i]!=NULL){
      traverse_tree_modified(ptr->ptr[i],sorted,counter,stacks);
      if (!first)			/* Since it is postorder traversal, first child should update its parent interval, the last should update the upper bound */
		 ptr->from=ptr->ptr[i]->from;
      else
		 ptr->to=ptr->ptr[i]->to;
      first=true;
    }
  }

  if (!first){   //if the node is a leaf, we check if there are matching strings.
    sorted[*counter]=ptr->from;
    struct stack_node*temp= stacks[ptr->from]; //the stack ins indexed by the old value of ptr->from.
    ptr->from=*counter;
    *counter=*counter+1;
    while (temp!=NULL){
      sorted[*counter]=temp->value;
      *counter=*counter+1;
      temp=temp->next;
    }
		
    ptr->to=(*counter)-1;
  }
}

struct stack_node* create_stack_node(uint i,struct stack_node *ptr){
  struct stack_node *temp=new struct stack_node();
	
  temp->value=i;
  if (ptr!=NULL)
    temp->next=ptr;
  else
    temp->next=NULL;

  return temp;
}



/* this method will simply divide strings between processors */

void distribute_k_1(int threads,uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{
  omp_set_num_threads(threads);
  
#pragma omp parallel for 
  for(ulong i=0;i<k;i++){
    ulong j=startpos[i];
    ulong next=startpos[i+1];

    while (min<=next-j){	//T[j]!=SEPERATOR
      ulong v=j;struct tree_node *curptr=ptr;ulong pos=1;ulong curpos=0;
      while (1){
		if (/*T[v]==SEPERATOR*/ v==next){  // There is a match
		  do_output_all_results(A,sorted,i,next-j,curptr,output,startpos);
		  break;
	    }

		if (pos<=curptr->pos){
			if (/*T[v]==T[startpos[sorted[curptr->from]]+curpos]*/ decode(T,v)==decode(T,startpos[sorted[curptr->from]]+curpos)){
				pos++;curpos++;v++;
			}else
				break;
		}else {
			if (curptr->ptr[get_index(decode(T,v) /*T[v]*/)]!=NULL){
				curptr=curptr->ptr[get_index(decode(T,v) /*T[v]*/)];
				pos=1;curpos++;v++;
			}else 
				break;
		}
      }
      j++;
    }
  }
}




void distribute_seq(int threads,uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{

  

  for(uint i=0;i<k;i++){
    ulong j=startpos[i];
    ulong next=startpos[i+1];

    while (min<=next-j){	//T[j]!=SEPERATOR
      long v=j;struct tree_node *curptr=ptr;long pos=1;long curpos=0;
      while (1){
	if (v==next){  // There is a match
	  do_output_all_results(A,sorted,i,next-j,curptr,output,startpos);
	  break;
	}

	if (pos<=curptr->pos){
	  if (decode(T,v)==decode(T,startpos[sorted[curptr->from]]+curpos)){
	    pos++;curpos++;v++;
	  }else
	    break;
	}else {
	  if (curptr->ptr[get_index(decode(T,v))]!=NULL){
	    curptr=curptr->ptr[get_index(decode(T,v))];
	    pos=1;curpos++;v++;
	  }else 
	    break;
	}
      }
      j++;
    }
  }




}




void do_output_all_results(int **A,uint sorted[],int i,int value,struct tree_node *curptr,int output,ulong startpos[]){
   if (output==1){
     for(int z=curptr->from;z<=curptr->to;z++){
       if (A[i][sorted[z]]==0 && value<=(startpos[sorted[z]+1]-startpos[sorted[z]]))   /* this will work because if the value is not 0
				    , it will be the maximum suffix prefix match. */
			A[i][sorted[z]]= value;
     }
   }
   else if (output==2){
     for(int z=curptr->from;z<=curptr->to;z++){
		if (value>0 && i!=sorted[z] && value<=(startpos[sorted[z]+1]-startpos[sorted[z]]))   
			printf("%d %d ---> %d\n", i,sorted[z], value);
	 }
   }
   
}

void msd(uchar *a, int l, int r, int d, uint *So)
{
	int count[5];
	int *temp = new int[K];


	//cerr << "l , r :"<<l<< " "<< r <<endl;
	if (r <= l + 1) {
		delete temp;
		return;
	}

	for (int i = 0; i<5; i++)
		count[i] = 0;

	for (int i = l; i < r; i++){
		if (startS[So[i]] + d >= N)
			count[0]++;
		else if (startS[So[i]] + d < startS[So[i] + 1]/*-1*/) {
			//cerr << "decode(a,startS[So[i]]+d):"<<decode(a,startS[So[i]]+d) << endl;
			count[choose_index(decode(a, startS[So[i]] + d)/*a[startS[So[i]]+d]*/)]++;
		}
		else
			count[0]++;
	}

	for (int k = 1; k <5; k++)
		count[k] += count[k - 1];

	/*
	for (int k = 0; k <5 ; k++)
	cerr<<"count k Before:"<<count[k]<<endl;
	*/

	for (int i = l; i < r; i++){
		//cerr<<"choose:"<<choose_index(a[startS[So[i]]+d])<<endl;
		if (startS[So[i]] + d >= N) {
			temp[count[0] - 1 + l] = So[i];
			count[0]--;
		}
		else if (startS[So[i]] + d < startS[So[i] + 1]/*-1*/){
			temp[count[choose_index(/*a[startS[So[i]]+d]*/ decode(a, startS[So[i]] + d))] - 1 + l] = So[i];
			count[choose_index(/*a[startS[So[i]]+d]*/  decode(a, startS[So[i]] + d))]--;
			//cerr <<"i , So[i]:" << i <<" "<<So[i]<<endl;
		}
		else {
			temp[count[0] - 1 + l] = So[i];
			count[0]--;
		}
	}


	for (int i = l; i < r; i++)
		So[i] = temp[i];

	/*
	for (int i = l; i < r; i++)
	cerr<<"So After:"<<So[i]<<endl;

	for (int k = 0; k <5 ; k++)
	cerr<<"count k After:"<<count[k]<<endl;
	*/
	delete temp;




	//	cerr <<"-------------"<<endl;
	for (int i = 1; i < 4; i++){
		msd(a, l + count[i], l + count[i + 1], d + 1, So);
	}


	msd(a, l + count[4], r, d + 1, So);

}


