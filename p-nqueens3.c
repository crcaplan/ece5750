//p-nqueens3.c: parallel implementation of nqueens problem using producer-consumer
// 		approach for finding subproblems of the recursive algorithm

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#define BILLION 1000000000L


//function protodeclarations
int isSafe(int *a, int n, int row, int col);
void * pnqueens(void *varg);
int nqueens(int profit, int col, int pid);
void printBoard(int *a, int n);

//GM struct, need to pass pid and column of recursion to pnqueens
typedef struct {
    int pid, col;
} GM;

//global variables for timing: barrier,
//start time, end of setup, end of excution, end of finish
pthread_barrier_t barrier;
struct timespec ts1, ts2, ts3, ts4;

//condition variable global vars
pthread_mutex_t mutex;
pthread_cond_t cv;
GM** buffer;
int considx, prodidx; //index of consumer in array, index of producer

//global variables
int **a, **b;         //each entry is a vector representing placement of queens (tmp and best) for each processor
int *profits, *sols;  //vector of best profits and # of sols, entries for each processor
int p, n;             //number of processors, array size

/* ====================================================================
pnqueens: function spawned for each of the threads, individually calls the
		  recursive function to continue solving the problem
=======================================================================*/
void * pnqueens(void *varg) {
	//each thread waits once created to time setup
	pthread_barrier_wait(&barrier);
	if (varg != NULL) clock_gettime(CLOCK_MONOTONIC, &ts2);

	GM* arg = (GM *) malloc(sizeof(GM)); //GM struct for this processor

	if (varg == NULL) {
		//if this is not the main processor (P0), attempt to consume off
		//the cv buffer once there is a valid entry.
	    pthread_mutex_lock(&mutex);
	    while (buffer[considx] == NULL) {
	        pthread_cond_wait(&cv, &mutex); //wait on cv, tmp unlock mutex
	    }
	    arg = buffer[considx]; //get consumed object
	    considx++;             //increment to next consumed obj
	    pthread_mutex_unlock(&mutex);
	} else {
		//if this is P0, the GM struct was passed as arg 
		arg = varg;
	}
	//read vals from struct, call recursive function
    int pid = arg->pid;
    int col = arg->col;
    nqueens(profits[pid], col, pid); //initial profit is set in profits[pid]
    return NULL;
}

/* ====================================================================
nqueens: recursive function we use to solve the nqueens problem - profit
		 is the initial profit @start of recursive call, col is the column
		 of the arr we attempt to add a queen, pid is the process id.
		 returns 1 if a solution is found, else 0.
=======================================================================*/
int nqueens(int profit, int col, int pid) {

    // base case: if all queens placed, return true
    if (col == n) {
    	//increment # solutions for this processor
        sols[pid] = sols[pid] + 1;
        if (profit > profits[pid]) {
        	//if this sol has better profit, change vector b[pid] to a[pid] &
        	//reassign best profit profits[pid] to this one.
            profits[pid] = profit;
            for(int i=0; i<n; i++) b[pid][i] = a[pid][i];
            return 1;
        }
    }
    
    //in this column (col), sequentially try placing a queen in each row
    int res = 0;  
    for (int i = 0; i < n; i++) {
        if ( isSafe(a[pid], n, i, col) ) {
            //if it's safe to place a queeen in row i of this column, place it & recurse
            a[pid][col] = i;
 
            //if we dont have enough subproblems yet, make new one
            //check i < n-1 b/c we dont want to leave this proc with no work to do
            if (prodidx < p && i < n-1){
            	// producer code: acquire lock & create subproblem for the new processor at the same
            	// point that we've just reached in the recursive call
                pthread_mutex_lock(&mutex);
                GM *newarg = (GM *) malloc(sizeof(GM));
                for (int i = 0; i < n; i++) a[prodidx][i] = a[pid][i]; //same board a[pid]
                newarg->pid = prodidx; //pid at newly created idx
                newarg->col = col + 1; //column of next recursive call
                sols[prodidx] = 0;     //sols init to 0 (new process hasnt found any yet)
                buffer[prodidx] = newarg;
                profits[prodidx] = profit + abs(i-col); //profit incr by different b/t row & col just placed (for recursion)
                prodidx++; //increment prodidx so next produced will be at different idx
                pthread_mutex_unlock(&mutex);
                pthread_cond_broadcast(&cv); //signal here to wake a process waiting to consume

            } else {
            	//otherwise make recursive call as normal
            	res = nqueens(profit + abs(i-col), col + 1, pid) || res;
        	}
 
            //if placing queen didnt lead to solution, remove it & backtrack
            a[pid][col] = -1;
        }
    }
 
    //if queen cant be placed anywhere return false
    return res;
}


