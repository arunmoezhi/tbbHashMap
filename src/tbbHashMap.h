#include"header.h"
using namespace tbb;
using namespace std;

typedef concurrent_hash_map<unsigned long,bool> tbbMap;
tbbMap map;
tbbMap::accessor a;

bool search(struct tArgs* t, unsigned long key)
{
	if(map.find(a,key))
	{
		t->successfulReads++;
		return true;
	}
	else
	{
		t->unsuccessfulReads++;
		return false;
	}
}

bool insert(struct tArgs* t, unsigned long key)
{
	if(map.insert(a,key))
	{
		t->successfulInserts++;
		return true;
	}
	else
	{
		t->unsuccessfulInserts++;
		return(false);
	}
}

bool remove(struct tArgs* t, unsigned long key)
{
	if(map.erase(key))
	{
		t->successfulDeletes++;
		return true;
	}
	else
	{
		t->unsuccessfulDeletes++;
		return(false);
	}
}