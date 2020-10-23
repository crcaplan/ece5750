#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define BILLION 1000000000L

void
nqueens(double **a, double *b, int n) {
    register int i, j, end;
    register double sum;
    end = n - 1;
    for(i = end; i >= 0; i--) {
        sum = b[i];
        for(j = end; j > i; j--)
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
    }
}

int
main(int argc, char **argv) {
    struct timespec start, end;
    double time;
    int n, i, j;
    double **a, *b, count = 1.0;
    
    if(argc != 2) {
        printf("Usage: bksb n\nAborting...\n");
        exit(0);
    }
    n = atoi(argv[1]);
    a = (double **) malloc(n * sizeof(double *));
    for(i = 0; i < n; i++) {
        a[i] = (double *) malloc(n * sizeof(double));
        for(j = i; j < n; j++) {
            a[i][j] = count;
            count++;
        }
    }
    b = (double *) malloc(n * sizeof(double));
    for(i = 0; i < n; i++) {
        b[i] = count;
        count++;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    bksb(a, b, n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    time =
    BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    
    printf("Elapsed: %lf seconds\n", time);
    for(i = 0; i < n; i++)
        printf("%lf ", b[i]);
    printf("\n");
    return 0;
}

void printBoard(int a**)
{
    static int k = 1;
    printf("%d-\n",k++);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            printf(" %d ", board[i][j]);
        printf("\n");
    }
    printf("\n");
}