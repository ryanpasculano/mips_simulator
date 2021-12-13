#define DEFAULT_SIZE_LSQ 50
#include <queue>
using namespace std;
#include <string>

struct lsqElt {
	// information for creating a load or store request
	uint32_t PC;
	uint32_t mem_addr;
};


class LSQ
{
	private:
	
	int size;

	// add queue of lsqElts here
	vector<lsqElt*> q;

	public:
	LSQ();	
	LSQ(int x);
	~LSQ();
	bool canAllocate();
	bool allocate(lsqElt* x);
	lsqElt* getHead();
	bool freeHead();
	friend ostream& operator << (ostream &out, const LSQ & l);

};
