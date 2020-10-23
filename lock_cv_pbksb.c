#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define BILLION 1000000000L
#define BLOCK_SIZE 1

typedef struct {
    double **a, *b;
    int *c, n, p, pid;
} GM;

//define locks and condition variables
pthread_mutex_t mutex;
pthread_cond_t cv;

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
    int pid, n, p, *c;
    double **a, *b;
    a = arg->a;
    b = arg->b;
    c = arg->c;
    n = arg->n;
    p = arg->p;
    pid = arg->pid;
    
    //set cyclic thread assignment by rows
    start = (n-1) - (BLOCK_SIZE * pid);
    end = start - BLOCK_SIZE + 1;
    
    while(start>=0){
        
        if(end<0)
            end = 0;
    
        for(i = start; i >= end; i--) {
            sum = b[i];
            for(j = n - 1; j > i; j--){
                //block here to see if bj is ready
                pthread_mutex_lock(&mutex);
                //printf("thread %d acquired lock at index %d\n", pid, j);
                    while(c[j]!= 1){
                        //printf("thread %d waiting on index %d, c is %d\n", pid, j, c[j]);
                        pthread_cond_wait(&cv, &mutex);
                        //printf("thread %d awake, c is %d\n", pid, c[j]);
                    }
                pthread_mutex_unlock(&mutex);
                //printf("thread %d dropped lock\n", pid);

                sum -= a[i][j] * b[j];
            }
            b[i] = sum / a[i][i];
            pthread_mutex_lock(&mutex);
            c[i] = 1;
            pthread_mutex_unlock(&mutex);
        
            //signal here to wake up processes waiting on bj in loop above
            pthread_cond_broadcast(&cv);
            //printf("thread %d broadcasting after finishing index %d, c is %d\n", pid, i, c[j]);
            
        }
        //update start and end
        start -= BLOCK_SIZE*p;
        end -= BLOCK_SIZE*p;
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

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv, NULL);

    clock_gettime(CLOCK_MONOTONIC, &start);


    pthread_t *threads = malloc(p * sizeof(threads));
    
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
    // for(i = 0; i < n; i++)
    //     printf("%lf ", b[i]);
    // printf("\n");
    return 0;
}

