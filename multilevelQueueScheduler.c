#include <stdlib.h>
#include <string.h>
#include "multilevelQueueScheduler.h"

static const int STEPS_TO_PROMOTION = 50;
static const int FOREGROUND_QUEUE_STEPS = 5;

int min( int x, int y );

/**** CUSTOM FUNCTIONS ***/
void red();
void yellow();
void reset();
void printQueue(Queue* q, char* queuePriority);
void printSchedule(schedule *ps);    

void cyan(){ printf("\033[0;36m");}
void red(){printf("\033[1;31m");}
void yellow(){printf("\033[1;33m");}
void reset(){printf("\033[0m");}

/* printSchedule
 * input: schedule
 * output: void
 *
 * Prints the queues inside the schedule
 */
void printSchedule(schedule *ps){ 
    
    printQueue(ps->foreQueue, "FOREGROUND");
    printQueue(ps->backQueue, "BACKGROUND");
}

/* printQueue
 * input: queue, string
 * output: void
 *
 * Prints the processes in the provided queue
 */
void printQueue(Queue* q, char* queuePriority){
    LLNode* currentNode;
    
    yellow();
    if ( (!isEmpty(q))){
        currentNode = q->qFront;
        printf("--- %s---\n", queuePriority);
        while(currentNode != NULL){
            // Added check to only print the TIQ for background processes            
            if ( strcmp(queuePriority,"BACKGROUND") == 0)
                printf("%-20s  \t TIQ: %d\n", currentNode->qt->processName, currentNode->qt->timeInQueue);
            else
                printf("%-20s\n", currentNode->qt->processName);
            
            currentNode = currentNode->pNext;
        }     
    }
    reset();
}

/* updateProcessTimes
 * input: schedule, stepsCompleted
 * output: void
 *
 * Adds the number of steps completed to each BACKGROUND processes' timeInQueue
 */
void updateProcessTimes(schedule *ps, int stepsCompleted){
	LLNode* currentNode;
    
	if(!isEmpty(ps->backQueue)){
		currentNode = ps->backQueue->qFront;

		while(currentNode != NULL){
			currentNode->qt->timeInQueue += stepsCompleted;
			currentNode = currentNode->pNext;
		}
	}
}

/* getMaxTIQ
 * input: queue
 * output: the max TIQ as an integer
 *
 * Looks through all the BACKGROUND processes to find the max timeInQueue
 */
int getMaxTIQ(Queue* q){
	int maxTIQ = 0;
	LLNode* currentNode;

	if(!isEmpty(q)){
		currentNode = q->qFront;

		while(currentNode != NULL){
			if(currentNode->qt->timeInQueue >= maxTIQ)
				maxTIQ = currentNode->qt->timeInQueue;
            
			currentNode = currentNode->pNext;
		}
	}
    
	return maxTIQ;
}


void findProcessToPromote(schedule* ps){
	//int maxTIQ = 0;
	LLNode* currentNode;
	process* removeProcess;

    /*
        1->2->N
    */
    
	if(!isEmpty(ps->backQueue)){
		currentNode = ps->backQueue->qFront;
		if(currentNode->qt->timeInQueue >= STEPS_TO_PROMOTION){
			promoteProcess(currentNode->qt->processName,currentNode->qt->data);
            removeProcess = dequeue(ps->backQueue);
            enqueue(ps->foreQueue,removeProcess);
			//we need to promote
		}else{
			removeProcess = dequeue(ps->backQueue);
            enqueue(ps->backQueue,removeProcess);
            //move process to back of backQueue
        }

		while(currentNode->pNext != NULL){
			currentNode = currentNode->pNext;
    		if(currentNode->qt->timeInQueue >= STEPS_TO_PROMOTION){
    			promoteProcess(currentNode->qt->processName,currentNode->qt->data);
	            removeProcess = dequeue(ps->backQueue);
	            enqueue(ps->foreQueue,removeProcess);
				//we need to promote
    		}else{
                removeProcess = dequeue(ps->backQueue);
	            enqueue(ps->backQueue,removeProcess);
	            //move process to back of backQueue
            }
		}
	}
	
}



//=============================================================================================

/* createSchedule
 * input: none
 * output: a schedule
 *
 * Creates and return a schedule struct.
 */
schedule* createSchedule( ) {  
	schedule* sched = (schedule*)malloc(sizeof(schedule));
    
	if( sched==NULL ){
        fprintf(stderr, "Error: Unable to allocate data");
        exit(-1);
    }
	else{
		sched->foreQueue = createQueue();
		sched->backQueue = createQueue();
		sched->currentTime = 0;
	}

    return sched;
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
	newProcess->timeInQueue = 0;
	newProcess->data = newProcessData;
	newProcess->timeScheduled = ps->currentTime;

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
        /*
            If one of the bg processes->timeInQueue + maxSteps >= 50:
                maxSteps = 50 - process->timeInQueue
        */
		maxSteps = FOREGROUND_QUEUE_STEPS;
		int maxTIQ;
		maxTIQ = getMaxTIQ(ps->backQueue);
		if(maxTIQ + maxSteps >= STEPS_TO_PROMOTION){
			maxSteps = STEPS_TO_PROMOTION - maxTIQ;
		}
		
        next = getNext(ps->foreQueue);
        loadProcessData(next->data);
        bool isFinished = runProcess(next->processName,ppSystemCall,pNumSteps);
		updateProcessTimes(ps,*pNumSteps);
		ps->currentTime += *pNumSteps;
		findProcessToPromote(ps);
        /* if a bg process is promotable, then promote it 

            look at all bg processes and promote the ones that are >= 50

            O->1->2->X->4     X->4->0->1->2
        */
            // promoteProcess(next->processName,next->data);
            // removeProcess = dequeue(ps->backQueue);
            // enqueue(ps->foreQueue,removeProcess);
		
        
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

/**********************************************************
        maxSteps = next->data->heap[1];
		int maxTIQ;
		maxTIQ = checkTIQ(ps);
		if(maxTIQ + maxSteps >= STEPS_TO_PROMOTION){
			maxSteps = STEPS_TO_PROMOTION - maxTIQ;
		}
*************************************************************/
        
        // if(maxSteps + next->timeInQueue >= STEPS_TO_PROMOTION)
        //     maxSteps = STEPS_TO_PROMOTION-next->timeInQueue; // if so, reduce StepsToComplete 
            
        isFinished = runProcess(next->processName,ppSystemCall,pNumSteps);
		updateProcessTimes(ps,*pNumSteps);
		ps->currentTime += *pNumSteps;
        //howLongInQueue = getTimeDifference(next->timeScheduled);
        
        if(!(isFinished) && (next->timeInQueue>=STEPS_TO_PROMOTION)){
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
    }//end of if(!(isEmpty(ps->backQueue)) && (isEmpty(ps->foreQueue)))
		
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