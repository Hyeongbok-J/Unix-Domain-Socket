# Unix Domain Socket
echo server-client using unix domain socket.

## Build and Run
Windows
```
Visual Studio 2017
```

Linux
```
$ g++ -o UnixSocket main.cpp UnixSocket.cpp UnixSocket.h -pthread
$ ./UnixSocket
```

## Usage
command
```
[listen] -> wait for client
[open] -> connect to server
[send] -> send message to server and receive it from server
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
