/*
* Name				Dmitrii Shakshin, Igor Pechko
* Student group		0303
* Email				d.shakshin@gmail.com
* Project	Implement a Lock-Free Linked List (based on Valois' paper)
*
*
* Generic Lock Free Linked List
*
* Contains three classes:
*
*  1. LockFreeList<ListType>
*     implements a generic lock free linked list
*	   (also conatins node sub-structure)
*
*  2. LockFreeList<ListType>::iterator
*     implements a linked list iterator for LockFreeList
*
*  3. LockFreeList<ListType>::list_node
*     implements a linked list node for LockFreeList
*
*/
//////////////////////////////////////////////////////////////////////////
//
//		File					Purpose
//		----					-------
/*
		lockfreelist.h			Declares and implements a Generic Lock Free Linked List class.

								This file contains three classes:

								  1. LockFreeList<ListType>
								     Implements a singly generic lock free linked list.
									 It also conatins the list_node and iterator sub-structures.

									 The LockFreeList class stores objects in a linked list.

									 The Storage sturcture heirarchy of an object of type
									 ListType looks like this:

										 LockFreeList(basenode) -> list_node(data) -> ListType

									 A LockFreeList object contains a pointer(basenode) to a
									 list of nodes, that are linked together.
									 Each node contains a pointer (data) that points to an object.

									 This class allows concurrent insertions and deletions into the list.
									 The inserting function does not need to provide mutual exclustion.
									 The calling functions can insert, delete and traverse without being
									 concerned with synchronization.  The implementation is "lock free."

									Important functions:

									 begin()
									 erase()
									 delete_node()
									 insert()
									 update_iterator()


								  2. LockFreeList<ListType>::iterator
								     Implements a linked list iterator for LockFreeList.
									 (This is a sub-class of LockFreeList.)

									Important functions:

									  update_iterator()
									  test_iterator()

									Overloaded Operators:

										++	prefix
										++  postfix
										=
										==
										!=
										->
										*


								  3. LockFreeList<ListType>::list_node
								     implements a linked list node for LockFreeList
									 (This is a sub-class of LockFreeList.)

									Important functions:

									 is_valid()
									 is_aux_cell()
									 is_normal_cell()
									 is_basenode()
									 is_lastnode()
									 is_not_basenode()
									 is_not_lastnode()
									 set_is_basenode()
									 set_is_lastnode()
									 compare_and_swap_next()

									This class contains the CriticalSection object "m_sync_compare_and_swap"
									for making the compare and swap function atomic.



*/

#ifndef LockFreeList9878765_H
#define LockFreeList9878765_H

#include <stdio.h>
#include <mutex>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define OS_WIN
#endif

#ifdef OS_WIN
#include "mingw.mutex.h"
#endif

using std::mutex;
using std::lock_guard;
using std::cout;
// *************************************
// *************************************
// ***                               ***
// ***        LockFreeList  class    ***
// ***                               ***
// *************************************
// *************************************
// Implements lock free list
// list is thread safe and more or less non blocking.
// Allows multiple concurent access.
template <class ListType>
class LockFreeList {
private:
    // *************************************
    // *************************************
    // ***                               ***
    // ***      List Node Structure      ***
    // ***                               ***
    // *************************************
    // *************************************
    // This structure implements the individual list nodes
    // that are inserted into the list.
    struct list_node {
    public:
        // ********************************************
        // *****    constructors / destructors    *****
        // ********************************************

        // ********************************************
        // *****   aux node (default constructor)  *****
        // ********************************************
        // Initallizes the node member data.
        // This one creates an aux node.
        // It has no data.
        list_node() {

            // Set data to Null to allow checking for aux node.
            data		= NULL;

            // Set next and previous to null.
            // Privious is null except for backtrack deletion mem management.
            // Next will be set later.
            next		= NULL;
            previous	= NULL;

            // These are initally false, and are reset later if appropriate.
            is_basenode_bool = false;
            is_lastnode_bool = false;

        }  // default constructor (aux node)

