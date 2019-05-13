package ru.hse.spb.hpcourse.set

import java.util.concurrent.atomic.AtomicReference

class LockFreeSet<K : Comparable<K>> : LockFreeSetInterface<K> {
    private val tree: AtomicReference<AVLTree<K>> = AtomicReference(AVLNil<K>())

    override fun isEmpty(): Boolean = tree.get().isEmpty

    override fun contains(value: K): Boolean = tree.get().contains(value)

    override fun add(value: K): Boolean = atomicUpdate { tree ->
            if (tree.contains(value))
                return@atomicUpdate AVLTreeUpdateResult(tree, false)
            return@atomicUpdate AVLTreeUpdateResult(tree.add(value), true)
        }

    override fun remove(value: K): Boolean = atomicUpdate { tree ->
        if (!tree.contains(value))
            return@atomicUpdate AVLTreeUpdateResult(tree, false)
        return@atomicUpdate AVLTreeUpdateResult(tree.remove(value), true)
    }

    override fun iterator(): Iterator<K> = AVLIterator(tree.get())

    private fun atomicUpdate(updateTree: (AVLTree<K>) -> AVLTreeUpdateResult<K>): Boolean {
        while (true) {
            val oldTree = tree.get()
            val result = updateTree(oldTree)
            val newTree = result.tree
            if (!result.returnFlag || tree.compareAndSet(oldTree, newTree))
                return result.returnFlag
        }
    }
}