# 42_scholl_web_server
This project is about writing your ow HTTP server.

### What is a Socket?
---

A way to speak to other programs using standard Unix file descriptors.  
Ok—you may have heard some Unix hacker state, “Jeez, everything in Unix is a
file!” What that person may have been talking about is the fact that when Unix
programs do any sort of I/O, they do it by reading or writing to a file
descriptor. A file descriptor is simply an integer associated with an open file.
But (and here’s the catch), that file can be a network connection, a FIFO, a
pipe, a terminal, a real on-the-disk file, or just about anything else.
Everything in Unix is a file!

### Two Types Of Internet Sockets
---

- SOCK_DGRAM

- SOCK_STREAM
	Stream sockets are reliable two-way connected communication streams. If you
	output two items into the socket in the order “1, 2”, they will arrive in
	the order “1, 2” at the opposite end. They will also be error-free. How do
	stream sockets achieve this high level of data transmission quality? They
	use a protocol called “The Transmission Control Protocol”, otherwise known
	as “TCP” (see RFC 7933 for extremely detailed info on TCP).

<br>

### Port
---

The port is simply a designator of one such message streamassociated with a process.  


#### Links

What is a Socket?
https://www.youtube.com/watch?v=Gr2ROxZXuvQ
https://www.youtube.com/watch?v=xBNBIDFP84A
TCP RFC793
	https://datatracker.ietf.org/doc/html/rfc793
A Protocol For Packet Network Intercommunication
	https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=1092259