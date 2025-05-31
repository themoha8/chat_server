#ifndef SYSCALL_H_SENTRY
#define SYSCALL_H_SENTRY

#include "errno.h"

/* Flags for mmap */
enum { map_private = 0x2, map_anonymous = 0x20, map_fixed = 0x10,
	prot_read = 0x1, prot_write = 0x2, s_setcockopt = 0x36
};

/* Flags for time */
enum { clock_realtime = 0x0 };

/* Flags for read and write */
enum { stdin = 0, stdout = 1, stderr = 2 };

/* Flags and data types for network routines */
typedef uint32 in_addr_t;
typedef uint16 sa_family_t;
typedef uint16 in_port_t;

/* Types */
enum {
	/* Stream socket. */
	sock_stream = 1,
	/* Datagram socket. */
	sock_dgram = 2,
	/* Automatically mark descriptor(s) as non-blocking. */
	sock_nonblock = 00004000
};

/* Option flags per-socket. */
enum {
	/* Allow local address & port reuse. */
	so_reuseport = 15,
	so_reuseaddr = 2
};

/* Address families. */
enum {
	/* Standardized name for AF_LOCAL. */
	af_unix = 1,
	/* Local to host (pipes, portals). */
	af_local = af_unix,
	/* Internetwork: UDP, TCP, etc. */
	af_inet = 2
};

/* Level number for (get/set)sockopt() to apply to socket itself. */
enum { sol_socket = 1 };

#define INADDR_ANY ((in_addr_t)0x00000000)

/* Internet address (a structure for historical reasons). */
struct in_addr {
	in_addr_t s_addr;
};

/* Socket address, internet style. */
struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
	char sin_zero[8];
};

/* Structure used by kernel to store most addresses. */
struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

/* Data types for epoll. */
typedef union epoll_data {
	void *ptr;
	int fd;
	uint32 u32;
	uint64 u64;
} epoll_data_t;

/* Without __attribute__((packed)) data.ptr is returned stripped from epoll_wait.
 * __attribute__((packed)) does not padding between types in a structure for alignment.
 * sizeof(struct epoll_event) = 12 if use __attribute__((packed)) else 16
 */
struct epoll_event {
	uint32 events;				/* Epoll events. */
	epoll_data_t data;			/* User data variable. */
} __attribute__ ((packed));

enum {
	/* Add a file descriptor to the interface. */
	epoll_ctl_add = 1,
	/* Remove a file descriptor from the interface. */
	epoll_ctl_del = 2,
	/* Change file descriptor epoll_event structure. */
	epoll_ctl_mod = 3
};

#define EPOLLIN 1
#define EPOLLOUT 4
#define EPOLLET 1u << 31

struct timespec {
	int64 tv_sec;				/* seconds */
	int64 tv_nsec;				/* and nanoseconds */
};

int64 sys_read(uint32 fd, char *buf, uint64 count, const error ** err);
int64 sys_write(uint32 fd, const char *buf, uint64 count, const error ** err);
const error *sys_close(uint32 fd);
void *sys_mmap(uintptr addr, uint64 len, uintptr prot, uintptr flags, uintptr fd, uintptr offset, const error ** err);
const error *sys_munmap(uintptr addr, uint64 len);
void sys_exit(int error_code);
const error *sys_clock_gettime(int which_clock, struct timespec *tp);
int sys_socket(int family, int type, int protocol, const error ** err);
const error *sys_bind(int sockfd, struct sockaddr *addr, int addrlen);
const error *sys_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);
const error *sys_listen(int sockfd, int qlen);
int sys_accept(int sockfd, struct sockaddr *addr, int *addrlen, const error ** err);
int sys_accept4(int sockfd, struct sockaddr *addr, int *addrlen, int flags, const error ** err);
int sys_fork(const error ** err);
int sys_epoll_create(int size, const error ** err);
int sys_epoll_create1(int flags, const error ** err);
int sys_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout, const error ** err);
const error *sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

#endif
