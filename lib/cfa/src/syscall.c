#include "u.h"					/* data types */
#include "syscall.h"

typedef struct syscall_error_t {
	uintptr r1;
	uintptr errno;
} syscall_result;

enum { s_read = 0x0, s_write = 0x1, s_close = 0x3, s_mmap = 0x9,
	s_munmap = 0xb, s_exit = 0x3c, s_clock_gettime = 0xe4,
	s_socket = 0x29, s_bind = 0x31, s_setsockopt = 0x36,
	s_listen = 0x32, s_accept = 0x2b, s_accept4 = 0x120,
	s_epoll_create = 0xd5, s_epoll_wait = 0xe8, s_epoll_ctl = 0xe9,
	s_fork = 0x39, s_epoll_create1 = 0x123
};

/* In order to preserve the value of the rcx register, we specified rcx 
 * as clobbered registers and the compiler saves this register before 
 * executing the asm code. But since rcx is stored in r11, and r11 is also
 * overwritten by the syscall call, we additionally specify r11.
 */
syscall_result syscall6(int trap, uintptr a1, uintptr a2, uintptr a3, uintptr a4, uintptr a5, uintptr a6)
{
	syscall_result r;

	__asm__ __volatile__("movq	%6, %%r10\n\t"
						 "movq	%7, %%r8\n\t"
						 "movq	%8, %%r9\n\t"
						 "syscall\n\t"
						 "cmpq	$-4095, %%rax\n\t"
						 "jbe 	0f\n\t"
						 "movq	$-1, %0\n\t"
						 "neg %%rax\n\t"
						 "movq	%%rax, %1\n\t"
						 "jmp	1f\n\t"
						 "0:\n\t" "movq	%%rax, %0\n\t" "movq 	$0, %1\n\t" "1:\n\t":"=m"(r.r1), "=m"(r.errno)
						 :"a"(trap), "D"(a1), "S"(a2), "d"(a3), "m"(a4), "m"(a5), "m"(a6)
						 :"rcx", "r8", "r9", "r10", "r11");

	return r;
}

/* if you pass the err argument as the fourth argument, it will be passed
 * through the rcx register, but after calling the syscall instruction, 
 * it will overwrite the rcx register (save rip) and r11 (save rflags) and 
 * eventually the program will try to access the address (rip) that is not 
 * writable and will receive a SEGFAULT. In order to preserve 
 * the value of the rcx register, we specified rcx as clobbered registers and 
 * the compiler saves this register before executing the asm code.
 */
syscall_result syscall3(int trap, uintptr a1, uintptr a2, uintptr a3)
{
	syscall_result r;

	__asm__ __volatile__("syscall\n\t"
						 "cmpq	$-4095, %%rax\n\t"
						 "jbe 	0f\n\t"
						 "movq	$-1, %0\n\t"
						 "neg	%%rax\n\t"
						 "movq	%%rax, %1\n\t"
						 "jmp	1f\n\t"
						 "0:\n\t" "movq	%%rax, %0\n\t" "movq 	$0, %1\n\t" "1:\n\t":"=m"(r.r1), "=m"(r.errno)
						 :"a"(trap), "D"(a1), "S"(a2), "d"(a3)
						 :"rcx");

	return r;
}

static const error *set_error(uintptr code)
{
	const error *err;

	if (code == 0)
		return nil;

	if (code > max_err_idx)
		err = &(syscall_errors[max_err_idx]);
	else
		err = &(syscall_errors[code]);
	return err;
}

int64 sys_read(uint32 fd, char *buf, uint64 count, const error ** err)
{
	syscall_result r = syscall3(s_read, fd, (uintptr) buf, count);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int64 sys_write(uint32 fd, const char *buf, uint64 count, const error ** err)
{
	syscall_result r = syscall3(s_write, fd, (uintptr) buf, count);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

const error *sys_close(uint32 fd)
{
	syscall_result r = syscall3(s_close, fd, 0, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

void *sys_mmap(uintptr addr, uint64 len, uintptr prot, uintptr flags, uintptr fd, uintptr offset, const error ** err)
{
	syscall_result r = syscall6(s_mmap, addr, len, prot, flags, fd, offset);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return (void *) r.r1;
}

const error *sys_munmap(uintptr addr, uint64 len)
{
	syscall_result r = syscall3(s_munmap, addr, len, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

void sys_exit(int error_code)
{
	syscall3(s_exit, error_code, 0, 0);
}

const error *sys_clock_gettime(int which_clock, struct timespec *tp)
{
	syscall_result r = syscall3(s_clock_gettime, which_clock, (uintptr) tp, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

int sys_socket(int family, int type, int protocol, const error ** err)
{
	syscall_result r = syscall3(s_socket, family, type, protocol);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

const error *sys_bind(int sockfd, struct sockaddr *addr, int addrlen)
{
	syscall_result r = syscall3(s_bind, sockfd, (uintptr) addr, addrlen);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

const error *sys_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen)
{
	syscall_result r = syscall6(s_setsockopt, sockfd, level, optname, (uintptr) optval,
								optlen, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

const error *sys_listen(int sockfd, int qlen)
{
	syscall_result r = syscall3(s_listen, sockfd, qlen, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}

int sys_accept(int sockfd, struct sockaddr *addr, int *addrlen, const error ** err)
{
	syscall_result r = syscall3(s_accept, sockfd, (uintptr) addr, (uintptr) addrlen);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int sys_accept4(int sockfd, struct sockaddr *addr, int *addrlen, int flags, const error ** err)
{
	syscall_result r = syscall6(s_accept4, sockfd, (uintptr) addr, (uintptr) addrlen,
								flags, 0, 0);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int sys_fork(const error ** err)
{
	syscall_result r = syscall3(s_fork, 0, 0, 0);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int sys_epoll_create(int size, const error ** err)
{
	syscall_result r = syscall3(s_epoll_create, size, 0, 0);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int sys_epoll_create1(int flags, const error ** err)
{
	syscall_result r = syscall3(s_epoll_create1, flags, 0, 0);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

int sys_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout, const error ** err)
{
	syscall_result r = syscall6(s_epoll_wait, epfd, (uintptr) events, maxevents,
								timeout, 0, 0);
	if (err != nil) {
		*err = set_error(r.errno);
	}
	return r.r1;
}

const error *sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	syscall_result r = syscall6(s_epoll_ctl, epfd, op, fd, (uintptr) event, 0, 0);
	if (r.errno != 0) {
		return set_error(r.errno);
	}
	return nil;
}
