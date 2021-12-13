#ifndef __FREELIST__
#define __FREELIST__
#define DEFAULT_SIZE_FREELIST 64
using namespace std;
#include <string>
#include <queue>


class FreeList{
	public:
	int size;
	queue<int> fl;


	FreeList();	
	FreeList(int x);
	~FreeList();
	void init(int start);
	int numFree();
	int getReg();
	bool returnReg(int i);
	friend ostream& operator << (ostream &out, const FreeList & f);

};

#endif
