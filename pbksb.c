#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define BILLION 1000000000L

typedef struct {
    double **a, *b;
    int *c, n, p, pid;
} GM;

//n = size of array
//p = number of processors/threads
//a = U array from handout
//b = b' vector from handout
//pid = process id, ie what thread number
//c = synchronization variable - update this to improve performance
//a is double pointer because 2D array
//start sets start of mem block, pid starts at 0
//block is how many rows each thread operates on
//j = n-1 is the last column
void *
pbksb(void *varg) {
    GM *arg = varg;
    register int i, j, start, end;
    register double sum;
    int pid, block, n, p, *c;
    double **a, *b;
    a = arg->a;
    b = arg->b;
    c = arg->c;
    n = arg->n;
    p = arg->p;
    pid = arg->pid;
    block = n / p;
    start = block * pid;
    end = start + block - 1;
    //remove this loop
    for(j = n - 1; j > end; j--)
        while(c[j] == 0);
    for(i = end; i >= start; i--) {
        sum = b[i];
        for(j = n - 1; j > i; j--)
            //block here to see if bj is ready
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
        c[i] = 1;
        //signal here to wake up processes waiting on bj in loop above
    }
    return NULL;
}

/*mutex lock and cv
* eliminate for loop at beginning w/ busy waiting on c
* c array is used to track which elements of x have been computed
* singular condition variable in combination with c array to tell which processes can go
* wait condition checking if bj is ready inside the second nested for loop
* we need a mutex to pass in to condition variable function - what should the mutex be locking around?
* signal after we set c[i] to 1
*
* need mutex to prevent two threads from grabbing the cv at the same time
* define mutex and cv globally, outside of loop
*
* how to deal with each thread having its own b and c?
* how to improve communication to computation ratio?
* bj needs to only be read after it is written to, but bi needs to be read beforehand without being blocked by the cv
*/


int 
main(int argc, char **argv) {
    struct timespec start, end;
    int i, j, p, n, *c;
    double **a, *b, time, count = 1.0;
    if(argc != 3) {
        printf("Usage: pbksb n p\nAborting...\n");
        exit(0);
    }
    n = atoi(argv[1]);
    p = atoi(argv[2]);
    a = (double **) malloc(n * sizeof(double *));
    for(i = 0; i < n; i++) {
        a[i] = (double *) malloc(n * sizeof(double));
        for(j = i; j < n; j++) {
            a[i][j] = count;
            count++;
        }
    }
    b = (double *) malloc(n * sizeof(double));
    c = (int *) malloc(n * sizeof(int));
    for(i = 0; i < n; i++) {
        b[i] = count;
        count++;
        c[i] = 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t *threads = malloc(p * sizeof(threads));
    
    //define locks and condition variables
    pthread_mutex_t mutex;
    pthread_cond_t c;

    //initialize locks and condition variables
    for(i = 0; i < p; i++) {
        GM *arg = malloc(sizeof(*arg));
        arg->a = a;
        arg->b = b;
        arg->c = c;
        arg->n = n;
        arg->p = p;
        arg->pid = i;
        pthread_create(&threads[i], NULL, pbksb, arg);
    }
    for(i = 0; i < p; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    free(threads);
    
    time = BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
    time = time / BILLION;
    
    printf("Elapsed time: %lf seconds\n", time);
    for(i = 0; i < n; i++)
        printf("%lf ", b[i]);
    printf("\n");
    return 0;
}

