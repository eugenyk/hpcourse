/*
* Name				Dmitrii Shakshin, Igor Pechko
* Student group		0303
* Email				d.shakshin@gmail.com
* Project	Implement a Lock-Free Linked List (based on Valois' paper)
*/
//
//////////////////////////////////////////////////////////////////////

//	Description of Program Files
//
//		File		Purpose
//		----		-------
//
//
//
//	test.cpp	Spawns threads and tests the list.
//
//				Implementation of test member functions.
//				This class performs a test on the lock free linked list.
//				The test involves many threads.
//				Each thread initially inserts many nodes into the list.
//				Then, each tread iterates to the 25th node in the list and begins
//				inserting and deleting a bunch of nodes.  It does this many times.
//				The objective is to create contention by having all of the threads
//				inserting and deleting in the same area.
//
/*
				Important functions:

				This class conatains 2 CriticalSection objects for syncronizing the s
				screen ouput and avoiding race conditions on the thread number member variable.

					ThreadFunc
					multithreaded_test()
					integrityTest()
					TestFuctionF()


*/

// Include these files.
#include <iostream>
#include <stdio.h>

#include "test.h"

#include "lockFreeList.h"

#include <thread>
#ifdef OS_WIN
#include "mingw.thread.h"
#endif

// Tell which library to use for the particular command.
using std::cout;
using std::cerr;
using std::endl;
using std::thread;

typedef LockFreeList<int>::iterator ListItr;


//******************************************************************************************
//******************************************************************************************
//******************************************************************************************
//*************                                                               **************
//*************                          Test Object                          **************
//*************                                                               **************
//******************************************************************************************
//******************************************************************************************
//******************************************************************************************
//	test.cpp	Implementation of test member functions.
//				This class performs a test on the lock free linked list.
//				The test involves many threads.
//				Each thread initially inserts many nodes into the list.
//				Then, each tread iterates to the 25th node in the list and begins
//				inserting and deleting a bunch of nodes.  It does this many times.
//				The objective is to create contention by having all of the threads
//				inserting and deleting in the same area.
//
//*******************************************************
//*******************************************************
//********                                      *********
//********         Test Object Constructor      *********
//********                                      *********
//*******************************************************
//*******************************************************
// Set the member variables.
// These are the initial variable values.
test::test()
{
    // Set member variables.
    mv_my_number = 1;		// counter so everyone gets a unique number to insert
    net_added_nodes = 0;	// Initially 0;

}

//*******************************************************
//*******************************************************
//********                                      *********
//********         test Object Destructor       *********
//********                                      *********
//*******************************************************
//*******************************************************
// There is nothing to destroy, because all of my handles
// closed as soon as they are no longer needed, and all of
// my objects are declared as objects, not pointers, and
// they therefore distappear automatically when they go
// out of scope.
test::~test() { }

//***********************************
//*****     get_my_number()     *****
//***********************************
int test::get_my_number() { // Returns a the current number and increments the counter.
    lock_guard<mutex> lock(m_sync_my_number); /// only one thread can increment number at a time

    int temp = mv_my_number;	// set temp to current
    mv_my_number++;			// increment current
    return temp;				// return previous

    // lock is destroyed on exit.
}	// end get my number

//*******************************************************
//*******************************************************
//********                                      *********
//********          multithreaded_test          *********
//********                                      *********
//*******************************************************
//*******************************************************
// This function is the thread helper (launcher) function.
// It just calls the thread and waits for it.
void test::multithreaded_test(int number_of_threads) {
    thread t([] (test* pto) mutable {
        int my_number;
        my_number = pto->get_my_number();  // each get a unique number, this must be mutexed
        pto->testHelper( my_number );
    },
    this
            );

    // If this isn't the nth thread, keep calling more.
    if(number_of_threads > 1) {
        // recursively call self with -1
        multithreaded_test( (number_of_threads - 1) );
    }

    t.join();

    // end thread stuff
}



//*******************************************************
//*******************************************************
//********                                      *********
//********				Test Helper	  		    *********
//********                                      *********
//*******************************************************
//*******************************************************
// This function is set up so it can run more than one
// test.  Right now it just runs 1.

int test::testHelper(int my_number) {
    //  debug
    //	cout <<"Starting testHelper\n";

    //******************************************
    //********                         *********
    //********     	Various Tests      *********
    //******************************************

    // Each thread does these tests.

    //*********************************
    //********     Test       *********
    //*********************************

    test::TestFuctionF(my_number);
    //	test::TestFuctionH(my_number);

    return 0;

}


