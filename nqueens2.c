//nqueens2.c: serial implementation of nqueens problem using a vector to store
//       the solutions matrix and a recursive algorithm to solve

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L

//function protodeclarations
int nqueens(int *a, int *b, int n, int profit, int col);
void printBoard(int *a, int n);
int isSafe(int *a, int n, int row, int col);

int total_profit; //best profit found so far
int sols; //total # solutions found so far 

/* ====================================================================
nqueens: recursive function we use to solve the nqueens problem - profit
         is the initial profit @start of recursive call, col is the column
         of the arr we attempt to add a queen, n is the size of matrix,
         a is a vector storing the current placed queens, b is the best
         sol found so far (board representation).
         returns 1 if a solution is found, else 0.
=======================================================================*/
int nqueens(int *a, int *b, int n, int profit, int col) {

    /// base case: if all queens placed, return true
    if (col == n) {
        //increment total # solutions found
        sols += 1;
        //if this sol has better profit, change vector b to a &
        //reassign best profit total_prodit to this one.
        if (profit>total_profit){
            total_profit = profit;
            memcpy(b,a,n*sizeof(int));
            return 1;
        }
    }
 
    //in this column (col), sequentially try placing a queen in each row
    int res = 0;
    int temp = 0;
    for (int i = 0; i < n; i++) {
        if ( isSafe(a, n, i, col) ) {
            //if it's safe to place a queeen in row i of this column, place it & recurse
            a[col] = i;
 
            // recursive call: make res true if sol found already or sol found down
            // this branch of the tree. incr profit by profit incurred by placing queen
            // in previous line, incr column for recursion
            res = nqueens(a, b, n, profit + abs(i-col), col + 1) || res;
 
            //if placing queen didnt lead to solution, remove it & backtrack
            a[col] = -1;
        }
    }
 
    //if queen cant be placed anywhere return false
    return res;
}

/* ====================================================================
main: initialize threads and global variables, and begin execution
=======================================================================*/
int main(int argc, char **argv) {
    struct timespec start, end; //timing variables
    double time; 
    int n, i, j; //indices, size of matrix
    int *a, *b;  //our a and b vector to keep track of current & best solution, respectively
    
    if(argc != 2) {
        printf("Usage: bksb n\nAborting...\n");
        exit(0);
    }
    n = atoi(argv[1]);

    //initially no solutions found and no positive profit
    sols = 0;
    total_profit = 0;

    //allocate a and b, setting entries to -1 to indicate queen has not been placed
    a = (int *) malloc(n * sizeof(int));
    for(i = 0; i < n; i++) a[i] = -1;
    b = (int *) malloc(n * sizeof(int));
    for(i = 0; i < n; i++) b[i] = -1;

    //start timing & call the recursive function
    clock_gettime(CLOCK_MONOTONIC, &start);
    nqueens(a, b, n, 0, 0);

    //print solutions & get the end time 
    printBoard(b, n);
    printf("Found %d solutions, max profit = %d\n", sols, total_profit);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    //calculate execution time & print
    time = BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    printf("%lf\n", time);
    
    return 0;
}

/* ====================================================================
printBoard: print the board represented by the vector avec (which are
    stored at some entry a[pid] of a), n also included as inpt
=======================================================================*/
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

/* ====================================================================
isSafe: given board represented by avec, returns true if queen can be
    placed @ row, col entry. 
=======================================================================*/
int isSafe(int *a, int n, int row, int col) {
	//need to check if we can set a[col] = row, based on current entries
    if (a[col] != -1) return 0; //if already queen in this col
    for (int i = 0; i < col; i++) {
        //check if there is a conflict if we place queen here
    	if (abs(row - a[i]) == col-i || a[i] == row) return 0;
    }
	return 1;
}
