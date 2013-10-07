#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#define MODE_SERVER 0
#define MODE_CLIENT 1
#define MAX_CONNECTIONS 10

struct ProxyInfo
{
	int mode;
	int tcpFD;
	int tapFD;
	struct sockaddr_in address;
}

void printHelp()
{
	printf("Usage: \nServer: vpnproxy <port> <local interface>\nClient: vpnproxy <remote host> <remote port> <local interface>");
}

/**************************************************
* allocate_tunnel:
* open a tun or tap device and returns the file
* descriptor to read/write back to the caller
*****************************************/
int allocate_tunnel(char *dev, int flags) 
{
	int fd, error;
	struct ifreq ifr;
	char *device_name = "/dev/net/tun";
	if( (fd = open(device_name , O_RDWR)) < 0 ) {
		perror("error opening /dev/net/tun");
		return fd;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	if (*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
	if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
		perror("ioctl on tap failed");
		close(fd);
		return error;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

// returns -1 on fail, FD on success
int createSocket(char *host, unsigned short port, struct ProxyInfo *proxyinfo)
{
	struct sockaddr_in to; /* remote internet address */
	struct hostent *hp; /* remote host info from gethostbyname() */

	memset(&to, 0, sizeof(to));

	to.sin_family = AF_INET;
	to.sin_port = htons(port);
	if (host == NULL)
	{
		to.sin_addr.s_addr = inet_addr(INADDR_ANY);
	} else
	{
		/* If internet "a.d.c.d" address is specified, use inet_addr()
		* to convert it into real address. If host name is specified,
		* use gethostbyname() to resolve its address */
		to.sin_addr.s_addr = inet_addr(host); /* If "a.b.c.d" addr */
		if (to.sin_addr.s_addr == -1)
		{
			hp = gethostbyname(host); //hostname in form of text
			if (hp == NULL) {
				fprintf(stderr, "Host name %s not found\n", hostname);
				return -1;
			}
			bcopy(hp->h_addr, &to.sin_addr, hp->h_length);
		}
	}


	int socketFD = socket(PF_INET, SOCK_STEAM, IPPROTO_TCP);
	if (socketFD == -1)
	{
		perror("Creating socket failed");
		return -2;
	}

	int optval = 1;
	/* avoid EADDRINUSE error on bind() */
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
		perror("setsockopt() failed");
		return -3;
	}


	/* give the connect call the sockaddr_in struct that has the address
	* in it on the connect call */
/*	if (connect(socketFD, &to, sizeof(to)) < 0) {
		perror("Connect failed");
		return -3;
	}
*/
}

// returns -1 on fail, FD on success
int createTap(char *tap)
{
	int tapFD = -1;
	if ( (tap_FD = allocate_tunnel(tap, IFF_TAP | IFF_NO_PI)) < 0 ) 
	{
		perror("Opening tap interface failed! \n");
		return -1;
	}
	return tapFD;
}

int main(int argc, char **argv)
{
	struct ProxyInfo proxyinfo;
	unsigned short port;
	char *host;
	char *tap;
	pthread_t tcpThread;
	pthread_t tapThread;

	if (argc == 3) // SERVER
	{
		proxyinfo.mode = MODE_SERVER;
		host = NULL;
		port = (unsigned short)strtoul(argv[1], NULL, 0)
		tap = arg[2];
	} else if (argc == 4) // CLIENT
	{
		proxyinfo.mode = MODE_CLIENT;
		host = argv[1];
		port = (unsigned short)strtoul(argv[2], NULL, 0)
		tap = arg[3];
	} else
	{
		printHelp();
		return 1;
	}

	
}
