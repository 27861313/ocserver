#include "inet.h"
#include "itcpserver.h"
#include <arpa/inet.h>
#include <common/ithread.h>
#include <sys/socket.h>
#include <sys/types.h>

uint32_i ioctcpfd_create()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int32_i ioctcp_connect(uint32_i fd, iendpoint remote)
{
	int32_i nr = connect(fd, (struct sockaddr *)&remote._sckaddr, iocnet_getsockaddrsize(&remote));
	if (nr == -1)
		return -1;
	return 0;
}

inline socklen_t iocnet_getsockaddrsize(const iendpoint *addrs)
{
	return sizeof(addrs->_sckaddr);
}

int32_i iocnet_setreuseaddr(int32_i sck)
{
	int flag = 1;
	return setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, (const void *)&flag, sizeof(flag));
}

int32_i iocnet_setnonblock(int32_i sck)
{
	int32_i flag = fcntl(sck, F_GETFL);
	flag |= O_NONBLOCK;
	return fcntl(sck, F_SETFL, flag);
}

int32_i iocnet_close(int32_i sck)
{
	int32_i ret = -1;
	do
	{
		ret = close(sck);
	} while (ret == -1 && errno == EINTR);

	return ret;
}

void iocnet_toaddr(iendpoint *outaddr, const struct sockaddr_in *inaddr)
{
	if (outaddr == NULL || inaddr == NULL)
	{
		return;
	}
	strncpy(outaddr->_addr, inet_ntoa(inaddr->sin_addr), IP_ADDR);
	outaddr->_port = inaddr->sin_port;
	memcpy(&outaddr->_sckaddr, (char *)inaddr, sizeof(struct sockaddr_in));
}

int32_i iocnet_bind(int32_i sck, iendpoint addr)
{
	memset(&addr._sckaddr, 0, sizeof(addr._sckaddr));
	addr._sckaddr.sin_family = AF_INET;
	addr._sckaddr.sin_port = htons((uint16_t)addr._port);
	if (strncmp(addr._addr, "", IP_BUF) != 0)
		addr._sckaddr.sin_addr.s_addr = inet_addr(addr._addr); //��ָ����ַ
	else
		addr._sckaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	return bind(sck, (struct sockaddr *)&addr._sckaddr, iocnet_getsockaddrsize(&addr));
}

int32_i iocnet_tcpbind(itcpserver *tcpser, const char *addr, unsigned short int port)
{
	strncpy(tcpser->_tcpendpoint._addr, addr, IP_ADDR);
	tcpser->_tcpendpoint._port = port;
	return iocnet_bind(tcpser->_tcpfd, tcpser->_tcpendpoint);
}

int32_i iocnet_listen(int32_i sck, int32_i backlog)
{
	return listen(sck, backlog);
}

int32_i iocnet_accept(int32_i sck, iendpoint *addrs)
{
	int32_i conn_fd = -1;
	struct sockaddr_in addr;
	socklen_t scklen;
	do
	{
		conn_fd = accept(sck, (struct sockaddr *)&addr, &scklen);
	} while (conn_fd == -1 && errno == EINTR);

	iocnet_toaddr(addrs, &addr);
	return conn_fd;
}

int32_i iocnet_recv(int32_i sck, void *buf, int32_i size)
{
	int irecv = 0;
	do
	{
		irecv = recv(sck, buf, size, 0);
	} while (irecv == -1 && errno == EINTR);

	return irecv;
}

int32_i iocnet_send(int32_i sck, const void *buf, int size)
{
	int32_i isend = 0;
	do
	{
		isend = send(sck, buf, size, 0);
	} while (isend == -1 && errno == EINTR);

	return isend;
}

int32_i iocnet_recvfrom(int32_i sck, void *buf, int size, const iendpoint *addrs)
{
	int32_i irecvfrom = 0;
	socklen_t scksize = iocnet_getsockaddrsize(addrs);
	do
	{
		irecvfrom = recvfrom(sck, buf, size, 0, (struct sockaddr *)&addrs->_sckaddr, &scksize);
		if (irecvfrom == 0)
			return 0;
	} while (irecvfrom == -1 && (errno == EINTR || errno == EAGAIN));

	return irecvfrom;
}

int32_i iocnet_sendkeepon(int32_i sck, void *buf, int size, const iendpoint *addrs)
{
	int32_i isend = 0;
	struct sockaddr_in cltaddr;
	socklen_t scksize = iocnet_getsockaddrsize(addrs);
	do
	{
		isend = sendto(sck, buf, size, 0, (struct sockaddr *)&addrs->_sckaddr, scksize);
		if (isend == 0)
			return 0;
		if (isend == -1 && errno == EAGAIN) //需要继续
		{
			//需要做一个记录，判断是否存在这种情况
			ISLEEP(20);
			continue;
		}
	} while (isend == -1 && (errno == EINTR || errno == EAGAIN));
	return isend;
}

int32_i iocnet_connect(int32_i sck, const char *ip, unsigned short int port)
{
	struct sockaddr_in servaddr = {0};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);
	return connect(sck, (struct sockaddr *)&servaddr, sizeof(servaddr));
}
