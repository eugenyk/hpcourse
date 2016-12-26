#pragma once
class Image
{
public:
	int *img;
	int size;
	Image(int size);
	int getV(int i, int j);
	int getI(int i, int j);
	void highliting(int index);
	void set(int val, int i, int j);
	~Image();
};

