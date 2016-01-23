#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//-----------------------------------------------------//
// Define a type to hold matrix information.
//-----------------------------------------------------//
typedef struct _MATRIX {
	float **data;
	int rows;
	int cols;
} Matrix;

//-----------------------------------------------------//
// Encapsulate thread parameters
//-----------------------------------------------------//
struct threadParams {
	Matrix *m1;
	Matrix *m2;
	Matrix *mOut;
};

struct threadParams threadData;

//-----------------------------------------------------//
// Drop a thread to do some of the matrix calculations.
//-----------------------------------------------------//
void *threadMultiply(void *threadarg) {
	struct threadParams *myData;
	myData = (struct threadParams *) threadarg;
	
	for (int i = myData->m1->cols / 2; i < myData->m1->cols; i++) {
		for (int j = 0; j < myData->m2->rows; j++) {
			myData->mOut->data[i][j] = 0;
			for (int k = 0; k < myData->m1->cols; k++) {
				myData->mOut->data[j][i] += myData->m1->data[j][k] * myData->m2->data[k][i];
			}
		}
	}
	pthread_exit(NULL);
}

//-----------------------------------------------------//
// Now define a function to multiply two matrices.
//-----------------------------------------------------//
void multiply(Matrix* mOut, Matrix* m1, Matrix* m2) {
		
	// Actually multiply the matrices, element by element
	for (int i = 0; i < m1->cols / 2; i++) {
		for (int j = 0; j < m2->rows; j++) {
			// Using add-assign, so init each element to 0
			mOut->data[j][i] = 0;
			for (int k = 0; k < m1->cols; k++) {
				mOut->data[j][i] += m1->data[j][k] * m2->data[k][i];
			}
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
	for (int i = 0; i < m->rows; i++) {
		free(m->data[i]);
	}
	free(m->data);
}

int main(int argc, char *argv[]) {
	Matrix m1, m2, mOut;
	initMatTest(&m1, 1000, 1000);
	initMatTest(&m2, 1000, 1000);
	
	threadData.m1 = &m1;
	threadData.m2 = &m2;
	
	// Initialize the output matrix with the correct number of rows and columns
	mOut.rows = m1.cols;
	mOut.cols = m2.rows;
	
	// Perform heap allocation based on row and column sizes
	mOut.data = malloc(mOut.rows * sizeof(float *));
	for (int i = 0; i < mOut.rows; i++)
		mOut.data[i] = malloc(mOut.cols * sizeof(float));
	
	clock_t start, end;
	double cpuTime;
	start = clock();
		
	pthread_t newThread;
	pthread_attr_t attr;
	
	threadData.mOut = &mOut;
	int iret = pthread_create(&newThread, NULL, threadMultiply, (void *) &threadData);
	
	multiply(&mOut, &m1, &m2);
	
	iret = pthread_join(newThread, NULL);

	end = clock();
	cpuTime = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%f\n", cpuTime);
	
	freeMat(&mOut);
	freeMat(&m1);
	freeMat(&m2);
}
