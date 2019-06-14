package ru.hse.spb.hpcourse.set

import java.util.concurrent.atomic.AtomicReference

class LockFreeSet<K : Comparable<K>> : LockFreeSetInterface<K> {
    private val tree: AtomicReference<AVLTree<K>> = AtomicReference(AVLNil<K>())

    override fun isEmpty(): Boolean = tree.get().isEmpty

    override fun contains(value: K): Boolean = tree.get().contains(value)

    override fun add(value: K): Boolean = atomicUpdate { tree ->
        if (tree.contains(value))
            return@atomicUpdate tree
        return@atomicUpdate tree.add(value)
    }

    override fun remove(value: K): Boolean = atomicUpdate { tree ->
        if (!tree.contains(value))
            return@atomicUpdate tree
        return@atomicUpdate tree.remove(value)
    }

    override fun iterator(): Iterator<K> = AVLIterator(tree.get())

    private fun atomicUpdate(updateTree: (AVLTree<K>) -> AVLTree<K>): Boolean {
        while (true) {
            val oldTree = tree.get()
            val newTree = updateTree(oldTree)

            when {
                oldTree == newTree -> return false
                tree.compareAndSet(oldTree, newTree) -> return true
            }
        }
    }
}