//*********************************
//********     Test       *********
//*********************************
/*
Program Test Sequence

The test sequence described below is approximately the 10th significant iteration of the test sequence.  I kept rewriting more robust versions that would test the ADT more exhaustively and create a lot of contention.

To test this ADT I created a separate class which runs a program test sequence.  The test sequence was written to be exhaustive (many inserts, deletes and traversals) and to create a lot of contention.  The test program runs 40 concurrent threads.  Each thread makes about 1,000 insertions and 400 deletions for a total of about 40,000 insertions and about 16,000 deletions.  The test sequence is as follows:

Each thread makes 500 insertions.  It does this 10 nodes at a time, and then moves the iterator back to the beginning of the list.

Each thread then performs the following sequence 100 times.

1.	Move the iterator to the begining of the list.
2.	Iterate to the 25th cell (note iterating skips auxiliary cells).
3.	Delete 2 nodes
4.	Iterate 2 nodes forward
5.	Insert 3 cells
6.	Move the iterator to the beginning of the list.
7.	Iterate to the 25th real node.
8.	Insert 3 nodes.
9.	Iterate forward 2 nodes.
10.	Delete 2 nodes.

Each of the 40 threads operates concurrently.
The general idea is that the deletions would cause the inserting iterators to "fall" back to its position.  This would then make them perform operations on the same cell or on directly adjacent cells, creating contention.

The test program tracks various statistics to verify the results.

The contention created by the numerous insertions and deletions causes some of the insertions and deletions fail.  This is the intended behavior of the ADT.

When an insertion or deletion fails, it returns the value of false.  It returns a value of true when they are successful.

Each thread keeps track of how many insertions and deletions it makes.  It also keeps track of how many of these fail.  Each thread then calculates a net number of additions to the list (i.e. successful insertions - successful deletions).

Each thread then adds these figures to the (net) total number added to the test data structure (using a synchronization object to prevent race conditions on the value).  That number is shown below as the "Sum of threads net additions to the list."


  */
int test::TestFuctionF(int my_number)  {
    int total_inserts  = 0;
    int total_deletes  = 0;
    int failed_inserts = 0;
    int failed_deletes = 0;
    // Here we will insert only our number.
    ListItr i; // ListItr is typedef

    // repeat 5 X
    for (int value1 = 0; value1 < 5 ; value1 ++)  {

        // Repeat 10 times.
        // Insert 10 and go back to beginning.
        for (int value2 = 0; value2 < 10 ; value2 ++)  {
            // Go back to the beginning of the list.
            i	=	(test::testLockFreeList1).begin();

            // Insert 10
            for (int value3 = 1; value3 < 10 ; value3 ++) {

                if (test::testLockFreeList1.insert( i, my_number ) ) {
                    total_inserts++;
                } else {
                    failed_inserts++;
                }
            }
        } // done inserting 10
    } // end of repeat 5 X
    // inserted a total of 500

    // repeat 100 X
    for (int value4 = 0; value4 < 100 ; value4 ++)  {

        // iterate to 25th spot
        i	=	(test::testLockFreeList1).begin();
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        // Delete 2 and increment the right counter.
        for (int value5 = 0; value5 < 2 ; value5 ++)  {
            // delete
            if ( test::testLockFreeList1.erase(i) ) {
                total_deletes++;
            } else {
                failed_deletes++;
            }

        } // end delete 2

        // forward 2 spots
        i++;
        i++;
        // Insert 3 and increment the right counter.
        for (int value6 = 0; value6 < 3 ; value6 ++)  {
            // insert and plus 2 spots
            if (test::testLockFreeList1.insert( i, my_number ) ) {
                total_inserts++;
            } else {
                failed_inserts++;
            }
        } // end insert 3

        // iterate to 25th spot
        i	=	(test::testLockFreeList1).begin();
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        i++;
        // Insert 3 and increment the right counter.
        for (int value7 = 0; value7 < 3 ; value7 ++)  {
            // insert and plus 2 spots
            if (test::testLockFreeList1.insert( i, my_number ) ) {
                total_inserts++;
            } else {
                failed_inserts++;
            }
            // forward 2 spots
            i++;
            i++;
        } // end insert 3

        // forward 2 spots
        i++;
        i++;

        // delete 2 and increment the right counter.
        for (int value8 = 0; value8 < 2 ; value8 ++)  {
            // delete
            if ( test::testLockFreeList1.erase(i) ) {
                total_deletes++;
            } else {
                failed_deletes++;
            }
            // forward 2 spots
            i++;
            i++;
        } // end repeat 5X




    } // end delete 2

    // Print out the stats.
    lock_guard<mutex> lock(m_sync_output); // Only let this thread output.
    cout<< "Thread "<<my_number<<":\n     total inserts = "<<total_inserts
        <<":\n     total deletes = "<<total_deletes<<":\n     failed inserts = "
        <<	failed_inserts <<":\n     failed deletes = "<<failed_deletes <<"\n";

    cout<< "I made a net of "<<(total_inserts - total_deletes)<<" additions to the list.\n\n";
    net_added_nodes += (total_inserts - total_deletes);
    return (total_inserts - total_deletes);
}






//*********************************
//********     Test       *********
//*********************************

// This calls the lock class test.
int test::integrityTest(int num_threads) {
    /*After all of the threads have quit, the program runs an integrity test on the list.  This is run in non-concurrent mode.  It adds up all the normal and auxiliary nodes in the list and reports the figures ("List internal add/delete counter: ListSize" and "total_aux_cells").*/
    test::testLockFreeList1.test_list2(num_threads);

    return 0;

}


//*********************************
//******** print test sum *********
//*********************************
// Prints stats compiled from all the threads.
int test::print_test_summary() {

    cout<< "Sum of threads net additions to the list : = "<<(net_added_nodes)<<"\n";

    return 0;

}
