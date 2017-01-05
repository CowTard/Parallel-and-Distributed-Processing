#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define NUMBER_OF_THREADS 4
#define a 0
#define b 1
#define n 5


/* initializing mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global Variable */
float global_sum_result = 0;

double f(double x)
{
    return (1 / ( 1 + x * x) );
}

void* my_pthread_func( void *intrv )
{
	/* This function receives a number as an argument.
       This argument indicates the intervall of the sums that have to make.

       if there is 10 threads and n = 5, and this function receives as argument the number 0, this function will return this sum of f(1)+f(2)
    */

    int interval =  (int) intrv;
    size_t number_of_sums_by_each_thread = n / NUMBER_OF_THREADS;
    double result = 0;

    int first_index = 0 + interval * number_of_sums_by_each_thread;
    int second_index = first_index + number_of_sums_by_each_thread;

    //printf("First Index: %d | Second Index: %d\n", first_index, second_index - 1);
    for (size_t i = first_index; i < second_index; i++) {
        result += f(i);
    }

    pthread_mutex_lock( &mutex );
    //printf("%lf\n", result);
    global_sum_result += result;
    pthread_mutex_unlock( &mutex );

    return NULL;
}

int main(void)
{
  	pthread_t my_thread[NUMBER_OF_THREADS];


    double y0 = f(a), yn = f(b);
    long h = (b-a) / n;
    double s = (y0 + yn) / 2;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_create( &my_thread[i], NULL, my_pthread_func, (void *) i) ) {
            printf("Error while creating the thread.");
            abort();
        }
  	}

    printf("H: %lf\n", h );
    printf("S: %lf\n", (y0 + yn) / 2);

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {

        if( pthread_join ( my_thread[i], NULL ) ) {
    		printf("Error while joining the thread.");
            abort();
      	}
  	}

    printf("Result: %lf\n", h * (s + global_sum_result));
  	exit(0);

}
