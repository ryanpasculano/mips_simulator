
#include <iostream>
#include <string>
#include "freelist.h"
using namespace std;

FreeList::FreeList(){
	size = DEFAULT_SIZE_FREELIST;
}	

FreeList::FreeList(int x){
	size = x;
}

FreeList::~FreeList(){
	
}

void FreeList::init(int start){
	while (! fl.empty()){
		fl.pop();
	}
	for (int i = start; i < size; i++){
		fl.push(i);	
	}

}

int FreeList::numFree(){
	return fl.size();
}

int FreeList::getReg(){
	if (fl.empty()){
		// no more free registers need to stall
		return -1;
	} else {

		int tmp = fl.front();
		fl.pop();
		return tmp;
	}
}

bool FreeList::returnReg(int i){
	
	if ((fl.size() < size) and  (i >= 0) and (i < size)){
		fl.push(i);
		return true;
	} else {
		// invalid register
		return false;
	}
}


ostream & operator << (ostream &out, const FreeList& f) {
	queue<int> tmp = f.fl;
	out << "Max Size:" << f.size << " Size: " << tmp.size()  << endl;
	while (! tmp.empty()){
		out << tmp.front() << " ";
		tmp.pop();
	}
	return out;

}


