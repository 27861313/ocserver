#include "iepoll.h"
#include <malloc.h>
#include <string.h>

//---------------------------------------------------------------
//function:
//          iocepoll_create
//Access:
//           public
//Parameter:
//          [in] int flags -
//          [in] int noFile -
//          [in] int eventsize -
//Returns:
//          iocepoll * -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocepoll *iocepoll_create(int flags, int noFile, int eventsize)
{
	struct rlimit rlim;
	rlim.rlim_cur = rlim.rlim_max = noFile;
	if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
	{
		fprintf(stderr, "iocepoll create warning: setrlimit failed!\n");
	}
	iocepoll *hepoll = (iocepoll *)malloc(sizeof(iocepoll));
	memset(hepoll, 0, sizeof(iocepoll));
	hepoll->_epfd = epoll_create1(flags);
	if (hepoll->_epfd == -1)
		return NULL;
	hepoll->_evts = (struct epoll_event *)malloc(sizeof(struct epoll_event) * eventsize);
	hepoll->_evtnum = eventsize;
	return hepoll;
}

//---------------------------------------------------------------
//function:
//          icoepoll_add_fd
//Access:
//           public
//Parameter:
//          [in] iocepoll * ocepoll -
//          [in] int fd -
//          [in] unsigned int events -
//          [in] unsigned char ETorNot -
//Returns:
//          int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int iocepoll_add_fd(int epfd, struct epoll_event *hevt, int fd, unsigned int events, unsigned char ETorNot /*false*/)
{
	memset((char *)hevt, 0, sizeof(struct epoll_event));
	hevt->events = events;
	if (ETorNot)
		hevt->events |= EPOLLET;
	hevt->data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, hevt) == -1)
		return -1;
	return 0;
}

//---------------------------------------------------------------
//function:
//          icoepoll_del_fd
//Access:
//           public
//Parameter:
//          [in] iocepoll * ocepoll -
//          [in] int fd -
//          [in] unsigned int events -
//          [in] unsigned char ETorNot -
//Returns:
//          int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int iocepoll_del_fd(int epfd, struct epoll_event *hevt, int fd, unsigned int events, unsigned char ETorNot /*false*/)
{
	memset(hevt, 0, sizeof(struct epoll_event));
	hevt->events = events;
	if (ETorNot)
		hevt->events |= EPOLLET;
	hevt->data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, hevt) == -1)
		return -1;
	return 0;
}

//---------------------------------------------------------------
//function:
//          icoepoll_get_fd
//Access:
//           public
//Parameter:
//          [in] iocepoll * ocepoll -
//          [in] int eventIndex -
//Returns:
//          int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int iocepoll_get_fd(iocepoll *ocepoll, int eventIndex)
{
	return ocepoll->_evts[eventIndex].data.fd;
}

//---------------------------------------------------------------
//function:
//          icoepoll_get_evt
//Access:
//           public
//Parameter:
//          [in] iocepoll * ocepoll -
//          [in] int eventIndex -
//Returns:
//          unsigned int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iocepoll_get_evt(iocepoll *ocepoll, int eventIndex)
{
	return ocepoll->_evts[eventIndex].events;
}

//---------------------------------------------------------------
//function:
//          icopoll_wait
//Access:
//           public
//Parameter:
//          [in] iocepoll ocepoll -
//          [in] int timeout -
//Returns:
//          int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int iocepoll_wait(iocepoll *ocepoll, int timeout)
{
	//m_evts.resize(m_evtfdnum);
	int nReady = 0;
	while (1)
	{
		nReady = epoll_wait(ocepoll->_epfd, ocepoll->_evts, ocepoll->_evtnum, timeout);
		if (nReady == 0)
			return -2; //time out
		else if (nReady == -1)
		{
			if (errno == EINTR)
				continue;
			else
				return -3;
		}
		else
			return nReady;
	}
}

//---------------------------------------------------------------
//function:
//          icoepoll_release
//Access:
//           public
//Parameter:
//          [in] iocepoll * ocepoll -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocepoll_release(iocepoll *ocepoll)
{
	if (ocepoll->_epfd > 0)
	{
		close(ocepoll->_epfd);
		ocepoll->_epfd = -1;
	}

	if (ocepoll->_evts != 0)
	{
		free(ocepoll->_evts);
		ocepoll->_evts = 0;
	}
	ocepoll = NULL;
}