#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#define BILLION 1000000000L

int isSafe(int *a, int n, int row, int col);
void * pnqueens(void *varg);
int nqueens(int profit, int col, int pid);
void printBoard(int *a, int n);
void printVec(int *avec, int n);


//condition variable global vars
pthread_mutex_t mutex;
pthread_cond_t cv;
int** buffer;
int idx;

/**producer consumer global vars
sem_t empty;
sem_t full;
int in = 0;
int out = 0;
GM** buffer;
pthread_mutex_t mutex;
**/

int **a, **b;
int *profits, *sols;
int p;
int n;



void * pnqueens(void *varg) {
    int *arg = varg;
    int pid = *arg;
    nqueens(profits[pid], 1, pid);
    return NULL;
}

int nqueens(int profit, int col, int pid) {
    /* base case: If all queens are placed
    then return true */

    if (col == n) {
        sols[pid] = sols[pid] + 1;

        if (profit > profits[pid]) {
            profits[pid] = profit;
            for(int i=0; i<n; i++){
            	b[pid][i] = a[pid][i];
            }
            return 1;
        }
    }
    
 

    /* Consider this column and try placing
    this queen in all rows one by one */
    int res = 0;
    int temp = 0;
    for (int i = 0; i < n; i++) {
        /* Check if queen can be placed on
        board[i][col] */
        //printVec(avec,n);
        //printf("%d \n",isSafe(avec,n,i,col));
        if ( isSafe(a[pid], n, i, col) ) {
            /* Place this queen in board[i][col] */
            a[pid][col] = i;
 
            // if we dont have enough subproblems yet, make new one
            // check i < n-1 b/c we dont want to leave this proc with no work to do
            if(idx < p && i < n-1){
            	//do a memcopy to assign current arr to the new processor
            	// probably need a lock/cv here so only 1 processor can assign new subproblem
            	// a[subprob] = a[pid];
            	// profitsi[subprob] = profit + abs(i-col);

                pthread_mutex_lock(&mutex);
                buffer[idx] = (int*) malloc(sizeof(int));
                idx++;
                profits[idx] = profit + abs(i-col);
                pthread_mutex_unlock(&mutex);
            
                //signal here to wake up processes waiting on bj in loop above
                pthread_cond_broadcast(&cv);

            } else {
            	res = nqueens(profit + abs(i-col), col + 1, pid) || res;
        	}
            //printVec(avec,n);
 
            /* If placing queen in board[i][col]
            doesn't lead to a solution, then
            remove queen from board[i][col] */
            a[pid][col] = -1; // BACKTRACK
        }
    }
 
    /* If queen can not be place in any row in
        this column col then return false */
    return res;
}



int 
main(int argc, char **argv) {
    struct timespec start, end;
    int i, j;
    int profit, sol, idxb; 
    double time;
    


    if(argc != 3) {
        printf("Usage: nqueens n p\nAborting...\n");
        exit(0);
    }

    n = atoi(argv[1]);
    p = atoi(argv[2]);
    //maybe try using memset instead of for loops
    a = (int **) malloc(p * sizeof(int *));
    for(i = 0; i < p; i++) {
        a[i] = (int *) malloc(n * sizeof(int));
        for(j = 0; j < n; j++) {
        	if (j == 0) a[i][j] = i;
            else a[i][j] = -1;
        }
    }
    b = (int **) malloc(p * sizeof(int *));
    for(i = 0; i < p; i++) {
        b[i] = (int *) malloc(n * sizeof(int));
        for(j = 0; j < n; j++) {
            b[i][j] = 0;
        }
    }
    profits = (int *) malloc(p * sizeof(int));
    sols =    (int *) malloc(p * sizeof(int));
    for (i = 0; i < p; i++) {
    	profits[i] = 0;
    	sols[i] = 0;
    }

    buffer = (int**) malloc(p * sizeof(int*));
    for (i = 0; i<p; i++){
        buffer[i] = (int *) malloc(sizeof(int));
        *buffer[i] = -1;
    }

    //fix syntax here, how to get arg not to error??
    buffer[0] = 0;

    idx = 1;

    clock_gettime(CLOCK_MONOTONIC, &start);


    pthread_t *threads = malloc(n * sizeof(threads));

    pthread_create(&threads[0], NULL, pnqueens, buffer[0]);

    //consumer stuff happens here
    for(i = 1; i < n; i++) {
        
        pthread_mutex_lock(&mutex);
        while(*buffer[idx]== -1){
            pthread_cond_wait(&cv, &mutex);
        }
        int* arg = buffer[idx];
        pthread_mutex_unlock(&mutex);


        pthread_create(&threads[i], NULL, pnqueens, arg);
    }
    

    for(i = 0; i < n; i++) pthread_join(threads[i], NULL);

    profit = 0;
	sol = 0;
	idxb = 0;
    for (i = 0; i < n; i++) {
    	sol = sol + sols[i];
    	if (profits[i] > profit) {
    		idxb = i;
    		profit = profits[i];
    	}
    }
    printBoard(b[idxb], n);
    printf("Found %d solutions, max profit = %d\n", sol, profit);

    clock_gettime(CLOCK_MONOTONIC, &end);
    free(threads);
    
    time = BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    
    printf("Elapsed time: %lf seconds\n", time);
}

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

void printVec(int *avec, int n) {
	for (int i = 0; i < n; i++) printf("%d ",avec[i]);
	printf("\n");
}


int isSafe(int *avec, int n, int row, int col) {
	//need to check if we can set a[col] = row, based on current entries
    if (avec[col] != -1) return 0;
    for (int i = 0; i < col; i++) {
    	if (abs(row - avec[i]) == col-i || avec[i] == row) return 0;
    }
	return 1;
}



//buffer should point to GM structs (which represent subproblems)

/**
void *producer(void *pno)
{   
    int item;
    for(int i = 0; i < p; i++) {
        item = rand(); // Produce an random item
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        buffer[in] = item;
        printf("Producer %d: Insert Item %d at %d\n", *((int *)pno),buffer[in],in);
        in = (in+1);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
}
void *consumer(void *cno)
{   
    for(int i = 0; i < p; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from %d\n",*((int *)cno),item, out);
        out = (out+1);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
}
**/