        // ********************************************
        // *****      real node (constructor)      *****
        // ********************************************
        // Initializes the node member data.
        // This one creates a real node.
        // It has data.
        list_node(const ListType & item) {

            // Set data to Null to allow checking for aux node.
            data		= new ListType(item);

            // Set next and previous to null.
            // Privious is null except for backtrack deletion mem management.
            // Next will be set later.
            next		= NULL;
            previous	= NULL;

            // These are initally false, and are reset later if appropriate.
            is_basenode_bool = false;
            is_lastnode_bool = false;

        }   // constructor with parameter

        // **************************
        // *****   destructor)  *****
        // **************************
        // Placeholder for future expanded cleanup functionallity.
        ~list_node() {
            // Due to the list implementation, a lot of
            // the pointers are lost.  Therefore, we really
            // can't clean up everything.

        }	// destructor


        // ******************************
        // *****  node member data  *****
        // ******************************
        // member data

    private:

        // syncro
        // This is used to protect the compar and swap operation.
        mutex m_sync_compare_and_swap;

        // These help us check for node status.
        bool is_basenode_bool;
        bool is_lastnode_bool;

    public:

        // Data is of type ListType (templated ).
        // It is the contents of that list node.
        ListType *		data;

        // These are pointers to other nodes (the link in linked list).
        list_node *	next, * previous;

        // ***********************************
        // *****  node member functions  *****
        // ***********************************
        // member data
    public:
        // **********   node status member functions   *********
        // These member functions check if the node status.
        // In other words what kind of node is it.
        // Is it a :
        //		 real node
        //		 aux node
        //		 base node
        //		 last node
        //		 not base node
        //		 not last node
        bool is_valid()		{
            return ( data != NULL );
        }

        bool is_aux_cell()		{
            return ( (data == NULL) && is_not_basenode() && is_not_lastnode() );
        }
        bool is_normal_cell()	{
            return ( data != NULL );
        }

        bool is_basenode()		{
            return ( is_basenode_bool );
        }
        bool is_lastnode()		{
            return ( is_lastnode_bool );
        }
        bool is_not_basenode() {
            return ( is_basenode_bool == false );
        }
        bool is_not_lastnode() {
            return ( is_lastnode_bool == false );
        }

        // **********   set node status member functions   *********
        // These member functions set the node status.
        bool set_is_basenode() {
            is_basenode_bool = true;
            return true;
        }
        bool set_is_lastnode() {
            is_lastnode_bool = true;
            return true;
        }


        // ******************************************
        // *****    p.compare_and_swap_next     *****
        // ******************************************
        // This function atomically checks if the next pointer is
        // the same as s (what the caller thinks it is ) and
        // updates it to n if it is.
        // This is a simulated compare and swap because it uses
        // the windows CRITICALSECTION object rather than a
        // hardware level atomic unit.
        bool compare_and_swap_next( 	list_node * s, 	list_node * n ) {
            // compare and swap
            // Syncronize the compare and swap functon.
            lock_guard<mutex> lock(m_sync_compare_and_swap);
            // compare
            if ( next == s ) {
                // and swap
                next = n;
                return true; // comfirm success
            } else {
                // debug code:  cout<<"c&s failed\n";
                return false; // return unsuccessful status.
            }
            // end compare and swap
        }

    };
    //*********************************************************************************
    //*********************************************************************************
    //************             lock free list class members              **************
    //*********************************************************************************
    //*********************************************************************************

    // 2 anchor nodes
    list_node * basenode; // basenode == firstnode
    list_node * lastnode; // new code
    // lock protect_ListSize

    int ListSize;  // List must mutex this too. (list will own this lock)
    mutex m_sync_ListSize;

    // Lock counter and increment the list size.
    bool AddToListSize(int adder) {
        lock_guard<mutex> lock(m_sync_ListSize);
        ListSize += adder;
        return true;
    }

public:
    // Default constructor.
    // instantiates linked list
    LockFreeList() {

        //  Starts as:
        //  basenode -> aux_node -> lastnode -> null

        basenode = new list_node();
        basenode->set_is_basenode();
        lastnode = new list_node();
        lastnode->set_is_lastnode();
        basenode->next = new list_node(); // aux_nodes use default constructor (no data initilization)

        basenode->next->next = lastnode;

        lastnode->next = NULL;

        ListSize=0;
    }   // create empty list


