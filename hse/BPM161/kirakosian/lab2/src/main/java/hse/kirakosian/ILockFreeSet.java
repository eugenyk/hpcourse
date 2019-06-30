package hse.kirakosian;


import java.util.Iterator;

/**
 * Lock-Free множество.
 * @param <T> Тип ключей
</T> */
public interface ILockFreeSet<T extends Comparable<T>> extends Iterable<T> {


        /**
         * Проверка множества на пустоту
         *
         * Алгоритм должен быть как минимум lock-free
         *
         * @return true если множество пусто, иначе - false
         */
        boolean isEmpty();

        /**
         * Добавить ключ к множеству
         *
         * Алгоритм должен быть как минимум lock-free
         *
         * @param value значение ключа
         * @return false если value уже существует в множестве, true если элемент был добавлен
         */
        boolean add(T value);


        /**
         * Удалить ключ из множества
         *
         * Алгоритм должен быть как минимум lock-free
         *
         * @param value значение ключа
         * @return false если ключ не был найден, true если ключ успешно удален
         */
        boolean remove(T value);


        /**
         * Проверка наличия ключа в множестве
         *
         * Алгоритм должен быть как минимум wait-free
         *
         * @param value значение ключа
         * @return true если элемент содержится в множестве, иначе - false
         */
         boolean contains(T value);

        /**
         * Возвращает lock-free итератор для множества
         *
         * @return новый экземпляр итератор для множества
         */
         @Override
         Iterator<T> iterator();
}