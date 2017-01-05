#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define NUMBER_OF_THREADS 10
#define N 100
#define X 10

/* initializing mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global Variable */
double global_sum_result = 0;

double factorial(int n)
{
    if (n == 0) return 1;
    else return n * factorial(n-1);
}


void* my_pthread_func( void *arg )
{

    int interval =  (int) arg;
    size_t number_of_sums_by_each_thread = N / NUMBER_OF_THREADS;
    double result = 0;

    int first_index = 0 + interval * number_of_sums_by_each_thread;
    int second_index = first_index + number_of_sums_by_each_thread;

    for (size_t i = first_index; i < second_index; i++) {
        result += pow(X,i) / factorial(i);
    }

    pthread_mutex_lock( &mutex );
    global_sum_result += result;
    pthread_mutex_unlock( &mutex );

    return NULL;
}

int main(void)
{
  	pthread_t my_thread[NUMBER_OF_THREADS];
    clock_t t;
    double length;

    t = clock();
    for (int i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_create( &my_thread[i], NULL, my_pthread_func, (void *) i) ) {
            printf("Error while creating the thread.");
            abort();
        }
  	}

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_join ( my_thread[i], NULL ) ) {
    		printf("Error while joining the thread.");
            abort();
      	}
  	}

    t = clock() - t;
    length = ((double)t)/CLOCKS_PER_SEC;

    printf("~ With %d threads it took %f seconds to get [%lf]\n", NUMBER_OF_THREADS, length, global_sum_result);

    printf("###################\n");
    t = clock();
    float result_math_h = exp(X);
    t = clock() - t;

    length = ((double)t)/CLOCKS_PER_SEC;
    printf("~ With Math.h it took %f seconds to get [%lf]\n", length, result_math_h);


  	exit(0);
}
