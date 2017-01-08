Write a program that calculates the prime numbers using the sieve of Eratosthenes. The program
should be made with the use of OpenMP.

Example:
We have a sequence of natural numbers:
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, ...

At first, we eliminate multiples of 2 by which we get:
1, 2, 3, * 5, * 7, * 9, * 11, * 13, ...

In the next step we remove multiples of 3:
1, 2, 3, * 5, * 7, *, *, 11 *, 13, ...
Further we delete the multiple of 5, 7 etc.
