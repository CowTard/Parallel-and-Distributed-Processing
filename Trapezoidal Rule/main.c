#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define NUMBER_OF_THREADS 5
#define a 0
#define b 1
#define n 100


/* initializing mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global Variable */
float global_sum_result = 0;

float f(double x)
{
    return (1 / ( 1 + x * x) );
}

void* my_pthread_func( void *intrv )
{
	/* This function receives a number as an argument.
       This argument indicates the intervall of the sums that have to make.

       if there is 10 threads and n = 5, and this function receives as argument the number 0, this function will return this sum of f(1)+f(2)

       a + i*h
    */

    int interval =  (int) intrv;
    size_t number_of_sums_by_each_thread = n / NUMBER_OF_THREADS;
    float result = 0;

    int first_index = 1 + interval * number_of_sums_by_each_thread;
    int second_index = first_index + number_of_sums_by_each_thread;

    if (interval == NUMBER_OF_THREADS - 1)
        second_index -= 1;

    //printf("First Index: %d | Second Index: %d\n", first_index, second_index - 1);
    for (size_t i = first_index; i < second_index; i++) {
        result += f(a + i * 0.01);
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

    float y0 = f(a), yn = f(b);
    float h = (float)(b-a) / n;
    float s = (y0 + yn) / 2;

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

    printf("~ With %d threads:\n", NUMBER_OF_THREADS);
    printf("    > Result: %lf\n", 4 * h * (s + global_sum_result));
    printf("    > In %f seconds!\n",length);
  	exit(0);

}