    // destructor
    ~LockFreeList() {

        if (basenode != NULL) {
            delete basenode;
        }
        // Debug Code:  cout<<"cleared";

    }

    // *****      get functions     *****
    list_node get_lastnode() {
        return lastnode;
    }


    int getListSize() {
        return ListSize;
    }

    friend class iterator;

    // *************************************
    // *************************************
    // ***                               ***
    // ***        iterator class         ***
    // ***                               ***
    // *************************************
    // *************************************
    //    Implements a linked list iterator for LockFreeList.
    //    (This is a sub-class of LockFreeList.)

    class iterator {
    protected:
        list_node * target;	// points to the real node we are at.
        list_node * pre_aux;	// points to the aux node before the real node.
        list_node * pre_cell;	// points to the real node before the
        // real node we are at.

    public:
        // This declaration allows the list class to access
        // the private data and member functions of the iterator class.
        friend class LockFreeList<ListType>;

        // *************************************
        // ***      iterator constructor     ***
        // *************************************
        // Initalizes the pointers to NULL.
        // These will be set later by other for sophisticated functions.
        iterator() {
            target		= NULL;
            pre_aux	= NULL;
            pre_cell	= NULL;
        }

        //**************************************************
        //******                                      ******
        //******          Iterator Functions          ******
        //******                                      ******
        //**************************************************

        // *************************************
        // ***                               ***
        // ***       update_iterator()       ***
        // ***                               ***
        // *************************************
        // This is a helper function for the ++ operator.
        // It moves the iterator forward and updates it in case something
        // changed or another process interfered.
        bool go_next() {

            if(target->next == NULL) {
                return false;    // substitute for testing for lastnode
            }

            pre_cell = saferead(target); // Sets pre cell to point to target.

            pre_aux = saferead(target->next); // Sets pre aux to target next.

            update_iterator(); // We never directly set target.  We let update find it for us.

            return true;
        }

        // *************************************
        // ***                               ***
        // ***          safe read ()         ***
        // ***                               ***
        // *************************************
        // Ensures that a process does not copy a pointer that subsequently becomes stale.
        // i.e make sure that first->next does not change between read the ptr.
        // Safe read :	reads the passed in node pointer,
        //				checks if it is null
        //				and double checks that it is the right node
        //				then passes it back
        list_node * saferead(list_node * node_ptr) { // Page 61 Valois

            while(1) {

                list_node * q = node_ptr; //read(node_ptr);
                if (q == NULL) {
                    return NULL;
                    // debug code:  cout<<"saferead returned NULL\n";
                }

                if (q == node_ptr) { // read(p) {
                    // Debug Code:  cout<<"saferead returns node_ptr\n";
                    return q;
                } else {

                }
            }
        }

        // *************************************
        // ***                               ***
        // ***         test_iterator()       ***
        // ***                               ***
        // *************************************
        // debug code do test iterator validity
        void test_iterator() {
            // Debug Code:  cout<<"test itr\n";
            if ( pre_aux->next == target ) {
                cout<<"pre_aux->next == target\n";
                return;
            }

        }

        // *************************************
        // ***                               ***
        // ***       update_iterator()       ***
        // ***                               ***
        // *************************************
        // This function updates the pointers in the iterator.
        // This is pre_aux relative.
        void update_iterator() { // update the iterator
            // find the target
            // Debug Code:  cout<<"updating iterator\n";

            // if the pre_aux = target then fine and dandy
            if ( pre_aux->next == target ) {
                return;
            }

            // Debug Code:  cout<<"updating - preaux->next not == target\n";
            // Set a couple temp pointers.
            list_node * p = pre_aux;
            list_node * n = p->next;

            // Move p and n forward til they are set to an aux node
            // and real node respectively.
            // Stop if we reach the end of the list, then target
            // will end up being the last node.
            while (  ( n->is_not_lastnode() ) && n->is_aux_cell()  )  {
                // Debug Code:  cout<<"in update while loop\n";

                // Atomically makes the list skip extra aux nodes.
                pre_cell->compare_and_swap_next( p, n );
                p = n;
                n = saferead(p->next);
            }

            // Set real pre aux to the temp node pointer.
            pre_aux = p;
            // Set the target to the temp node pointer.
            target = n;

            // Debug code:
            if(target->is_aux_cell()) {
                // Debug Code:  cout<<"Target is Aux Node\n";
            }
            // Debug Code:  cout<<"updating end\n";
        }


