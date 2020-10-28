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

typedef struct {
    int pid, col;
} GM;

//condition variable global vars
pthread_mutex_t mutex;
pthread_cond_t cv;
GM** buffer;
int considx, prodidx;


int **a, **b;
int *profits, *sols;
int p;
int n;

void * pnqueens(void *varg) {
    GM *arg = varg;
    int pid = arg->pid;
    int col = arg->col;
    nqueens(profits[pid], col, pid);
    return NULL;
}

int nqueens(int profit, int col, int pid) {
    /* base case: If all queens are placed
    then return true */

    if (col == n) {
        sols[pid] = sols[pid] + 1;
        //printBoard(a[pid],n);
        //printf("processor = %d\n",pid);

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
        /* Check if queen can be placed on*/
        if ( isSafe(a[pid], n, i, col) ) {
            /* Place this queen in board[i][col] */
            a[pid][col] = i;
 
            // if we dont have enough subproblems yet, make new one
            // check i < n-1 b/c we dont want to leave this proc with no work to do
            if (prodidx < p && i < n-1){
            	//do a memcopy to assign current arr to the new processor
            	// probably need a lock/cv here so only 1 processor can assign new subproblem
            	// a[subprob] = a[pid];
            	// profitsi[subprob] = profit + abs(i-col);

                pthread_mutex_lock(&mutex);
                GM *newarg = (GM *) malloc(sizeof(GM));
                for (int i = 0; i < n; i++) {
                    a[prodidx][i] = a[pid][i];
                }
                newarg->pid = prodidx;
                newarg->col = col + 1;
                sols[prodidx] = 0;
                buffer[prodidx] = newarg;
                profits[prodidx] = profit + abs(i-col);
                prodidx++;
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
        	a[i][j] = -1;
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

    buffer = (GM **) malloc(p * sizeof(GM *));
    for (i = 0; i < p; i++){
        buffer[i] = NULL;
    }

    //fix syntax here, how to get arg not to error??
    GM *newarg = (GM *) malloc(sizeof(GM));
    newarg->pid = 0;
    newarg->col = 0;
    buffer[0] = newarg;
    considx = 1;
    prodidx = 1;

    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t *threads = malloc(p * sizeof(threads));
    pthread_create(&threads[0], NULL, pnqueens, buffer[0]);

    //consumer stuff happens here
    for(i = 1; i < p; i++) {
        
        pthread_mutex_lock(&mutex);
        while (buffer[considx] == NULL) {
            pthread_cond_wait(&cv, &mutex);
        }
        GM* arg = buffer[considx];
        considx++;
        pthread_mutex_unlock(&mutex);
        pthread_create(&threads[i], NULL, pnqueens, arg);
    }
    

    for(i = 0; i < p; i++) pthread_join(threads[i], NULL);

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
