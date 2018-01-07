package ru.spbau.mit;

class NodeWindow<T extends Comparable<T>> {
    private Node<T> previous;
    private Node<T> current;

    NodeWindow(Node<T> previous, Node<T> current) {
        this.previous = previous;
        this.current = current;
    }

    Node<T> getPrevious() {
        return previous;
    }

    void setPrevious(Node<T> newPrevious) {
        this.previous = newPrevious;
    }

    Node<T> getCurrent() {
        return current;
    }

    void setCurrent(Node<T> newCurrent) {
        this.current = newCurrent;
    }
}
