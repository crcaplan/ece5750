#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define BILLION 1000000000L

typedef struct {
    double **a, *b;
    int *c, n, p, pid;
} GM;

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
    for(j = n - 1; j > end; j--)
        while(c[j] == 0);
    for(i = end; i >= start; i--) {
        sum = b[i];
        for(j = n - 1; j > i; j--)
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
        c[i] = 1;
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

