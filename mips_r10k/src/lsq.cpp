
#include <iostream>
#include <string>
#include "lsq.h"
using namespace std;

LSQ::LSQ(){
	size = DEFAULT_SIZE_LSQ;
	
}	

LSQ::LSQ(int x){
	size = x;

}

LSQ::~LSQ(){
	
	
}


bool LSQ::canAllocate(){
	if (q.size() >= size){
		return false;
	} else {
		return true;
	}
}


bool LSQ::allocate(lsqElt* x){
	if (q.size() >= size){
		return false;
	} else {
		q.push_back(x);
		return true;
	}
}

lsqElt* LSQ::getHead(){
	if (q.empty()){
		return  nullptr;
	} else {
		return q.front();
	}
	
}

bool LSQ::freeHead(){
	if (q.empty()){
		return false;
	} else {
		q.erase(q.begin());
		return true;
	}
}

ostream & operator << (ostream &out, const LSQ & l) {
	vector<lsqElt*> tmp = l.q;
	out << "Max Size:" << l.size << " Size: " << tmp.size()  << endl;
	
	return out;

}

