#include<stdio.h>
#include<omp.h>

#define SIZE_OF_INITIAL_LIST 50

int main()
{
	omp_init_lock(&lock);
	srand(time(NULL));

    // Create and populate the sieve's list
    int sieve_list[SIZE_OF_INITIAL_LIST];

    for(int i = 0; i < SIZE_OF_INITIAL_LIST; i++)
    {
        sieve_list[i] = i + 1;
    }

	omp_set_num_threads(4);

    #pragma omp parallel for firstprivate(a)
		for(i=0;i<10;i++)
		{
			printf("Thread %d a=%d\n",omp_get_thread_num(),a);
			a++;
		}

}
