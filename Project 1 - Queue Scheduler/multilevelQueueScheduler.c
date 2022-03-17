#include <stdlib.h>

#include "multilevelQueueScheduler.h"

int min( int x, int y );

static const int STEPS_TO_PROMOTION = 50;
static const int FOREGROUND_QUEUE_STEPS = 5;

/* createSchedule
 * input: none
 * output: a schedule
 *
 * Creates and return a schedule struct.
 */
schedule* createSchedule( ) {
    /* TODO: initialize data in schedule */
	//processData* pData = (processData*)malloc(sizeof(processData));
	schedule* sched = (schedule*)malloc(sizeof(schedule));
	if( sched==NULL ){
        fprintf(stderr, "Error: Unable to allocate data");
        exit(-1);
    }
	else{
		sched->foreQueue = createQueue();
		sched->backQueue = createQueue();
	}

    return sched; /* TODO: Replace with your return value */
}

/* isScheduleUnfinished
 * input: a schedule
 * output: bool (true or false)
 *
 * Check if there are any processes still in the queues.
 * Return TRUE if there is.  Otherwise false.
 */
bool isScheduleUnfinished( schedule *ps ) {
    /* TODO: check if there are any process still in a queue.  Return TRUE if there is. */
	if (isEmpty(ps->foreQueue) && isEmpty(ps->backQueue))
    	return false; /* TODO: Replace with your return value */
	else
		return true;
}

/* addNewProcessToSchedule
 * input: a schedule, a string, a priority
 * output: void
 *
 * Create a new process with the provided name and priority.
 * Add that process to the appropriate queue
 */
void addNewProcessToSchedule( schedule *ps, char *processName, priority p ) {
    /* TODO: complete this function.
    The function "initializeProcessData" in processSimulator.c will be useful in completing this. */
	processData* newProcessData = initializeProcessData(processName);
	process* newProcess = (process*)malloc(sizeof(process));

	newProcess->processName = processName;
	newProcess->priority = p;
	newProcess->timeSteps = 0;
	newProcess->data = newProcessData;
	newProcess->timeScheduled = getCurrentTimeStep();

	if(newProcess->priority == FOREGROUND){
		//add process to foreQueue
		enqueue(ps->foreQueue, newProcess);
	}
	else if(newProcess->priority == BACKGROUND){
		//add process to backQueue
		enqueue(ps->backQueue, newProcess);
	}
	
    //free( processName ); /* TODO: This is to prevent a memory leak but you should remove it once you create a process to put processName into */
}

void printSchedule(schedule *ps){
    
    LLNode* currentNode;// = (LLNode*)malloc(sizeof(LLNode));
    
    if (isScheduleUnfinished(ps)){
        if ( (!isEmpty(ps->foreQueue))){
            
            currentNode = ps->foreQueue->qFront;
            printf("---Stuff in the FOREGROUND ---\n");
            printf("%s\n", currentNode->qt->processName);
            
            while(!(currentNode->pNext == NULL)){
                currentNode = currentNode->pNext;
                printf("%s\n", currentNode->qt->processName);
            }
        }
        
        if ( (!isEmpty(ps->backQueue))){
            // LLNode* currentNode = (LLNode*)malloc(sizeof(LLNode));
            currentNode = ps->backQueue->qFront;
            printf("---Stuff in the BACKGROUND ---\n");
            printf("%s\n", currentNode->qt->processName);
            
            while(!(currentNode->pNext == NULL)){
                currentNode = currentNode->pNext;
                printf("%s\n", currentNode->qt->processName);
            }        
        }

        printf("-------------------------\n");
        
    }
}


/* runNextProcessInSchedule
 * input: a schedule
 * output: a string
 *
 * Use the schedule to determine the next process to run and for how many time steps.
 * Call "runProcess" to attempt to run the process.  You do not need to print anything.
 * You should return the string "runProcess" returns.  You do not need to use/modify this string in any way.
 */
char* runNextProcessInSchedule( schedule *ps ) {
    /* TODO: complete this function.
    The function "runProcess", "promoteProcess", "loadProcessData", and "freeProcessData"
    in processSimulator.c will be useful in completing this.
    You may want to write a helper function to handle promotion */
    //int numSteps = 0;

	char *ret = NULL;
	int maxSteps;
	int *pNumSteps = &maxSteps;
	char **ppSystemCall = &ret;
	queueType removeProcess;
	process* next;
	
		if(!(isEmpty(ps->foreQueue))){
			//doing foreground stuff
			//bool runProcess( char *pName, char **ppSystemCall, int *pNumSteps )
			maxSteps = FOREGROUND_QUEUE_STEPS;
			next = getNext(ps->foreQueue);
			loadProcessData(next->data);
			bool isFinished = runProcess(next->processName,ppSystemCall,pNumSteps);
			//if finished, remove from queue and free process & process data
			//if not finished, move to back of foreground queue
	   	
			if(isFinished){
				removeProcess = dequeue(ps->foreQueue);
				freeProcessData( );
				free(removeProcess);
			}
	    	else if(!isFinished){
				removeProcess = dequeue(ps->foreQueue);
				enqueue(ps->foreQueue,removeProcess);
			}
	
	        printSchedule(ps);
		}

        
		if(!(isEmpty(ps->backQueue)) && (isEmpty(ps->foreQueue))){
	
			bool isFinished = false;
			next = getNext(ps->backQueue);
			loadProcessData(next->data);

            maxSteps = next->data->heap[1];
            int howLongInQueue = getTimeDifference(next->timeScheduled);

            if(maxSteps + howLongInQueue >= STEPS_TO_PROMOTION)
                maxSteps = STEPS_TO_PROMOTION-howLongInQueue; // if so, reduce StepsToComplete 
                
				isFinished = runProcess(next->processName,ppSystemCall,pNumSteps);
				howLongInQueue = getTimeDifference(next->timeScheduled);
				
                if(!(isFinished) && (howLongInQueue>=STEPS_TO_PROMOTION)){
					promoteProcess(next->processName,next->data);
					removeProcess = dequeue(ps->backQueue);
					enqueue(ps->foreQueue,removeProcess);
				}
				if(isFinished){
					removeProcess = dequeue(ps->backQueue);
					freeProcessData( );
					free(removeProcess);
				}
	        printSchedule(ps);
		}
		
    return ret; /* TODO: be sure to store the value returned by runProcess in ret */
}

/* freeSchedule
 * input: a schedule
 * output: none
 *
 * Free all of the memory associated with the schedule.
 */
void freeSchedule( schedule *ps ) {
    /* TODO: free any data associated with the schedule as well as the schedule itself.
    the function "freeQueue" in queue.c will be useful in completing this. */
	free(ps->foreQueue->qFront);
	free(ps->foreQueue->qRear);
	free(ps->backQueue->qFront);
	free(ps->backQueue->qRear);
	freeQueue(ps->foreQueue);
	freeQueue(ps->backQueue);
	free(ps);
}

int min( int x, int y ){
    if( x<y )
        return x;
    return y;
}
