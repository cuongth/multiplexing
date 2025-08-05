# multiplexing
I/O Multiplexing mechanism in Linux.

We implement a echo-server handles multiple I/O operations concurrently within a single process.

- server_select.c: is a server using select system call.
- server_poll.c: is a server using poll system call.
- server_epoll.c: is a server using epoll system call.
- client_benchmark.c: is a client that creates 80 threads. Each thread establishes a connection with the server and sends 100 messages "Hello Server" on that connection to server.

## Compiling

    gcc -o server_epoll server_epoll.c
    gcc -o server_poll server_poll.c
    gcc -o server_select server_select.c

## Execute

    // one tab executes server, for example, server_select
    ./server_select
    // another tab executes client to benchmark
    ./client_benchmark
    // after client has finished to send messages, we kill the server,
    // then benchmark with server_poll, server_epoll similarly.

## Benchmark

    <table>
      <tr>
        <td><b>amd ryzen 3 1200 quad-core</b></td>
        <td>Total time taken:</td>
      </tr>
      <tr>
        <td><b>select server</b></td>
        <td>14.522968 seconds</td>
      </tr>
      <tr>
        <td><b>poll server</b></td>
        <td>1.447111 seconds seconds</td>
      </tr>
      <tr>
        <td><b>epoll server</b></td>
        <td>1.224191 seconds</td>
      </tr>
    </table>


