#ifndef NBHASHTABLE_H
#define NBHASHTABLE_H

#include <cstdio>
#include <mutex>
#include <atomic>
#include <algorithm> 
#define EMPTY_FLAG -1
#define NUM_STATE_BITS 3


typedef int NBType;

//typedef int ProbeBound;
//Два значения, которые будут автоматически обменены
//Методы, которые принимают
//Тип int VersionState;
//Тип int ProbeBound;
typedef struct buckett {
	VersionState *vs;
	NBType key;
} BucketT;

class NBHashTable {
	
	std::mutex mainMutex;
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
		

             
		static void printBits(int ref);
		static int setBit(int, bool, int);
};


};

class VersionState : public std::atomic_int {ctnnth

	static bool getBit(int, int);
	
	public:

                // объявление первого, у всех изначально к нему есть доступ
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
		

                // Конструкторы, сетеры
		VersionState(int version, VersionState::State state);
		void set(int version, VersionState::State state);
		static int makeRaw(int version, VersionState::State state);
		

                // Гетеры, позволяет получить данные, доступ к которым ограничены
		static VersionState::State getState(int raw);
		static int getVersion(int raw);
		
		static const char *getStateString(VersionState::State state);
};


#endif