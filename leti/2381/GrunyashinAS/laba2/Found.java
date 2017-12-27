package com.lockfreeset;

class Found<T> {
    Node<T> previousNode;
    Node<T> currentNode;
    Node<T> nextNode;

    Found(Node<T> previous, Node<T> current , Node<T> next) {
        this.setPrevious(previous);
        this.setCurrent(current);
        this.setNext(next);
    }

    public void setPrevious(Node<T> previous) {
        this.previousNode = previous;
    }

    public void setCurrent(Node<T> current) {
        this.currentNode = current;
    }

    public void setNext(Node<T> next) {
        this.nextNode = next;
    }

    public Node<T> getPrevious() {
        return this.previousNode;
    }

    public Node<T> getCurrent() {
        return this.currentNode;
    }

    public Node<T> getNext() {
        return this.nextNode;
    }
}