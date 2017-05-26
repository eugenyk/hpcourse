#pragma once
class Image
{
public:
	int *img; // массив пикселей
	int size; // его размер
	int id; // id картинки
	Image(int size); 
	int getV(int i, int j); // получаем значение пикселя(яркость) [0 - 255]
	int getId(); 
	int getI(int i, int j); // преобразуем индексы двумерного массива в индекс одномерного
	void highliting(int index); // выделение прилегающих пикселей вокруг определённого
	void set(int val, int i, int j); // установка значения пикселя
	~Image();
private:
	static int countForId ; // число id -шников
	
};

