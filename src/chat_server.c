/* Chat server */
#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"
#include "pool.h"
#include "arena.h"

enum { max_line_len = 3, backlog = 128, max_events = 16, max_clients = 2 };

//static const char welcome_msg[] = "Welcome to the chat, you are known as ";
//static const char entered_msg[] = " has entered the chat";
//static const char left_msg[] = " has left the chat";
static const char too_long_msg[] = "Line too long! Good bye...\n";
static const char limit_conn_msg[] = "Connection limit reached, rejecting client\n";

typedef struct client_t {
	int fd;
	int buf_used;
	char buf[max_line_len + 1];
	char *name;
	struct client_t *next;
} client;

typedef struct server_t {
	int ls;
	client *first;
} server;

enum { arena_size = sizeof(client) * max_clients + sizeof(server),
	pool_size = max_clients * sizeof(client),
	default_alignment = sizeof(void *),
};

static uint16 hton(uint16 port)
{
	return (port << 8) | (port >> 8);
}

static void session_send_all(string msg, int except, client * to)
{
	while (to != nil) {
		if (except != to->fd)
			sys_write(to->fd, (char *) msg.base, msg.len, nil);
		to = to->next;
	}
}

static void check_line(client * c, server * serv)
{
	string m;
	int i, pos = -1;

	for (i = 0; i < c->buf_used; i++)
		if (c->buf[i] == '\n') {
			pos = i;
			break;
		}

	if (pos == -1)
		return;

	c->buf[pos + 1] = '\0';

	m.base = c->buf;
	m.len = pos + 2;
	session_send_all(m, c->fd, serv->first);

	c->buf_used -= pos + 1;
	memmove(c->buf, c->buf + pos + 1, c->buf_used);
}

static void session_close(client * c, server * serv, pool * p)
{
	client *prev = nil;
	client *temp = serv->first;

	sys_close(c->fd);

	if (temp != nil && temp->fd == c->fd) {
		serv->first = temp->next;
		fmt_fprintf(stdout, "free chank pointer: %p\n", temp);
		pool_put(p, temp);
		return;
	}

	while (temp != nil && temp->fd != c->fd) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == nil) {
		fmt_fprintf(stderr, "session_close failed (item not found)\n");
		return;
	}

	prev->next = temp->next;
	fmt_fprintf(stdout, "free chank pointer: %p\n", temp);
	pool_put(p, temp);
}

static void session_read(client * c, server * serv, pool * p)
{
	const error *err;
	int n, bufp = c->buf_used;

	for (;;) {
		n = sys_read(c->fd, c->buf + bufp, max_line_len - bufp, &err);
		if (err != nil) {
			if (err->code == EAGAIN)
				break;
			else if (err->code == EINTR)
				continue;
			else {
				fmt_fprintf(stderr, "sys_read failed: %s\n", err->msg);
				session_close(c, serv, p);
				return;
			}
		} else if (n == 0) {
			check_line(c, serv);
			session_close(c, serv, p);
			return;
		}

		c->buf_used += n;
		if (c->buf_used == max_line_len) {
			sys_write(c->fd, too_long_msg, sizeof(too_long_msg), nil);
			session_close(c, serv, p);
			return;
		}
	}
	check_line(c, serv);
}

/* =========== server =========== */

static void server_handle(int epfd, server * serv, pool * p)
{
	int conn_sock;
	struct epoll_event ev;
	client *c;
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
		if (conn_sock == max_clients + 5) {
			sys_write(conn_sock, limit_conn_msg, sizeof(limit_conn_msg), nil);
			sys_close(conn_sock);
			continue;
		}

		c = pool_get(p);
		fmt_fprintf(stdout, "new chank pointer: %p\n", c);
		if (c == nil) {
			sys_write(stderr, "Failed to acquire new session\n", 30, nil);
			sys_close(conn_sock);
			continue;
		}
		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = c;
		err = sys_epoll_ctl(epfd, epoll_ctl_add, conn_sock, &ev);
		if (err != nil) {
			fmt_fprintf(stderr, "sys_epoll_ctl failed: %s\n", err->msg);
			sys_close(conn_sock);
			pool_put(p, c);
			continue;
		}
		c->fd = conn_sock;
		c->buf_used = 0;
		c->name = nil;
		c->next = serv->first;
		serv->first = c;
	}
}

static int server_go(server * serv, pool * p)
{
	struct epoll_event ev, evt[max_events] = { 0 };
	int epfd, i;
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
			else
				session_read(evt[i].data.ptr, serv, p);
		}
	}
	return 0;
}

static int server_init(server * serv, uint16 port)
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

	err = sys_listen(serv->ls, backlog);
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
	server *serv;

	arena_create(&a, arena_size);
	fmt_fprintf(stdout, "arena pointer: %p, arena buf_size: %d\n", a.buf, a.buf_len);
	serv = (server *) arena_alloc(&a, sizeof(server));
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
	pool_init(&p, pbuf, pool_size, sizeof(client), default_alignment);

	if (server_init(serv, 7070))
		sys_exit(3);

	sys_exit(server_go(serv, &p));
}
