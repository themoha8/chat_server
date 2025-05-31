/* Chat server */
#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"
#include "pool.h"
#include "arena.h"

enum { max_events = 16, max_buf = 3, max_sess = 2 };

typedef struct session_t {
	int fd;
	int buf_used;
	char buf[max_buf + 1];
	struct session_t *next;
} session;

typedef struct server_t {
	int ls;
	session *first;
} server_str;

enum { arena_size = sizeof(session) * max_sess + sizeof(server_str),
	pool_size = max_sess * sizeof(session),
	default_alignment = 8
};

enum { ok_read = 0, err_read, eof_read, long_read };

static uint16 hton(uint16 port)
{
	return (port << 8) | (port >> 8);
}

static void session_send_if_lf(session * sess, server_str * serv)
{
	session *f;
	int i, pos = -1;

	for (i = 0; i < sess->buf_used; i++)
		if (sess->buf[i] == '\n') {
			pos = i;
			break;
		}

	if (pos == -1)
		return;

	sess->buf[pos + 1] = '\0';

	for (f = serv->first; f != nil; f = f->next) {
		if (f->fd != sess->fd)
			sys_write(f->fd, sess->buf, pos + 1, nil);
	}

	sess->buf_used -= pos + 1;
	memmove(sess->buf, sess->buf + pos + 1, sess->buf_used);
}

static void session_close(session * sess, server_str * serv, pool * p)
{
	session *prev = nil;
	session *temp = serv->first;

	sys_close(sess->fd);

	if (temp != nil && temp->fd == sess->fd) {
		serv->first = temp->next;
		//fmt_fprintf(stdout, "free chank pointer: %p\n", temp);
		pool_put(p, temp);
		return;
	}

	while (temp != nil && temp->fd != sess->fd) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == nil) {
		fmt_fprintf(stderr, "session_close failed (item not found)\n");
		return;
	}

	prev->next = temp->next;
	//fmt_fprintf(stdout, "free chank pointer: %p\n", temp);
	pool_put(p, temp);
}

static int session_read(session * sess)
{
	const error *err;
	int n, bufp = sess->buf_used;

	for (;;) {
		n = sys_read(sess->fd, sess->buf + bufp, max_buf - bufp, &err);
		if (err != nil) {
			if (err->code == EAGAIN)
				break;
			else if (err->code == EINTR)
				continue;
			else {
				fmt_fprintf(stderr, "sys_read failed: %s\n", err->msg);
				return err_read;
			}
		} else if (n == 0) {
			return eof_read;
		}

		sess->buf_used += n;
		if (sess->buf_used == max_buf) {
			sys_write(sess->fd, "Line too long! Good bye...\n", 28, nil);
			return long_read;
		}
	}
	return ok_read;
}

/* =========== server =========== */

static void server_handle(int epfd, server_str * serv, pool * p)
{
	int conn_sock;
	struct epoll_event ev;
	session *sess;
	const error *err;

	for (;;) {
		conn_sock = sys_accept4(serv->ls, nil, nil, sock_nonblock, &err);
		if (err != nil) {
			if (err->code == EAGAIN)
				break;
			else {
				fmt_fprintf(stderr, "sys_accept4 failed: %s\n", err->msg);
				continue;
			}
		}

		/* fd 0, 1, 2, 3 (epoll), 4 (ls) is busy */
		if (conn_sock == max_sess + 5) {
			sys_write(conn_sock, "Connection limit reached, rejecting client\n", 44, nil);
			sys_close(conn_sock);
			continue;
		}

		sess = pool_get(p);
		//fmt_fprintf(stdout, "new chank pointer: %p\n", sess);
		if (sess == nil) {
			sys_write(stderr, "Failed to acquire new session\n", 30, nil);
			sys_close(conn_sock);
			continue;
		}
		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = sess;
		err = sys_epoll_ctl(epfd, epoll_ctl_add, conn_sock, &ev);
		if (err != nil) {
			fmt_fprintf(stderr, "sys_epoll_ctl failed: %s\n", err->msg);
			sys_close(conn_sock);
			pool_put(p, sess);
			continue;
		}
		sess->fd = conn_sock;
		sess->buf_used = 0;
		sess->next = serv->first;
		serv->first = sess;
	}
}

