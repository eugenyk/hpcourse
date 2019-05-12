package ru.hse.spb.hpcourse.set

class AVLIterator<K : Comparable<K>>(tree: AVLTree<K>): Iterator<K> {
    private val stack = mutableListOf<AVLNode<K>>()

    init {
        goToLeftestNode(tree)
    }

    override fun hasNext() = stack.isNotEmpty()

    override fun next(): K {
        val topNode = stack.last()
        stack.removeAt(stack.size - 1)
        goToLeftestNode(topNode.right)
        return topNode.nodeKey
    }

    private fun goToLeftestNode(tree: AVLTree<K>) {
        var node = tree
        while (node is AVLNode) {
            stack.add(node)
            node = node.left
        }
    }
}