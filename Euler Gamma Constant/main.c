#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define NUMBER_OF_THREADS 10

/* initializing mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global Variable */
double n;
double global_sum_result = 0;


// @0xC0DEFACE stackoverflow
unsigned int round_closest(unsigned int dividend, unsigned int divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}

void* my_pthread_func( void *arg )
{

    int interval =  (int) arg;
    double number_of_sums_by_each_thread = round_closest(n,NUMBER_OF_THREADS);
    double result = 0;

    int first_index = 1 + interval * number_of_sums_by_each_thread;
    int second_index = first_index + number_of_sums_by_each_thread;

    //printf("%d %d\n", interval, NUMBER_OF_THREADS);
    if (interval == NUMBER_OF_THREADS - 1)
    {
        if (second_index == n) second_index + 1;

        if (second_index > n) second_index = n + 1;
    }

    //printf("First Index: %d | Second Index: %d\n", first_index, second_index);

    for (size_t i = first_index; i < second_index; i++) {
        result += (double) 1 / i;
    }

    pthread_mutex_lock( &mutex );
    global_sum_result += result;
    pthread_mutex_unlock( &mutex );

    return NULL;
}

int main(void)
{
  	pthread_t my_thread[NUMBER_OF_THREADS];

    printf("The number of elements [n]: ");
    scanf("%lf", &n);

    if (n < NUMBER_OF_THREADS)
    {
        printf("N must be higher than number of threads\n");
        exit(1);
    }

    clock_t t;
    double length;
    t = clock();

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_create( &my_thread[i], NULL, my_pthread_func, (void *) i) ) {
            printf("Error while creating the thread.");
            abort();
        }
  	}

    printf("~ Calculating!\n");

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_join ( my_thread[i], NULL ) ) {
    		printf("Error while joining the thread.");
            abort();
      	}
  	}

    t = clock() - t;
    length = ((double)t)/CLOCKS_PER_SEC;

    printf("~ With %d threads it took %f seconds to get [%lf]\n", NUMBER_OF_THREADS, length, global_sum_result - log(n));

  	exit(0);
}
