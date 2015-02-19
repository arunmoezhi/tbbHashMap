#include"tbbHashMap.h"

int NUM_OF_THREADS;
int findPercent;
int insertPercent;
int deletePercent;
unsigned long keyRange;
double MOPS;
volatile bool start=false;
volatile bool stop=false;
volatile bool steadyState=false;
struct timespec runTime,transientTime;

struct timespec diff(timespec start, timespec end) //get difference in time in nano seconds
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) 
	{
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else 
	{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

static inline unsigned long getRandom(gsl_rng* r)
{
	return(gsl_rng_uniform_int(r,keyRange) + 1);
}

void *operateOnTree(void* tArgs)
{
	struct timespec s,e;
  int chooseOperation;
  unsigned long lseed;
	unsigned long key;
  int threadId;
  struct tArgs* tData = (struct tArgs*) tArgs;
  threadId = tData->tId;
  const gsl_rng_type* T;
  gsl_rng* r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
	lseed = tData->lseed;
  gsl_rng_set(r,lseed);

  tData->isNewNodeAvailable=false;
	tData->readCount=0;
  tData->successfulReads=0;
  tData->unsuccessfulReads=0;
  tData->readRetries=0;
  tData->insertCount=0;
  tData->successfulInserts=0;
  tData->unsuccessfulInserts=0;
  tData->insertRetries=0;
  tData->deleteCount=0;
  tData->successfulDeletes=0;
  tData->unsuccessfulDeletes=0;
  tData->deleteRetries=0;
	tData->seekLength=0;
  while(!start)
  {
  }
	
	while(!steadyState)
	{
	  chooseOperation = gsl_rng_uniform(r)*100;
		key = gsl_rng_uniform_int(r,keyRange) + 1;
    if(chooseOperation < findPercent)
    {
      search(tData, key);
    }
    else if (chooseOperation < insertPercent)
    {
      insert(tData, key);
    }
    else
    {
      remove(tData, key);
    }
	}
  tData->readCount=0;
  tData->successfulReads=0;
  tData->unsuccessfulReads=0;
  tData->readRetries=0;
  tData->insertCount=0;
  tData->successfulInserts=0;
  tData->unsuccessfulInserts=0;
  tData->insertRetries=0;
  tData->deleteCount=0;
  tData->successfulDeletes=0;
  tData->unsuccessfulDeletes=0;
  tData->deleteRetries=0;
	tData->seekLength=0;
	
	while(!stop)
  {
    chooseOperation = gsl_rng_uniform(r)*100;
		key = gsl_rng_uniform_int(r,keyRange) + 1;

    if(chooseOperation < findPercent)
    {
			tData->readCount++;
      clock_gettime(CLOCK_REALTIME,&s);
      search(tData, key);
			clock_gettime(CLOCK_REALTIME,&e);
			tData->searchTime.tv_sec += diff(s,e).tv_sec;tData->searchTime.tv_nsec += diff(s,e).tv_nsec;
    }
    else if (chooseOperation < insertPercent)
    {
			tData->insertCount++;
      clock_gettime(CLOCK_REALTIME,&s);
      insert(tData, key);
			clock_gettime(CLOCK_REALTIME,&e);
			tData->insertTime.tv_sec += diff(s,e).tv_sec;tData->insertTime.tv_nsec += diff(s,e).tv_nsec;
    }
    else
    {
			tData->deleteCount++;
      clock_gettime(CLOCK_REALTIME,&s);
      remove(tData, key);
			clock_gettime(CLOCK_REALTIME,&e);
			tData->deleteTime.tv_sec += diff(s,e).tv_sec;tData->deleteTime.tv_nsec += diff(s,e).tv_nsec;
    }
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  struct tArgs** tArgs;
  double totalTime=0;
  double avgTime=0;
	unsigned long lseed;
	//get run configuration from command line
  NUM_OF_THREADS = atoi(argv[1]);
  findPercent = atoi(argv[2]);
  insertPercent= findPercent + atoi(argv[3]);
  deletePercent = insertPercent + atoi(argv[4]);
  runTime.tv_sec = atoi(argv[5]);
	runTime.tv_nsec =0;
	transientTime.tv_sec=0;
	transientTime.tv_nsec=2000000;
  keyRange = (unsigned long) atol(argv[6]);
	lseed = (unsigned long) atol(argv[7]);

  tArgs = (struct tArgs**) malloc(NUM_OF_THREADS * sizeof(struct tArgs*)); 

  const gsl_rng_type* T;
  gsl_rng* r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r,lseed);
	
	//Initialize the tree. Must be called before doing any operations on the tree
  
  struct tArgs* initialInsertArgs = (struct tArgs*) malloc(sizeof(struct tArgs));
  initialInsertArgs->successfulInserts=0;
  initialInsertArgs->isNewNodeAvailable=false;
	
  while(initialInsertArgs->successfulInserts < keyRange/2) //populate the tree with 50% of keys
  {
    insert(initialInsertArgs,gsl_rng_uniform_int(r,keyRange) + 1);
  }
  pthread_t threadArray[NUM_OF_THREADS];
  for(int i=0;i<NUM_OF_THREADS;i++)
  {
    tArgs[i] = (struct tArgs*) malloc(sizeof(struct tArgs));
    tArgs[i]->tId = i;
    tArgs[i]->lseed = gsl_rng_get(r);
  }

	for(int i=0;i<NUM_OF_THREADS;i++)
	{
		pthread_create(&threadArray[i], NULL, operateOnTree, (void*) tArgs[i] );
	}
	
	start=true; //start operations
	nanosleep(&transientTime,NULL); //warmup
	steadyState=true;
	nanosleep(&runTime,NULL);
	stop=true;	//stop operations
	
	for(int i=0;i<NUM_OF_THREADS;i++)
	{
		pthread_join(threadArray[i], NULL);
	}	

  unsigned long totalReadCount=0;
  unsigned long totalSuccessfulReads=0;
  unsigned long totalUnsuccessfulReads=0;
  unsigned long totalReadRetries=0;
  unsigned long totalInsertCount=0;
  unsigned long totalSuccessfulInserts=0;
  unsigned long totalUnsuccessfulInserts=0;
  unsigned long totalInsertRetries=0;
  unsigned long totalDeleteCount=0;
  unsigned long totalSuccessfulDeletes=0;
  unsigned long totalUnsuccessfulDeletes=0;
  unsigned long totalDeleteRetries=0;
	unsigned long totalSeekLength=0;
	timespec totalSearchTime;
	timespec totalInsertTime;
	timespec totalDeleteTime;
	totalSearchTime.tv_sec=0;totalSearchTime.tv_nsec=0;
	totalInsertTime.tv_sec=0;totalInsertTime.tv_nsec=0;
	totalDeleteTime.tv_sec=0;totalDeleteTime.tv_nsec=0;
 
  for(int i=0;i<NUM_OF_THREADS;i++)
  {
    totalReadCount += tArgs[i]->readCount;
    totalSuccessfulReads += tArgs[i]->successfulReads;
    totalUnsuccessfulReads += tArgs[i]->unsuccessfulReads;
    totalReadRetries += tArgs[i]->readRetries;

    totalInsertCount += tArgs[i]->insertCount;
    totalSuccessfulInserts += tArgs[i]->successfulInserts;
    totalUnsuccessfulInserts += tArgs[i]->unsuccessfulInserts;
    totalInsertRetries += tArgs[i]->insertRetries;
    totalDeleteCount += tArgs[i]->deleteCount;
    totalSuccessfulDeletes += tArgs[i]->successfulDeletes;
    totalUnsuccessfulDeletes += tArgs[i]->unsuccessfulDeletes;
    totalDeleteRetries += tArgs[i]->deleteRetries;
		totalSeekLength += tArgs[i]->seekLength;
		totalSearchTime.tv_sec +=tArgs[i]->searchTime.tv_sec;totalSearchTime.tv_nsec +=tArgs[i]->searchTime.tv_nsec;
		totalInsertTime.tv_sec +=tArgs[i]->insertTime.tv_sec;totalInsertTime.tv_nsec +=tArgs[i]->insertTime.tv_nsec;
		totalDeleteTime.tv_sec +=tArgs[i]->deleteTime.tv_sec;totalDeleteTime.tv_nsec +=tArgs[i]->deleteTime.tv_nsec;
  }

	unsigned long totalOperations = totalReadCount + totalInsertCount + totalDeleteCount;
	MOPS = totalOperations/(runTime.tv_sec*1000000.0);
	//printf("k%d;%d-%d-%d;%d;%ld;",atoi(argv[6]),findPercent,(insertPercent-findPercent),(deletePercent-insertPercent),NUM_OF_THREADS,size());
	printf("k%d;%d-%d-%d;%d;",atoi(argv[6]),findPercent,(insertPercent-findPercent),(deletePercent-insertPercent),NUM_OF_THREADS);
	printf("(%.2f %.2f %.2f);",(totalReadRetries * 100.0/totalOperations),(totalInsertRetries * 100.0/totalInsertCount),(totalDeleteRetries * 100.0/totalDeleteCount));
	printf("(%.2f %.2f %.2f);",(double) totalSearchTime.tv_nsec/(totalReadCount*1000),(double) totalInsertTime.tv_nsec/(totalInsertCount*1000),(double) totalDeleteTime.tv_nsec/(totalDeleteCount*1000));
	printf("%.2f;%.2f\n",(totalSeekLength*1.0/totalOperations),MOPS);
	pthread_exit(NULL);
}
