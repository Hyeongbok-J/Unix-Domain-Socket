# Unix Domain Socket

A Unix domain socket or IPC socket (inter-process communication socket) is a data communications endpoint for exchanging data between processes executing on the same host operating system.

## Build and Run
Windows
```
Visual Studio 2017
```

Linux
```
$g++ -o UnixSocket main.cpp UnixSocket.cpp UnixSocket.h -pthread
$./UnixSocket

command [listen] -> server
command [open] -> client
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