        //*************************************
        //******                         ******
        //******  Overloading Operators  ******
        //******                         ******
        //*************************************
        // Like any other c++ ADT, we need to overload many operators.

        //*************************************
        //******       = Operator        ******
        //*************************************
        // Overloaded = operator
        // Takes an iterator as an argument,
        // sets this (aka left hand side) equal to rhs
        // and returns this.

        // This is used in conjunction with the begin function or
        // the ++ pre fix operator.  This assignment operator
        // passes the iterator to the iterator on the left
        // of the = sign that we are setting to this one.

        const iterator & operator=(const iterator & rhs )
        {
            // set left to right
            target		= rhs.target;
            pre_aux	= rhs.pre_aux;
            pre_cell	= rhs.pre_cell;

            // return pointer to this structure.
            return *this;
        }

        // **************************************
        // ****    Overloaded ++ operator    ****
        // **************************************
        // post fix increment operator
        // ++ and post fix must take a dummy int parameter
        // the parameter should never be used
        // this fn is a unary operator
        // it takes the iterator, increments it and returns an
        // pre-incremented iterator.
        iterator operator++(int n) {
            iterator temp;
            temp = *this; // store pre increment value to return.
            go_next();	   // Helper funtion.
            return temp;
        }

        // **************************************
        // ****    Overloaded ++ operator    ****
        // **************************************
        // Moves formard on real node.
        // pre fix increment operator
        // this fn is a unary operator
        // It takes the iterator, increments it and returns it.
        iterator &operator++()
        {
            go_next();	// ++ helper function
            return *this;
        }

        // **************************************
        // ****    Overloaded == operator    ****
        // **************************************
        // Takes an iterator as an argument,
        // Determines if the data in this == data in rhs
        // and returns a bool for true or false
        bool operator==(const iterator & rhs) const
        {
            return this->data == rhs.nodepointer->data;
        }

        // **************************************
        // ****    Overloaded != operator    ****
        // **************************************
        // Takes an iterator as an argument,
        // Determines if the data in this != data in rhs
        // and returns a bool for true or false
        bool operator!=(const iterator & rhs) const
        {
            return target->data != rhs.target->data;
        }

        // **************************************
        // ****    Overloaded * operator     ****
        // **************************************
        // Takes an iterator as an argument,
        // and returns its data contents.
        ListType & operator*() 	 {
            return *(target->data);
        }
        // **************************************
        // ****    Overloaded -> operator    ****
        // **************************************
        // Takes an iterator as an argument,
        // and returns the real node it points to.
        list_node * & operator->() 	 {
            return (target);
        }

    };  // END iterator class
    //  ****************************************
    //  ****************************************
    //  ****************************************


    // **************************************
    // **************************************
    // **   LockFreeList class functions   **
    // **************************************
    // **************************************

    //****************************
    //***    begin function    ***
    //****************************
    // This function sets the iterator to the first real node in the list.
    // If there are no real nodes in the list then it points to the last node.
    iterator begin() const {
        iterator *b = new iterator;
        //first(b);

        // first
        if (b->target != NULL) {

        }

        b->pre_cell = b->saferead(basenode);

        b->pre_aux  = b->saferead(basenode->next);

        b->target = NULL;
        b->update_iterator();

        // end first
        return *b;
        //	return list_iterator<ListType>(basenode->next);
    }

    // *************************************
    // ***           insert              ***
    // *************************************

    // The insert function is used to insert an object into the linked list.

