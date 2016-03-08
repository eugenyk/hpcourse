package chat.common;

@FunctionalInterface
public interface ICommand<T> {

    String execute(T arg);
}
