package ru.hse.spb.hpcourse.set

import org.junit.Assert.*
import org.junit.Test

class AVLTreeTest {
    private fun generateTree(n: Int): AVLTree<Int> {
        var tree: AVLTree<Int> = AVLNil()

        for (i in 1..n)
            tree = tree.add(i)

        return tree
    }

    @Test
    fun testEmptyTreeConstructor() {
        AVLNil<Int>()
    }

    @Test
    fun testAdd() {
        AVLNil<Int>().add(5)
    }

    @Test
    fun testContains() {
        var tree: AVLTree<Int> = AVLNil()
        assertEquals(false, tree.contains(5))
        tree = tree.add(5)
        assertEquals(true, tree.contains(5))
        assertEquals(false, tree.contains(6))
    }

    @Test
    fun testAddManyElements() {
        generateTree(1000)
    }

    @Test
    fun testAddSame() {
        AVLNil<Int>().add(5).add(5)
    }

    @Test
    fun testTreeHeight() {
        val tree = generateTree(1000)
        print(tree.height)
        assertTrue(tree.height <= Math.log(1002.0) / 0.2)
    }

    @Test
    fun testIsEmpty() {
        var tree: AVLTree<Int> = AVLNil()
        assertEquals(true, tree.isEmpty)
        tree = tree.add(5)
        assertEquals(false, tree.isEmpty)
    }

    @Test
    fun testRemoveNothing() {
        val tree = AVLNil<Int>().add(5).remove(4)
        assertEquals(true, tree.contains(5))
        assertEquals(false, tree.contains(4))
    }

    @Test
    fun testRemove() {
        val tree = AVLNil<Int>().add(5).remove(5)
        assertEquals(false, tree.contains(5))
    }

    @Test(expected = NoSuchElementException::class)
    fun testMaxEmpty() {
        AVLNil<Int>().max()
    }

    @Test
    fun testMax() {
        assertEquals(10, generateTree(10).max())
    }
}