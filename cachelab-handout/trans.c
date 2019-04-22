/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
        	tmp = A[i][j];
        	B[j][i] = tmp;
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;
    int tmpUL1;
    int tmpUL2;
    int tmpUR1;
    int tmpUR2;
    int tmpLL1;
    int tmpLL2;
    int tmpLR1;
    int tmpLR2;
    //for(int a = 0; a < 128; a++) {

		for (i = 0; i < 16; i = i+2) {
			for (j = 0; j < 16; j=j+2) {
			    tmpUL1 = A[i][j];
			    tmpUL2 = B[j][i];
			    tmpUR1 = A[i][j + 1];
			    tmpUR2 = B[j + 1][i];
			    tmpLL1 = A[i + 1][j];
			    tmpLL2 = B[j][i + 1];
			    tmpLR1 = A[i + 1][j + i];
			    tmpLR2 = B[j + 1][i + 1];

			    A[i][j] = tmpUL2;
			    B[j][i] = tmpUL1;
			    A[i][j + 1] = tmpUR2;
			    B[j + 1][i] = tmpUR1;
			    A[i + 1][j] = tmpLL2;
			    B[j][i + 1] = tmpLL1;
			    A[i + 1][j + i] = tmpLR2;
			    B[j + 1][i + 1] = tmpLR1;
			}
		}
    //}
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

