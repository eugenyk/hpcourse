package ru.hse.spb.hpcourse.set

sealed class AVLTree<K : Comparable<K>> {
    abstract val height: Int
    abstract val balance: Int
    abstract val isEmpty: Boolean

    abstract fun contains(key: K): Boolean
    abstract fun add(key: K): AVLTree<K>
    abstract fun remove(key: K): AVLTree<K>

    abstract fun max(): K

    abstract fun rebalance(): AVLTree<K>
}

data class AVLTreeUpdateResult<K : Comparable<K>>(val tree: AVLTree<K>, val returnFlag: Boolean)

class AVLNode<K : Comparable<K>>(val left: AVLTree<K>, val nodeKey: K, val right: AVLTree<K>) : AVLTree<K>() {
    override val height: Int = Math.max(left.height, right.height) + 1
    override val balance: Int = left.height - right.height
    override val isEmpty: Boolean = false

    override fun contains(key: K): Boolean {
        val comparisonResult = key.compareTo(nodeKey)

        return when {
            comparisonResult < 0 -> left.contains(key)
            comparisonResult == 0 -> true
            comparisonResult > 0 -> right.contains(key)
            else -> throw IllegalStateException()
        }
    }

    override fun add(key: K): AVLTree<K> {
        val comparisonResult = key.compareTo(nodeKey)

        return when {
            comparisonResult < 0 -> AVLNode(left.add(key), nodeKey, right)
            comparisonResult == 0 -> this
            comparisonResult > 0 -> AVLNode(left, nodeKey, right.add(key))
            else -> throw IllegalStateException()
        }.rebalance()
    }

    override fun remove(key: K): AVLTree<K> {
        val comparisonResult = key.compareTo(nodeKey)

        return when {
            comparisonResult < 0 -> AVLNode(left.remove(key), nodeKey, right)
            comparisonResult == 0 -> when (left) {
                is AVLNil -> right
                is AVLNode -> {
                    val previousKey = left.max()
                    AVLNode(left.removeMax(), previousKey, right)
                }
            }
            comparisonResult > 0 -> AVLNode(left, nodeKey, right.remove(key))
            else -> throw IllegalStateException()
        }.rebalance()
    }

    override fun max(): K {
        if (right is AVLNil)
            return nodeKey
        return right.max()
    }

    override fun rebalance(): AVLTree<K> = when(balance) {
        -2 -> if (right.balance <= 0) this.rotateLeft() else this.rotateRightLeft()
        2 -> if (left.balance >= 0) this.rotateRight() else this.rotateLeftRight()
        in -1..1 -> this
        else -> throw IllegalStateException("AVL tree is not balanced properly")
    }

    private fun rotateLeft(): AVLTree<K> {
        check(right is AVLNode)
        return AVLNode(AVLNode(left, nodeKey, right.left), right.nodeKey, right.right)
    }

    private fun rotateRight(): AVLTree<K> {
        check(left is AVLNode)
        return AVLNode(left.left, left.nodeKey, AVLNode(left.right, nodeKey, right))
    }

    private fun rotateRightLeft(): AVLTree<K> {
        check(right is AVLNode)
        return AVLNode(left, nodeKey, right.rotateRight()).rotateLeft()
    }

    private fun rotateLeftRight(): AVLTree<K> {
        check(left is AVLNode)
        return AVLNode(left.rotateLeft(), nodeKey, right).rotateRight()
    }

    private fun removeMax(): AVLTree<K> = when(right) {
        is AVLNil -> left
        is AVLNode -> AVLNode(left, nodeKey, right.removeMax())
    }
}

class AVLNil<K : Comparable<K>> : AVLTree<K>() {
    override val height = 0
    override val balance = 0
    override val isEmpty: Boolean = true

    override fun contains(key: K): Boolean = false
    override fun add(key: K): AVLTree<K> = AVLNode(AVLNil(), key, AVLNil())
    override fun remove(key: K): AVLTree<K> = this

    override fun max(): K = throw NoSuchElementException("Max is undefined when tree is empty")

    override fun rebalance(): AVLTree<K> = this
}