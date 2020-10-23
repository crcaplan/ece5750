#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L

int nqueens(int **a, int n, int profit, int col);
void printBoard(int **a, int n);
int isSafe(int **a, int n, int row, int col);

int total_profit;
int **b;
int sols;

int nqueens(int **a, int n, int profit, int col)
{
    /* base case: If all queens are placed
    then return true */

    if (col == n)
    {
        sols += 1;

        if (profit>total_profit){
            
            total_profit = profit;
            for(int i=0; i<n; i++){
                memcpy(b[i], a[i], n);
            }
            return 1;
        }
    }
 
    /* Consider this column and try placing
    this queen in all rows one by one */
    int res = 0;
    int temp = 0;
    for (int i = 0; i < n; i++)
    {
        /* Check if queen can be placed on
        board[i][col] */
        if ( isSafe(a, n, i, col) )
        {
            /* Place this queen in board[i][col] */
            a[i][col] = 1;
 
            // Make result true if any placement
            // is possible
            res = nqueens(a, n, profit + abs(i-col), col + 1) || res;
 
            /* If placing queen in board[i][col]
            doesn't lead to a solution, then
            remove queen from board[i][col] */
            a[i][col] = 0; // BACKTRACK
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
    int **a, **b;
    double count = 1.0;
    
    if(argc != 2) {
        printf("Usage: bksb n\nAborting...\n");
        exit(0);
    }
    n = atoi(argv[1]);


    a = (int **) malloc(n * sizeof(int *));
    for(i = 0; i < n; i++) {
        a[i] = (int *) malloc(n * sizeof(int));
        for(j = i; j < n; j++) {
            a[i][j] = count;
            count++;
        }
    }

    b = (int **) malloc(n * sizeof(int));

    
    clock_gettime(CLOCK_MONOTONIC, &start);
    nqueens(a, n, 0, 0);
    printBoard(b, n);
    //printf('%d', total_profit);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    time =
    BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    
    printf("Elapsed: %lf seconds\n", time);
    return 0;
}

void printBoard(int **a, int n)
{
    static int k = 1;
    printf("%d-\n",k++);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            printf(" %d ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}


int isSafe(int **a, int n, int row, int col)
{
    int i, j;
 
    /* Check this row on left side */
    for (i = 0; i < col; i++)
        if (a[row][i])
            return 0;
 
    /* Check upper diagonal on left side */
    for (i=row, j=col; i>=0 && j>=0; i--, j--)
        if (a[i][j])
            return 0;
 
    /* Check lower diagonal on left side */
    for (i=row, j=col; j>=0 && i<n; i++, j--)
        if (a[i][j])
            return 0;
 
    return 1;
}

