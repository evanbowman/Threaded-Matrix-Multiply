#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THRD_NUM 2
// NOTE: optimized for dual core processors. For threaded applications, it is a good
// practice to drop in #cores + 1 threads, so the processor can work on a thread while
// the other perform memory fetches

//-----------------------------------------------------//
// Define a type to hold matrix information.
//-----------------------------------------------------//
typedef struct _MATRIX {
	float **data;
	int rows;
	int cols;
} Matrix;

Matrix m1, m2, mOut;

//-----------------------------------------------------//
// Drop threads to do some of the matrix calculations.
// The program sends out 'runners' to do some of the 
// work on 'slices' of the matrix
//-----------------------------------------------------//
void *runner(void *slice) {
	int s = (int)slice;
	int i, j, k;
	for (i = (s * mOut.rows) / THRD_NUM; i < ((s+1) * mOut.cols) / THRD_NUM; i++) {  
		for (j = 0; j < mOut.rows; j++) {
		 	mOut.data[i][j] = 0;
			for ( k = 0; k < mOut.rows; k++)
	 			mOut.data[i][j] += m1.data[i][k] * m2.data[k][j];
		}
	}
}

//-----------------------------------------------------//
// A function for initializing a test matrix
//-----------------------------------------------------//
void initMatTest(Matrix* m, int rows, int cols) {
	m->rows = rows;
	m->cols = cols;
	m->data = malloc(rows * sizeof(float *));
	for (int i = 0; i < rows; i++)
		m->data[i] = malloc(cols * sizeof(float));
		
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			m->data[i][j] = 17.5;
		}
	}
}

//-----------------------------------------------------//
// A function to free the memory allocated for a Matrix
//-----------------------------------------------------//
void freeMat(Matrix* m) {
	// First free all of the inner arrays, or they'll leak!
	for (int i = 0; i < m->rows; i++) {
		free(m->data[i]);
	}
	free(m->data);
}

int main(int argc, char *argv[]) {
	initMatTest(&m1, 1000, 1000);
	initMatTest(&m2, 1000, 1000);
		
	// Initialize the output matrix with the correct number of rows and columns
	mOut.rows = m1.cols;
	mOut.cols = m2.rows;
	
	// Perform heap allocation based on row and column sizes
	int i;
	mOut.data = malloc(mOut.rows * sizeof(float *));
	for (i = 0; i < mOut.rows; i++)
		mOut.data[i] = malloc(mOut.cols * sizeof(float));
	
	//---For timing purposes---//
	clock_t start, end;
	double cpuTime;
	start = clock();
	//-------------------------//
		
	pthread_t *threads = (pthread_t*) malloc(THRD_NUM * sizeof(pthread_t));
	//pthread_attr_t attr;
	// Drop in some threads to do work on the matrix
	for (i = 1; i < THRD_NUM; i++) {
		pthread_create (&threads[i], NULL, runner, (void*) i);
	}
	
	// On the current thread start working on the first slice
	runner(0);
		
	for (i = 0; i < THRD_NUM; i++) {
		pthread_join(threads[i], NULL);
	}
	free(threads);
	

	//---For timing purposes---//
	end = clock();
	cpuTime = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%f seconds\n", cpuTime);
	//-------------------------//

	freeMat(&mOut);
	freeMat(&m1);
	freeMat(&m2);
}