static int server_go(server_str * serv, pool * p)
{
	struct epoll_event ev, evt[max_events] = { 0 };
	int epfd, res, i;
	session *sess;
	const error *err;

	epfd = sys_epoll_create1(0, &err);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_epoll_create1 failed: %s\n", err->msg);
		sys_close(serv->ls);
		return 1;
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = serv;
	err = sys_epoll_ctl(epfd, epoll_ctl_add, serv->ls, &ev);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_epoll_ctl failed: %s\n", err->msg);
		sys_close(epfd);
		sys_close(serv->ls);
		return 2;
	}

	for (;;) {
		int ev_count = sys_epoll_wait(epfd, evt, max_events, -1, &err);
		if (err != nil) {
			if (err->code != EINTR)
				fmt_fprintf(stderr, "sys_epoll_wait failed: %s\n", err->msg);
			continue;
		}

		for (i = 0; i < ev_count; i++) {
			if (evt[i].data.ptr == serv)
				server_handle(epfd, serv, p);
			else {
				sess = (session *) evt[i].data.ptr;
				res = session_read(sess);
				if (res == eof_read) {
					session_send_if_lf(sess, serv);
					session_close(sess, serv, p);
				} else if (res == ok_read)
					session_send_if_lf(sess, serv);
				else
					session_close(sess, serv, p);
			}
		}
	}
	return 0;
}

static int server_init(server_str * serv, uint16 port)
{
	struct sockaddr_in addr;
	int enable = 1;
	const error *err;

	serv->ls = sys_socket(af_inet, sock_stream | sock_nonblock, 0, &err);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_socket failed: %s\n", err->msg);
		return 1;
	}
	serv->first = nil;

	/* Disable TIME_WAIT (block port) after incorrect program termination. */
	err = sys_setsockopt(serv->ls, sol_socket, so_reuseaddr, &enable, sizeof(enable));
	if (err != nil) {
		fmt_fprintf(stderr, "sys_setsockopt failed: %s\n", err->msg);
		return 2;
	}

	addr.sin_family = af_inet;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = hton(port);
	addr.sin_zero[0] = 0L;
	err = sys_bind(serv->ls, (struct sockaddr *) &addr, sizeof(addr));
	if (err != nil) {
		fmt_fprintf(stderr, "sys_bind failed: %s\n", err->msg);
		return 3;
	}

	err = sys_listen(serv->ls, 128);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_listen failed: %s\n", err->msg);
		return 4;
	}

	return 0;
}

void _start(void)
{
	arena a;
	pool p;
	byte *pbuf;
	server_str *serv;

	arena_create(&a, arena_size);
	fmt_fprintf(stdout, "arena pointer: %p, arena buf_size: %d\n", a.buf, a.buf_len);
	serv = (server_str *) arena_alloc(&a, sizeof(server_str));
	if (serv == nil) {
		fmt_fprintf(stderr, "arena_alloc (server_str) failed\n");
		sys_exit(1);
	}
	fmt_fprintf(stdout, "server_str pointer: %p, server_str size: %d\n", serv, a.curr_off);

	pbuf = (byte *) arena_alloc(&a, pool_size);
	if (pbuf == nil) {
		fmt_fprintf(stderr, "arena_alloc (pool) failed\n");
		sys_exit(2);
	}
	fmt_fprintf(stdout, "pool pointer: %p, pool size: %d\n", pbuf, a.curr_off - a.prev_off);
	pool_init(&p, pbuf, pool_size, sizeof(session), default_alignment);

	if (server_init(serv, 7070))
		sys_exit(3);

	sys_exit(server_go(serv, &p));
}