    // It accepts an iterator and an object as parameters.
    // It creates a new node, a new copy of the passed object,
    // and an auxiliary node.  It then points the new node at the
    // auxiliary node and points the auxiliary node at the node
    // that the iterator is currently pointing at.
    // It than swings the pointer from the pre_aux node (the auxiliary
    // node right before the current node) that the iterator is also
    // pointing at so that it points at the new node.
    // Then the insert is complete.
    //
    // This requires additional manipulations ( using update_iterator() )
    // and atomic syncronization ( using the compare_and_swap() function )
    // to maintain the integrity of the list sequence.

    bool insert(iterator i, const ListType & data) {

        i.update_iterator();

        list_node * q = new list_node(data);// the real node
        list_node * a = new list_node(); // aux_node
        q->next = a;
        a->next = i.target;
        bool successful = i.pre_aux->compare_and_swap_next(i.target, q); // compare_and_swap( pre_aux, target, q );
        i.update_iterator();
        if(successful) {
            AddToListSize(1);
        }
        return successful;
    }
    // *************************************
    // ***          delete node          ***
    // *************************************
    // Help erase to delete the node.
    // This function actuall deletes the node.
    bool delete_node(iterator i) {

        list_node * d = i.target;
        list_node * n = i.target->next;
        bool r = i.pre_aux->compare_and_swap_next( d, n ); // swing next pointer
        // actual delete is done
        if(r != true) {
            return false;   // if unsuccessful say so
        }
        //----------------------------------------------------
        // Decrement list size and return.
        if (r == true) {
            AddToListSize(-1);
            return true;
        }

        // We shouldn't get here, but return true just in case.
        return true;
    }

    // *************************************
    // ***            erase              ***
    // *************************************
    // This function takes a node out of the list,
    // and free's its aux_node.
    bool erase(iterator & i) {
        if (i->is_not_lastnode()) // fail if we try to delete the basenode
        {
            //for(int trys = 0;trys < 10;trys++) { // I took this out to encourage failure.

            // We now just try once.
            for(int trys = 0; trys < 1; trys++) {
                // Debug Code:  cout<<"Delete - try "<<trys<<"\n";

                i.update_iterator(); // make the iterator pointers be correct.
                if(delete_node(i)) { // if delete is successful
                    // Now we update the iterator pointers which also
                    // get rid of the extra aux node.
                    i.update_iterator();

                    return true;
                }
                // if delete is un-successful
                // then try some more times.
                // note we don't try again to encourage failing under contention.
                i.update_iterator(); // again make iterators pointers be correct.
                // (they should be right, but if erase failed then they probably aren't)
            }

            return false;
        }
        return false;
    }

    // *************************************
    // ***                               ***
    // ***       update_iterator()       ***
    // ***                               ***
    // *************************************
    // This function updates the pointers in the iterator.
    // This is pre_aux relative.
    void update_iterator(iterator * itr) { // update the iterator
        // find the target

        // if the pre_aux = target then fine and dandy
        if ( itr->pre_aux->next == itr->target ) {
            return;
        }

        // Set a couple temp pointers.
        list_node * p = itr->pre_aux;
        list_node * n = saferead(p->next);

        // Move p and n forward til they are set to an aux node
        // and real node respectively.
        // Stop if we reach the end of the list, then target
        // will end up being the last node.
        while (  (n != lastnode) && n->is_aux_cell()  )  {

            // Atomically makes the list skip extra aux nodes.
            itr->pre_cell->compare_and_swap_next( p, n );

            p = n;
            n = saferead(p->next);
        }

        // Set real pre aux to the temp node pointer.
        itr->pre_aux = p;
        // Set the target to the temp node pointer.
        itr->target = n;

    } // end update iterator

    // *************************************
    // ***           test list           ***
    // *************************************
    // This function just counts the nodes in the list.
    // This can be only run in single threaded mode.
    // it is just for testing purposes.
    void test_list() {
        int count = 0;
        list_node * testnode = basenode;
        while(testnode) {
            cout<<"node "<<count<<"\n";
            count++;
            testnode = testnode->next;
        }
    }

    // *************************************
    // ***          test list 2          ***
    // *************************************

