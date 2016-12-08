#include "UnitTests.h"

void testFirstAlg()
{
	Image * image = new Image(10, 10);

	for (int i = 0; i < image->M; i++)
		for (int j = 0; j < image->N; j++)
			image->matrix[i][j] = 0;

	image->matrix[1][1] = 100;
	image->matrix[5][5] = 100;
	image->matrix[7][7] = 100;

	ImagePtr ptr(image);
	vector<Pixel> maxPixels = getMaxBrightness(ptr);
	bool pass = true;

	if (maxPixels.size() != 3)
		pass = false;

	if (pass && (maxPixels[0].i != 1 || maxPixels[0].j != 1))
		pass = false;

	if (pass && (maxPixels[1].i != 5 || maxPixels[1].j != 5))
		pass = false;

	if (pass && (maxPixels[2].i != 7 || maxPixels[2].j != 7))
		pass = false;

	cout << "getMaxBrightness:\t" << (pass ? "Pass" : "Fail") << endl;
}

void testSecondAlg()
{
	Image * image = new Image(10, 10);

	for (int i = 0; i < image->M; i++)
		for (int j = 0; j < image->N; j++)
			image->matrix[i][j] = 255;

	image->matrix[1][1] = 100;
	image->matrix[5][5] = 100;
	image->matrix[7][7] = 100;

	ImagePtr ptr(image);
	vector<Pixel> minPixels = getMinBrightness(ptr);
	bool pass = true;

	if (minPixels.size() != 3)
		pass = false;

	if (pass && (minPixels[0].i != 1 || minPixels[0].j != 1))
		pass = false;

	if (pass && (minPixels[1].i != 5 || minPixels[1].j != 5))
		pass = false;

	if (pass && (minPixels[2].i != 7 || minPixels[2].j != 7))
		pass = false;

	cout << "getMinBrightness:\t" << (pass ? "Pass" : "Fail") << endl;
}

void testThirdAlg()
{
	Image * image = new Image(10, 10);

	for (int i = 0; i < image->M; i++)
		for (int j = 0; j < image->N; j++)
			image->matrix[i][j] = 255;

	image->matrix[1][1] = 100;
	image->matrix[5][5] = 100;
	image->matrix[7][7] = 100;

	ImagePtr ptr(image);
	vector<Pixel> pixels = getEquBrightness(ptr, 100);
	bool pass = true;

	if (pixels.size() != 3)
		pass = false;

	if (pass && (pixels[0].i != 1 || pixels[0].j != 1))
		pass = false;

	if (pass && (pixels[1].i != 5 || pixels[1].j != 5))
		pass = false;

	if (pass && (pixels[2].i != 7 || pixels[2].j != 7))
		pass = false;

	cout << "getEquBrightness:\t" << (pass ? "Pass" : "Fail") << endl;
}

void testAll()
{
	testFirstAlg();
	testSecondAlg();
	testThirdAlg();
}