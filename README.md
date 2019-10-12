# Example TCP echo server that uses `kqueue(2)` #
This is a bare minimum TCP echo server that used as a show case for how many connections/second a TCP server based on `kqueue` can handle.
 The code contains comments on what section of code does.


### Run ###
`make`

`./kqueue-tcp-server`

Now connect to the server at port 1815. For instance, using netcat:

`nc -v localhost 1815`

[kqueue_manpage_link]: https://www.freebsd.org/cgi/man.cgi?query=kqueue&apropos=0&sektion=2&manpath=FreeBSD+12.0-RELEASE+and+Ports&arch=default&format=html