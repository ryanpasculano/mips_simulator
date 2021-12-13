//Ryan
#include <iostream>
#include "../src/freelist.h"
#include <string>
using namespace std;

int main(){

	//Testing Initialization
	FreeList* f = new FreeList(20);
	f -> init(10);
	cout << "Initializing Freelist of size 20 with 10 free elements" << endl;
	cout << "Freelist upon initialization:" << endl;
	cout << *f << endl;

	//Testing getReg()
	int i = f -> getReg();
	cout << "recieved reg from FreeList: " << i << endl;
	
	cout << "FreList after one item removed" << endl;
	cout << *f << endl;


	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	f -> getReg();
	cout << "Getting when there is no free register: " << f -> getReg() << endl;
	
	// Testing returnReg(int)
	for (int j = -1; j < 23; j+=2){
		cout << "Adding  " << j << " to FreeList" << endl;
		if (f -> returnReg(j)){
			cout << j << " successfully added to FreeList" << endl;
		} else {
			cout << j <<  " could not be added to FreeList" << endl;
		}
	}
	
	cout << "Ending FreeList" << endl;
	cout << *f << endl;		
	delete f;

	return 0;
}
