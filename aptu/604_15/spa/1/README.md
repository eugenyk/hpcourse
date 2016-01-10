async-chat-server
=================

Для сборки потребуется [stack](https://github.com/commercialhaskell/stack).
Установка stack описана [тут](http://docs.haskellstack.org/en/stable/README.html).

* Сборка: `stack build`
* Запуск: `stack exec async-chat-server` в качестве параметра можно передать количество
воркеров, например: `stack exec async-chat-server 8`.
