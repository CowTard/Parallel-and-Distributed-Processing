#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>
#include<time.h>

int main(int argc, char* argv[])
{
    time_t t;
    srand((unsigned) time(&t));

    // Get maximum exponent
    int maximum_exp = 4;

    // Array where it is hold the Xn values
    double values_of_xn[maximum_exp];

    // Array where it is hold the Bn values
    int values_of_bn[maximum_exp];

    // Randomize the values of Bn
    for (size_t i = 0; i < maximum_exp; i++) {
        values_of_bn[i] = rand() % 20;
    }

    // Creating An before Xn for each equations
    int values_of_an[maximum_exp][maximum_exp];

    printf("The matrix: \n\n");
    for (size_t i = 0; i < maximum_exp; i++) {

        for (size_t t = 0; t < maximum_exp; t++) {

            if (t <= i)
            {
                values_of_an[i][t] = rand() % 20 + 1;
            }
            else
            {
                values_of_an[i][t] = 0;
            }
            printf("%d ", values_of_an[i][t]);
        }
        printf("\n");
    }

    printf("\n\nThe B values: \n\n");
    for (size_t i = 0; i < maximum_exp; i++) {
        printf("B[%d] = %d\n",i, values_of_bn[i]);
    }

    // Start the openmpi part
    omp_set_num_threads(4);

    for (size_t i = 0; i < maximum_exp; i++) {
        double adding_up = 0;

        #pragma omp parallel for reduction(+:adding_up)
        for (size_t t = 0; t < i; t++) {
            adding_up += values_of_an[i][t] * values_of_xn[t];
        }

        values_of_xn[i] = (values_of_bn[i] - adding_up)/values_of_an[i][i];
    }

    printf("\n\nThe Xn values: \n\n");
    for (size_t i = 0; i < maximum_exp; i++) {
        printf("X%d = %0.3f\n",i, values_of_xn[i]);
    }

}
