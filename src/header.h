#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include<time.h>
#include<stdint.h>
#include<tbb/atomic.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include<limits.h>
#include<assert.h>
#define K 2
#define LEFT 0
#define RIGHT 1
#include "tbb/concurrent_hash_map.h"
//#define STATS_ON


struct tArgs
{
  int tId;
	timespec searchTime;
	timespec insertTime;
	timespec deleteTime;
  unsigned long lseed;
  unsigned long readCount;
  unsigned long successfulReads;
  unsigned long unsuccessfulReads;
  unsigned long readRetries;
  unsigned long insertCount;
  unsigned long successfulInserts;
  unsigned long unsuccessfulInserts;
  unsigned long insertRetries;
  unsigned long deleteCount;
  unsigned long successfulDeletes;
  unsigned long unsuccessfulDeletes;
  unsigned long deleteRetries;
  unsigned long simpleDeleteCount;
  unsigned long complexDeleteCount;
  bool isNewNodeAvailable;
	unsigned long seekInsertRetries;
	unsigned long seekDeleteRetries;
	unsigned long seekLength;
	unsigned long pad[3];
};

bool search(struct tArgs*, unsigned long);
bool insert(struct tArgs*, unsigned long);
bool remove(struct tArgs*, unsigned long);
unsigned long size();
void printKeys();
bool isValidTree();