/* ====================================================================
main: initialize threads and global variables, and begin execution
=======================================================================*/
int main(int argc, char **argv) {
    int i, j;                //indices
    int profit, sol, idxb;   //for calculating final solution 
    double initializationtime, executiontime, finishtime; //times

    if(argc != 3) {
        printf("Usage: nqueens n p\nAborting...\n");
        exit(0);
    }

    n = atoi(argv[1]);
    p = atoi(argv[2]);
    
    //init a to a pxn matrix of all -1's (no queen placed),
    //where a[pid] represents a board
    a = (int **) malloc(p * sizeof(int *));
    for(i = 0; i < p; i++) {
        a[i] = (int *) malloc(n * sizeof(int));
        for(j = 0; j < n; j++) {
        	a[i][j] = -1;
        }
    }

    //init b to pxn matrix of all 0's where b[pid] represnets the best
    //solution found by processor pid (the board)
    b = (int **) malloc(p * sizeof(int *));
    for(i = 0; i < p; i++) {
        b[i] = (int *) malloc(n * sizeof(int));
        for(j = 0; j < n; j++) {
            b[i][j] = 0;
        }
    }

    //length p vectors of best total profit and #sols found by each proc 
    profits = (int *) malloc(p * sizeof(int));
    sols =    (int *) malloc(p * sizeof(int));
    for (i = 0; i < p; i++) {
    	profits[i] = 0;
    	sols[i] = 0;
    }

    //buffer for the producer-consumer scheme, each entry is the GM struct
    //that processor pid will consume. initially NULL
    buffer = (GM **) malloc(p * sizeof(GM *));
    for (i = 0; i < p; i++){
        buffer[i] = NULL;
    }

    //struct for processor 0: pid=0, col=0 (start of recursion).
    GM *newarg = (GM *) malloc(sizeof(GM));
    newarg->pid = 0;
    newarg->col = 0;
    buffer[0] = newarg;

    //initially, prod-cons buffer indices are both at 1 (past 0 for proc 0)
    considx = 1;
    prodidx = 1;

    //start timing setup of threads 
    pthread_barrier_init(&barrier, NULL, p);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    pthread_t *threads = malloc(p * sizeof(threads));

    //create all threads (all w/ pid > 0 init have NULL inpt struct, wait until it's produced)
    pthread_create(&threads[0], NULL, pnqueens, buffer[0]);
    for(i = 1; i < p; i++) pthread_create(&threads[i], NULL, pnqueens, NULL);

    //wait until all have finished & time end of exec
    for(i = 0; i < p; i++) pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &ts3);

	//find best profit by a proc, sum of all sols found by each, & record
	//idx of best so we can print the board
    profit = 0;
	sol = 0;
	idxb = 0;
    for (i = 0; i < p; i++) {
    	sol = sol + sols[i];
    	if (profits[i] > profit) {
    		idxb = i;
    		profit = profits[i];
    	}
    }

    //print solution, free threads, stop clock
    printBoard(b[idxb], n);
    printf("Found %d solutions, max profit = %d\n", sol, profit);
    free(threads);
    clock_gettime(CLOCK_MONOTONIC, &ts4);
    
    //calc times & print in msec 
    initializationtime = BILLION *(ts2.tv_sec - ts1.tv_sec) +(ts2.tv_nsec - ts1.tv_nsec);
    initializationtime = initializationtime / BILLION;
    executiontime = BILLION *(ts3.tv_sec - ts2.tv_sec) +(ts3.tv_nsec - ts2.tv_nsec);
    executiontime = executiontime / BILLION;
    finishtime = BILLION *(ts4.tv_sec - ts3.tv_sec) +(ts4.tv_nsec - ts3.tv_nsec);
    finishtime = finishtime / BILLION;
    printf("%f,%f,%f\n", initializationtime, executiontime, finishtime);
}

/* ====================================================================
printBoard: print the board represented by the vector avec (which are
	stored at some entry a[pid] of a), n also included as inpt
=======================================================================*/
void printBoard(int *avec, int n) {
	//a is a vector where i is the column of the NxN matrix and a[i] is the row where 
	//a queen is placed
    for (int j = 0; j < n; j++) {
    	for (int i = 0; i < n; i++) {
    		if (avec[i] == j) printf("1 ");
    		else printf("0 ");
    	}
    	printf("\n");
    }
    printf("\n");
}


/* ====================================================================
isSafe: given board represented by avec, returns true if queen can be
	placed @ row, col entry. 
=======================================================================*/
int isSafe(int *avec, int n, int row, int col) {
	//need to check if we can set a[col] = row, based on current entries
    if (avec[col] != -1) return 0; //if already queen in this col
    for (int i = 0; i < col; i++) {
    	//check if there is a conflict if we place queen here
    	if (abs(row - avec[i]) == col-i || avec[i] == row) return 0;
    }
	return 1;
}
