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
//		main.cpp	This file contains the main function.
//					The main function creates a the list test object and
//					runs the multithreaded test and integrity test.

#include <iostream>

#include "test.h"

using namespace std;



//*******************************************************
//*******************************************************
//********                                      *********
//********                 Main                 *********
//********                                      *********
//*******************************************************
//*******************************************************
// Program start point - main function.
// Accept and test user arguments, do a little user i/o,
// instantiate and run traverse object.
int main() {

    // Instantiate test object and pass the arguments to it.
    int number_of_threads = 40;
    test Lock_Free_List_Handle;
    // Call the thread helper member function.
    Lock_Free_List_Handle.multithreaded_test(number_of_threads);
    // Print out the test summary results.
    Lock_Free_List_Handle.print_test_summary();
    /*
    After all of the threads have quit, the program runs an integrity test on the list.  This is run in non-concurrent mode.  It adds up all the normal and auxiliary nodes in the list and reports the figures ("List internal add/delete counter: ListSize" and "total_aux_cells").
    */
    // Looks at the resulting list after the threads stop.
    Lock_Free_List_Handle.integrityTest(number_of_threads);
    return 0; // done
}

