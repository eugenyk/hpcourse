auchat server
=============

Для сборки потребуется [Rust](https://www.rust-lang.org/) и Linux (поддержку
Windows обещают в следующем релизе библиотеки
[mio](https://github.com/carllerche/mio), которая используется для асинхронного
IO).

* Сервер `cargo run --release --bin server -- --workers=4`
* Клиент `cargo run --release --bin client -- --login=Alice`
* Бенчмарк  `cargo run --release --bin bench -- --rps --packed --med --large --huge --c10k`


Структура кода
==============


`src/bin/*.rs` -- точки входа для клиента, сервера и бенчмарка.

`src/pb` -- фреймворк для написания асинхронных многопоточных protobuf серверов.

`src/chat.rs` -- чат, написаный с использованием фреймворка.