    void test_list2(int number_of_unique_values) {
        // This is the integrity test.
        // It is called by test::integrity_Iest
        // It should only be run with a single thread (non-concurrent).

        // It is only for testing purposes,
        //	it is not part of the actual ADT.

        // It is designed around the int data type.
        // Therefore, if you intend to run it on another data type
        // the type should be fully overloaded and this fn
        // should be slightly modified.
        /*
        After all of the threads have quit, the program runs an
        integrity test on the list.  This is run in non-concurrent mode.
        It adds up all the normal and auxiliary nodes in the list and
        reports the figures ("List internal add/delete counter: ListSize"
        and "total_aux_cells").*/
        cout<<"Integrity test of list :";

        // Array to store the value counts (max 1000 threads).
        int value_count[1000];

        // Initialize array values to 0.
        for(int i = 0; i<number_of_unique_values+1; i++) {
            value_count[i]=0;
        }

        // Declare and initialize some variables.
        int count = 0;
        int total_normal_cells = 0;
        int total_aux_cells = 0;
        list_node * testnode = basenode;

        // While we are not past the end of the list.
        while(testnode) {

            //debug code : cout<<"node "<<count<<" ";

            // If normal cell.
            if(testnode->is_valid() &&(testnode->is_normal_cell()) ) {
                //debug code : cout<<"is a normal cell and its value is "<<*(testnode->data)<<"\n";

                // Increment normal cell count.
                total_normal_cells++;

                // Increment the value count array.
                value_count[*(testnode->data)]=value_count[*(testnode->data)]+1;
            }

            // Test if aux cell.
            if(testnode->is_aux_cell()) {
                //debug code : cout<<"is an auxiliary cell.  \n";

                // Increment aux cell count.
                total_aux_cells++;
            }

            // Debug code:
            // Test if base node.
            if(testnode->is_basenode() ) {
                //debug code : cout<<"is the basenode.  \n";
            }
            // Test if last node.
            if(testnode->is_lastnode()) {
                //debug code : cout<<"is the lastnode.  \n";
            }

            // Increment total count.
            count++;
            // Go next (non-concurrent version)
            testnode = testnode->next;
        }

        // Report stats.
        cout<<"\nIntegrity test : total_normal_cells        = "<<total_normal_cells<<"\n";
        cout<<  "List internal add/delete counter: ListSize = "<<ListSize<<"\n";
        cout<<"\ntotal_aux_cells                            = "<<total_aux_cells<<"\n";

        // Report value counts.
        for(int i = 1 ; i < number_of_unique_values + 1 ; i++ ) {
            cout<<"\nThere are "<<value_count[i]<<" nodes with the value "<<i<<".";
        }

        // Formatting
        cout<<"\n\n****************************************\n\n";
        // New test.
        count = 0;
        // print the first 1000
        testnode = basenode;
        int upTo1000 = 0;

        // This loop reports the status of each node.
        // It actually only prints the first 1,000.
        // Because 30000 is too many to read and store.

        // It reports if the node is a :
        //				real node,
        //				aux node,
        //				base node or
        //				last node.

        // It also reports the value of the node if it is a real node.
        while(testnode) {

            cout<<"node "<<count<<" ";
            // Test and output node status.
            if(testnode->is_valid() &&(testnode->is_normal_cell()) ) {
                cout<<"is a normal cell and its value is "<<*(testnode->data)<<"\n";
                total_normal_cells++;

                value_count[*(testnode->data)]=value_count[*(testnode->data)]+1;
            }
            // Test and output node status.
            if(testnode->is_aux_cell()) {
                cout<<"is an auxiliary cell.  \n";
                total_aux_cells++;
            }
            if(testnode->is_basenode() ) {
                cout<<"is the basenode.  \n";
            }
            if(testnode->is_lastnode()) {
                cout<<"is the lastnode.  \n";
            }

            // increment counter

            count++;
            // go next
            testnode = testnode->next;

            // increment first 1000 counter
            upTo1000++;

            // Break if 1001
            if(upTo1000 > 1000) {
                break;
            }
        }
    }
};

#endif
