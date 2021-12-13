//Ryan
#include <iostream>
#include "../src/rs.h"
#include "../src/util.h"
#include <string>
using namespace std;

int main(){

	//Testing Initialization
	RS* r = new RS();
	cout << "Initializing RS" << endl;
	cout << *r << endl;
	//Testing getReg()
	for (int i = 0; i < 6; i++){
		cout << "canAllocate(" << i << "): " << r->canAllocate(i) << endl;
	}	

	cout << "assignslot(0, 1, 2, true, 3, true): " << endl;
	r->assignSlot(0, 1, 2, true, 3, true);
	cout << "assignslot(0, 4, 5, false, 6, false): " << endl;
	r->assignSlot(2, 3, 4, false, 5, false);
	cout << "assignslot(0, 7, 8, true, 9, false): " << endl;
	r->assignSlot(3, 7, 8, true, 9, false);
	cout << *r << endl;
	//Testing getReg()
	for (int i = 0; i < 6; i++){
		cout << "canAllocate(" << i << "): " << r->canAllocate(i) << endl;
	}	


	cout << "assignslot(0, 3, 4, true, 5, false): " << endl;
	r->assignSlot(0, 3, 4, true, 5, false);
	
	cout << *r << endl;
	//Testing getReg()
	for (int i = 0; i < 6; i++){
		cout << "canAllocate(" << i << "): " << r->canAllocate(i) << endl;
	}	


	

/*int i = f -> getReg();
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
*/
	return 0;
}
