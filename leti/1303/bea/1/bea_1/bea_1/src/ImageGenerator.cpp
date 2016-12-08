#include "ImageGenerator.h"

ImagePtr generateImage(int M, int N)
{
	Image * imagePtr = new Image(M, N);

	for (int i = 0; i < M; i++)
		for (int j = 0; j < N; j++)
			imagePtr->matrix[i][j] = 0 + rand() % 256;

	ImagePtr sharedPtr(imagePtr);

	// cout << sharedPtr << endl;

	return sharedPtr;
}