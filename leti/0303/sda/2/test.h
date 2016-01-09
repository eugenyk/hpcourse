/*
*
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
//		test.h:		Header file for the test class.
//					It declares the interface for the test class.
//
//
//
//
//******************************************************************************************
//******************************************************************************************
//******************************************************************************************
//*************                                                               **************
//*************                           Test Object                         **************
//*************                                                               **************
//*************                           Header File                         **************
//*************                                                               **************
//******************************************************************************************
//******************************************************************************************
//******************************************************************************************
//
//////////////////////////////////////////////////////////////////////

// If def to prevent multiple compilations.
#if !defined(AFX_test_H__A892AC06_6934_41DF_A784_5C8E45F6EEED__INCLUDED_)
#define AFX_test_H__A892AC06_6934_41DF_A784_5C8E45F6EEED__INCLUDED_

// include files
#include <stdio.h>
#include "lockFreeList.h"

#include <mutex>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define OS_WIN
#endif

#ifdef OS_WIN
#include "mingw.mutex.h"
#endif

// *************************************
// *************************************
// ***                               ***
// ***          Test Class           ***
// ***          Declaration          ***
// ***                               ***
// *************************************
// *************************************
class test {

private:
    // Initializes the class interface functions.
    int mv_my_number; // Gives each thread a unique number.
    int mv_number_of_threads;// Stores the number of threads.
    int net_added_nodes; // Sum of all threads.
    LockFreeList<int> testLockFreeList1; // The list object.
    //bool stop;// = 0; // should thread stop? 1=yes

private:
    // This points to a critical section (my construct)
    // It synchronizes my_number
    mutex m_sync_my_number;
    // This points to a critical section (my construct)
    // It synchronizes output to the screen
    mutex m_sync_output;

public:

    // Initializes constructors, destructors
    // and the class interface functions.
    test();
    virtual ~test();

    // Various testing function declarations.
    void multithreaded_test(int number_of_threads);
    int  testHelper(int my_number);
    int	 integrityTest(int num_threads);
    int  TestFuctionF(int my_number); // This is the one we use.
    int  print_test_summary();


    int  get_my_number(); // Sycronizes the giving of a unique thread number.
};

#endif // !defined(AFX_test_H__A892AC06_6934_41DF_A784_5C8E45F6EEED__INCLUDED_)
