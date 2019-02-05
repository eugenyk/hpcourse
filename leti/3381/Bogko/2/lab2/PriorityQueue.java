package lab2; /**
 * Lock-Free ������� � ������������
 * @param <T> ��� ���������
 */

import java.util.Queue;


public interface PriorityQueue<E extends Comparable<E>> extends Queue<E> {

    /**
     * �������� ������� �� �������
     *
     * ����� ������ ���� lock-free (wait-free ��� ��������� � ����)
     *
     * @return true ���� ������� �����, ����� - false
     */
    boolean isEmpty();
}