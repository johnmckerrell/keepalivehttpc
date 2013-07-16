/*
** SOCKET.C
**
** Written by Steven Grimm (koreth@ebay.sun.com) on 11-26-87
** Please distribute widely, but leave my name here.
**
** Various black-box routines for socket manipulation, so you don't have to
** remember all the structure elements.
*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifndef FD_SET		/* for 4.2BSD */
#define FD_SETSIZE      (sizeof(fd_set) * 8)
#define FD_SET(n, p)    (((fd_set *) (p))->fds_bits[0] |= (1 << ((n) % 32)))
#define FD_CLR(n, p)    (((fd_set *) (p))->fds_bits[0] &= ~(1 << ((n) % 32)))
#define FD_ISSET(n, p)  (((fd_set *) (p))->fds_bits[0] & (1 << ((n) % 32)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif

extern int errno;

/*
** serversock()
**
** Creates an internet socket, binds it to an address, and prepares it for
** subsequent accept() calls by calling listen().
**
** Input: port number desired, or 0 for a random one
** Output: file descriptor of socket, or a negative error
*/
int serversock(port)
int port;
{
	int	sock, x;
	struct	sockaddr_in server;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return -errno;

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	x = bind(sock, &server, sizeof(server));
	if (x < 0)
	{
		close(sock);
		return -errno;
	}

	listen(sock, 5);

	return sock;
}

/*
** portnum()
**
** Returns the internet port number for a socket.
**
** Input: file descriptor of socket
** Output: inet port number
*/
int portnum(fd)
int fd;
{
	int	length, err;
	struct	sockaddr_in address;

	length = sizeof(address);
	err = getsockname(fd, &address, &length);
	if (err < 0)
		return -errno;

	return ntohs(address.sin_port);
}

/*
** clientsock()
**
** Returns a connected client socket.
**
** Input: host name and port number to connect to
** Output: file descriptor of CONNECTED socket, or a negative error (-9999
**         if the hostname was bad).
*/
int clientsock(host, port)
char *host;
int port;
{
	int	sock;
	struct	sockaddr_in server;
	struct	hostent *hp, *gethostbyname();

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (isdigit(host[0]))
		server.sin_addr.s_addr = inet_addr(host);
	else
	{
		hp = gethostbyname(host);
		if (hp == NULL)
			return -9999;
		bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return -errno;

	if (connect(sock, &server, sizeof(server)) < 0)
	{
		close(sock);
		return -errno;
	}

	return sock;
}

/*
** readable()
**
** Poll a socket for pending input.  Returns immediately.  This is a front-end
** to waitread() below.
**
** Input: file descriptor to poll
** Output: 1 if data is available for reading
*/
readable(fd)
int fd;
{
	return(waitread(fd, 0));
}

/*
** waitread()
**
** Wait for data on a file descriptor for a little while.
**
** Input: file descriptor to watch
**	  how long to wait, in seconds, before returning
** Output: 1 if data was available
**	   0 if the timer expired or a signal occurred.
*/
waitread(fd, time)
int fd, time;
{
	fd_set readbits, other;
	struct timeval timer;
	int ret;

	timerclear(&timer);
	timer.tv_sec = time;
	FD_ZERO(&readbits);
	FD_ZERO(&other);
	FD_SET(fd, &readbits);

	ret = select(fd+1, &readbits, &other, &other, &timer);
	if (FD_ISSET(fd, &readbits))
		return 1;
	return 0;
}
