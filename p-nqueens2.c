#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define BILLION 1000000000L

int isSafe(int *a, int n, int row, int col);
void * pnqueens(void *varg);
int nqueens(int *avec, int *bvec, int n, int profit, int *profitsi, int *solsi, int col);
void printBoard(int *a, int n);
void printVec(int *avec, int n);


typedef struct {
    int **a, **b;
    int *profitsi, *solsi;
    int pid, n, p;
} GM;


void * pnqueens(void *varg) {
    GM *arg = varg;
    int pid = arg->pid;
    int n = arg->n; 
    int p = arg->p;
    int **a = arg->a; 
    int **b = arg->b;
    int *profitsi = arg->profitsi;
    int *solsi = arg->solsi;
    nqueens(a, b, n, a[pid][0], profitsi, solsi, 1);
    return NULL;
}

int nqueens(int **a, int **b, int n, int profit, int *profitsi, int *solsi, int col, int pid) {
    /* base case: If all queens are placed
    then return true */

    if (col == n) {
        *solsi = *solsi + 1;

        if (profit > *profitsi) {
            *profitsi = profit;
            memcpy(b,a,n*sizeof(int));
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
            if(subprob < p && i < n-1){
            	//do a memcopy to assign current arr to the new processor
            	// probably need a lock/cv here so only 1 processor can assign new subproblem
            	// a[subprob] = a[pid];
            	// profitsi[subprob] = profit + abs(i-col);
                subprob += 1;
            } else {
            	res = nqueens(a, b, n, profit + abs(i-col), profitsi, solsi, col + 1, pid) || res;
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
    int i, j, p, n;
    int **a, **b;
    int *profits;
    int *sols;
    int profit, sol, idxb; 
    double time;
    int *subprobs;

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

    clock_gettime(CLOCK_MONOTONIC, &start);

    *subprobs = n;

    pthread_t *threads = malloc(n * sizeof(threads));

    for(i = 0; i < n; i++) {
        GM *arg = malloc(sizeof(*arg));
        arg->avec = &a[i][0];
        arg->bvec = &b[i][0];
        arg->profitsi = &profits[i];
        arg->solsi = &sols[i];
        arg->n = n;
        arg->p = p;
        arg->pid = i%n;
        while(subprobs < i);
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


