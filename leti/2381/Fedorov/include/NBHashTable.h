#ifndef NBHASHTABLE_H
#define NBHASHTABLE_H

#include <cstdio>
#include <atomic>
#include <algorithm> 
#include "ProbeBound.h"
#include "VersionState.h"
#define EMPTY_FLAG -1
#define NUM_STATE_BITS 3


//Это определение типа существует исключительно для того, чтобы помочь в дальнейшем понимании кода 
//Если мы когда-нибудь хотели, чтобы добавить поддержку нескольких типов, а не просто инц
//Мы можем сразу отличить, какие типы мы должны быть редактирования.
//Мы также могли бы сделать нашу переменную шаблона класса точно так же, чтобы свести к минимуму корректировки кода.
typedef int NBType;

//typedef int ProbeBound;
//Это наши два значения, которые будут автоматически обменены
//Мы напишем методы, которые принимают
//Тип int VersionState;
//Тип int ProbeBound;
typedef struct buckett {
	VersionState *vs;
	NBType key;
} BucketT;

class NBHashTable {
	
	
	int kSize;
	ProbeBound *bounds;
	BucketT *buckets;
	
	BucketT* getBucketValue(int startIndex, int probeJumps);
	bool doesBucketContainCollision(int startIndex, int probeJumps);
	void initProbeBound(int startIndex);
	int getProbeBound(int startIndex);
	void conditionallyRaiseBound(int startIndex, int probeJumps);
	void conditionallyLowerBound(int startIndex, int probeJumps);
	int hash(NBType n);
	bool assist(NBType key, int h, int i, int ver_i);
	
	public:
		NBHashTable(int ks = 8);
		~NBHashTable();
		bool insert(NBType n);
		bool put(NBType n);
		bool remove(NBType n);
		bool contains(int);
		int size();
		
		void printHashTableInfo();

class ProbeBound : public std::atomic_int {
	
	static bool getBit(int val, int bit);
	
	public:


		ProbeBound(int pb = 0, bool scanning = false);
		void set(int pb, bool scanning);
		static int makeRaw(int pb, bool scanning);
		
	
           
		static int getBound(int raw);
		static bool isScanning(int raw);
		

                // Предназнаяены только для тестирования
		static void printBits(int ref);
		static int setBit(int, bool, int);
};


};

class VersionState : public std::atomic_int {ctnnth

	static bool getBit(int, int);
	
	public:

                // объявление первого, у всех изначально к нему есьт доступ
		enum State
		{
			BUSY = 0,
			MEMBER,
			INSERTING,
			EMPTY,
			COLLIDED,
			VISIBLE
		};
	
		static int setBit(int, bool, int);
		

                // Конструкторы, отправитель
		VersionState(int version, VersionState::State state);
		void set(int version, VersionState::State state);
		static int makeRaw(int version, VersionState::State state);
		

                // Получатель, позволяет получить данные доступ к которым ограничены
		static VersionState::State getState(int raw);
		static int getVersion(int raw);
		
		static const char *getStateString(VersionState::State state);
};


#endif