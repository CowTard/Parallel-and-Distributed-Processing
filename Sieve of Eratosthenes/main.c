#include<stdio.h>
#include<omp.h>
#include<math.h>

#define SIZE_OF_INITIAL_LIST 120

int main()
{
    // Create and populate the sieve's list
    int sieve_list[SIZE_OF_INITIAL_LIST], primes[SIZE_OF_INITIAL_LIST];
    int check_limit = floor(sqrt(SIZE_OF_INITIAL_LIST));

    // Starting the arrays. Prime array has 1 if the index is not prime, or 0 otherwise.
    for(int i = 0; i < SIZE_OF_INITIAL_LIST; i++)
    {
        sieve_list[i] = i;
        primes[i] = -1; // Not yet seen
    }

    /*
        1. Choose the next candidate to be prime
        2. Use parallel process to calculate the elements to crossout.
    */


	omp_set_num_threads(4);

    #pragma omp parallel for
        for(int i = 2; i < check_limit; i++)
		{
            if (primes[i] != -1)
            {
                continue;
            }
            else
            {
                primes[i] = 0;
                for (size_t t = i * 2; t < SIZE_OF_INITIAL_LIST; t += i) {
                    primes[t] = 1;
                }
            }
		}

    printf("The primes until %d are [", SIZE_OF_INITIAL_LIST);
    for (size_t prime_cand = 0; prime_cand < SIZE_OF_INITIAL_LIST; prime_cand++) {

        if (prime_cand < 2) continue; // 0 and 1 are not primes.
        if (primes[prime_cand] == 0 || primes[prime_cand] == -1)
        {
            printf("%d ", sieve_list[prime_cand]);
        }
    }
    printf("]\n");
}
