# multiplexing
I/O Multiplexing mechanism in Linux.

We implement a echo-server handles multiple I/O operations concurrently within a single process.

- server_select.c: is a server using select system call.
- server_poll.c: is a server using poll system call.
- server_epoll.c: is a server using epoll system call.
- client_benchmark.c: is a client that creates 75 threads. Each thread connect with the server and sends 100 messages "Hello Server" to server.

## Compiling

    gcc -o server_epoll server_epoll.c
    gcc -o server_poll server_poll.c
    gcc -o server_select server_select.c

## Execute

    // one tab executes server, for example, server_select
    ./server_select
    // another tab executes client to benchmark
    ./client_benchmark
    // after client has finished to send messages, we kill the server, then benchmark with server_poll, server_epoll similarly.

