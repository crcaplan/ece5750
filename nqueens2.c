#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L

int nqueens(int *a, int *b, int n, int profit, int col);
void printBoard(int *a, int n);
int isSafe(int *a, int n, int row, int col);

int total_profit;
int sols;

int nqueens(int *a, int *b, int n, int profit, int col) {
    /* base case: If all queens are placed
    then return true */

    if (col == n) {
        sols += 1;

        if (profit>total_profit){
            total_profit = profit;
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
        if ( isSafe(a, n, i, col) ) {
            /* Place this queen in board[i][col] */
            a[col] = i;
 
            // Make result true if any placement
            // is possible
            res = nqueens(a, b, n, profit + abs(i-col), col + 1) || res;
 
            /* If placing queen in board[i][col]
            doesn't lead to a solution, then
            remove queen from board[i][col] */
            a[col] = -1; // BACKTRACK
        }
    }
 
    /* If queen can not be place in any row in
        this column col then return false */
    return res;
}

int
main(int argc, char **argv) {
    struct timespec start, end;
    double time;
    int n, i, j;
    int *a, *b;
    
    if(argc != 2) {
        printf("Usage: bksb n\nAborting...\n");
        exit(0);
    }
    n = atoi(argv[1]);

    sols = 0;
    total_profit = 0;

    a = (int *) malloc(n * sizeof(int));
    for(i = 0; i < n; i++) a[i] = -1;

    b = (int *) malloc(n * sizeof(int));
    for(i = 0; i < n; i++) b[i] = -1;

    
    //printBoard(b, n);
    clock_gettime(CLOCK_MONOTONIC, &start);
    nqueens(a, b, n, 0, 0);
    printf("%d \n", sols);
    printBoard(b, n);
    printf("%d \n", total_profit);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    time =
    BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    
    printf("Elapsed: %lf seconds\n", time);
    // free(a);
    // free(b);
    return 0;
}

void printBoard(int *a, int n) {
	//a is a vector where i is the column of the NxN matrix and a[i] is the row where 
	//a queen is placed
    for (int j = 0; j < n; j++) {
    	for (int i = 0; i < n; i++) {
    		if (a[i] == j) printf("1 ");
    		else printf("0 ");
    	}
    	printf("\n");
    }
    printf("\n");
}


int isSafe(int *a, int n, int row, int col) {
	//need to check if we can set a[col] = row, based on current entries
    if (a[col] != -1) return 0;
    for (int i = 0; i < col; i++) {
    	if (abs(row - a[i]) == col-i || a[i] == row) return 0;
    }
	return 1;
}
