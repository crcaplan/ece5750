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
    
    //set start and end for cyclic thread assignment
    start = (n-1) - (BLOCK_SIZE * pid);
    end = start - BLOCK_SIZE + 1;
    
    while(start>=0){
        
        //corner case for block sizes that don't divide evenly
        //was used for testing different block sizes in preprocessor directive
        if(end<0)
            end = 0;
    
        for(i = start; i >= end; i--) {
            sum = b[i];
            for(j = n - 1; j > i; j--){
                //busy waiting until b[j] is ready
                while(c[j]==0);
                sum -= a[i][j] * b[j];
            }
            //calculate b[j] and update readiness vector
            b[i] = sum / a[i][i];
            c[i] = 1;
    
            
        }
        //update start and end
        start -= BLOCK_SIZE*p;
        end -= BLOCK_SIZE*p;
    }
    return NULL;
}



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

