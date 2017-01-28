#pragma once
class Image
{
public:
	int *img; // ������ ��������
	int size; // ��� ������
	int id; // id ��������
	Image(int size); 
	int getV(int i, int j); // �������� �������� �������(�������) [0 - 255]
	int getId(); 
	int getI(int i, int j); // ����������� ������� ���������� ������� � ������ �����������
	void highliting(int index); // ��������� ����������� �������� ������ ������������
	void set(int val, int i, int j); // ��������� �������� �������
	~Image();
private:
	static int countForId ; // ����� id -������
	
};

