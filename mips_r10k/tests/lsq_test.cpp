//Ryan
#include <iostream>
#include "../src/lsq.h"
#include <string>
using namespace std;

int main(){

	LSQ* t = new LSQ(10);
	cout << *t << endl;
	lsqElt x = {5};
	
	t -> allocate(&x);
	cout << *t << endl;

	lsqElt* y = t -> getHead();
	t -> freeHead();
	
	cout << *t << endl;
	delete t;
	return 0;
	// allocate
	// gethead
	// free head

}
