/* Chat server */
#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "assert.h"
#include "fmt.h"
#include "time.h"
#include "pool.h"
#include "arena.h"

static const char welcome_msg[] = "Welcome to the chat, you are known as ";
static const char entered_msg[] = " has entered the chat\n";
static const char left_msg[] = " has left the chat\n";
static const char too_long_msg[] = "Line too long! Good bye...\n";
static const char too_long_name[] = "Name too long! Good bye...\n";
static const char limit_conn_msg[] = "Connection limit reached, rejecting client\n";

enum {
	max_line_len = 4,
	max_name_len = 4,
	backlog = 128,
	max_events = 16,
	max_clients = 1,
	max_pools = 3,
	page_size = 4096
};

typedef struct client_t {
	int fd;
	int buf_used;
	char buf[max_line_len];
	char name[max_name_len];
	int name_used;
	bool name_ok;
	struct client_t *next;
	struct client_t *prev;
} client;

typedef struct client_pool_t {
	pool p;
	int free_ch;
	int used_ch;
	client *client;
} client_pool;

typedef struct server_t {
	int ls;
	int n_pls;
	client_pool **first_clp;
} server;

enum { pool_size = sizeof(client) * max_clients,
	default_alignment = sizeof(void *)
};

static uint16 hton(uint16 port)
{
	return (port << 8) | (port >> 8);
}

static void session_send_all(string msg, client * except, server * serv)
{
	int i;
	client *c;

	for (i = 0; i < serv->n_pls; i++) {
		c = serv->first_clp[i]->client;
		while (c != nil) {
			if (except != c)
				sys_write(c->fd, (char *) msg.base, msg.len, nil);
			c = c->next;
		}
	}
}

static void check_line_and_send(client * c, server * serv)
{
	int i, pos = -1;
	/* 17 - for time and brackets */
	char msg[max_line_len + max_name_len + 17];
	slice s;
	struct tm t;
	const error *err;
	struct timespec tp;

	for (i = 0; i < c->buf_used; i++)
		if (c->buf[i] == '\n') {
			pos = i;
			break;
		}

	if (pos == -1)
		return;

	i = 0;
	s = unsafe_slice(msg, sizeof(msg));
	i = c_string_in_slice(s, c->name);

	err = sys_clock_gettime(clock_realtime, &tp);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_clock_gettime: %s\n", err->msg);
		i += c_nstring_in_slice(slice_left(s, i), ": ", 2);
		i += c_nstring_in_slice(slice_left(s, i), c->buf, pos + 1);
		i += c_nstring_in_slice(slice_left(s, i), "", 1);
	} else {
		t = time_to_tm(tp.tv_sec);
		i += c_nstring_in_slice(slice_left(s, i), " (", 2);
		i += tm_in_slice2(slice_left(s, i), &t);
		i += c_nstring_in_slice(slice_left(s, i), "): ", 3);
		i += c_nstring_in_slice(slice_left(s, i), c->buf, pos + 1);
		i += c_nstring_in_slice(slice_left(s, i), "", 1);
	}

	session_send_all(get_string(slice_right(s, i)), c, serv);

	c->buf_used -= pos + 1;
	memmove(c->buf, c->buf + pos + 1, c->buf_used);
}

static void session_close(client * c, server * serv)
{
	client *prev = nil;
	client *temp = nil;
	client_pool *clp;
	const error *err;
	int i = 0;
	slice s;
	char msg[max_line_len + sizeof(left_msg)];

	if (c->name_ok == true) {
		s = unsafe_slice(msg, sizeof(msg));
		i = c_string_in_slice(s, c->name);
		i += c_nstring_in_slice(slice_left(s, i), left_msg, sizeof(left_msg) - 1);
		i += c_nstring_in_slice(slice_left(s, i), "\n", 2);
		session_send_all(get_string(slice_right(s, i)), c, serv);
	}

	sys_close(c->fd);
	for (i = 0; i < serv->n_pls; i++) {
		temp = serv->first_clp[i]->client;
		clp = serv->first_clp[i];
		if (temp != nil && temp == c) {
			clp->client = temp->next;
			clp->free_ch++;
			clp->used_ch--;
			if (serv->n_pls > 1 && clp->used_ch == 0) {
				err = sys_munmap((uintptr) clp, page_size);
				if (err != nil) {
					fmt_fprintf(stderr, "session_close: sys_munmap failed: %s\n", err->msg);
				}
#ifdef DEBUG_PRINT
				int n;
				for (n = 0; n < serv->n_pls; n++)
					fmt_fprintf(stdout, "before: serv->first_clp[%d]: %p\n", n, serv->first_clp[n]);
#endif

				serv->n_pls--;
				while (i < serv->n_pls) {
					serv->first_clp[i] = serv->first_clp[i + 1];
					i++;
				}
				serv->first_clp[i] = nil;

#ifdef DEBUG_PRINT
				for (n = 0; n < serv->n_pls + 1; n++)
					fmt_fprintf(stdout, "after: serv->first_clp[%d]: %p\n", n, serv->first_clp[n]);
#endif

				return;
			}
			pool_put(&clp->p, temp);

#ifdef DEBUG_PRINT
			fmt_fprintf(stdout, "free client chunk address: %p\n", temp);
#endif

			return;
		}
	}

	for (i = 0; i < serv->n_pls; i++) {
		temp = serv->first_clp[i]->client;
		clp = serv->first_clp[i];
		while (temp != nil && temp != c) {
			prev = temp;
			temp = temp->next;
		}
		if (temp != nil)
			break;
	}

	if (temp == nil) {
		fmt_fprintf(stderr, "session_close failed (item not found)\n");
		return;
	}

	prev->next = temp->next;
	clp->free_ch++;
	clp->used_ch--;
	if (serv->n_pls > 1 && clp->used_ch == 0) {
		err = sys_munmap((uintptr) clp, page_size);
		if (err != nil) {
			fmt_fprintf(stderr, "session_close: sys_munmap failed: %s\n", err->msg);
		}
#ifdef DEBUG_PRINT
		int n;
		for (n = 0; n < serv->n_pls; n++)
			fmt_fprintf(stdout, "before: serv->first_clp[%d]: %p\n", n, serv->first_clp[n]);
#endif

		serv->n_pls--;
		while (i < serv->n_pls) {
			serv->first_clp[i] = serv->first_clp[i + 1];
		}
		serv->first_clp[i] = nil;

#ifdef DEBUG_PRINT
		for (n = 0; n < serv->n_pls + 1; n++)
			fmt_fprintf(stdout, "after: serv->first_clp[%d]: %p\n", n, serv->first_clp[n]);
#endif

		return;
	}
	pool_put(&clp->p, temp);

#ifdef DEBUG_PRINT
	fmt_fprintf(stdout, "free client chunk address: %p\n", c);
#endif
}

static void session_name_read(client * c, server * serv)
{
	const error *err;
	int n, bufn = c->name_used;
	/* if welcome_msg > entered_msg */
	char msg[sizeof(welcome_msg) + max_name_len];
	slice s;

	for (;;) {
		n = sys_read(c->fd, c->name + bufn, max_name_len - bufn, &err);
		if (err != nil) {
			if (err->code == EAGAIN)
				break;
			else if (err->code == EINTR)
				continue;
			else {
				fmt_fprintf(stderr, "session_read: sys_read failed: %s\n", err->msg);
				session_close(c, serv);
				return;
			}
		} else if (n == 0) {
			session_close(c, serv);
			return;
		}

		c->name_used += n;
		if (c->name_used > max_name_len) {
			sys_write(c->fd, too_long_name, sizeof(too_long_name), nil);
			session_close(c, serv);
			return;
		}
	}

	for (n = 0; n < c->name_used; n++) {
		if (c->name[n] == '\n') {
			c->name[n] = '\0';
			c->name_ok = true;
			c->name_used = n;
		}
	}

	if (c->name_ok == true) {
		s = unsafe_slice(msg, sizeof(msg));

		n = c_nstring_in_slice(s, welcome_msg, sizeof(welcome_msg) - 1);
		n += c_nstring_in_slice(slice_left(s, n), c->name, c->name_used);
		n += c_nstring_in_slice(slice_left(s, n), "\n", 2);
		sys_write(c->fd, s.base, n, nil);

		n = c_nstring_in_slice(s, c->name, c->name_used);
		n += c_nstring_in_slice(slice_left(s, n), entered_msg, sizeof(entered_msg));

		session_send_all(get_string(slice_right(s, n)), c, serv);
	}
}

static void session_line_read(client * c, server * serv)
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
				fmt_fprintf(stderr, "session_read: sys_read failed: %s\n", err->msg);
				session_close(c, serv);
				return;
			}
		} else if (n == 0) {
			/* Is it necessary? */
			check_line_and_send(c, serv);
			session_close(c, serv);
			return;
		}

		c->buf_used += n;
		if (c->buf_used > max_line_len) {
			sys_write(c->fd, too_long_msg, sizeof(too_long_msg), nil);
			session_close(c, serv);
			return;
		}
	}
	check_line_and_send(c, serv);
}

static void session_new_clp(server * serv)
{
	arena a;
	byte *pbuf;
	client_pool *clp_new;

	arena_create(&a, pool_size + sizeof(client_pool));

	clp_new = (client_pool *) arena_alloc(&a, sizeof(client_pool));
	if (clp_new == nil) {
		fmt_fprintf(stderr, "session_new_clp: arena_alloc (client_pool) failed\n");
		sys_exit(1);
	}

	pbuf = (byte *) arena_alloc(&a, pool_size);
	if (pbuf == nil) {
		fmt_fprintf(stderr, "session_new_clp: arena_alloc (pool) failed\n");
		sys_exit(2);
	}

	pool_init(&clp_new->p, pbuf, pool_size, sizeof(client), default_alignment);
	clp_new->used_ch = 0;
	clp_new->free_ch = clp_new->p.buf_len / clp_new->p.chunk_size;
	clp_new->client = nil;

	serv->first_clp[serv->n_pls] = clp_new;
	serv->n_pls++;
}

static client *session_new(int epfd, server * serv)
{
	client *c = nil;
	client_pool *clp;
	int i;

	for (i = 0; i < serv->n_pls; i++) {
		clp = serv->first_clp[i];
		if (clp->free_ch > 0) {
			c = pool_get(&clp->p);
			clp->free_ch--;
			clp->used_ch++;
			c->next = clp->client;
			clp->client = c;

#ifdef DEBUG_PRINT
			fmt_fprintf(stdout, "new client chunk address: %p\n", c);
#endif

			return c;
		}
	}

	if (c == nil) {
		if (serv->n_pls == max_pools)
			return nil;

		session_new_clp(serv);
		clp = serv->first_clp[serv->n_pls - 1];
		c = pool_get(&clp->p);
		clp->client = c;
		clp->free_ch--;
		clp->used_ch++;
	}
#ifdef DEBUG_PRINT
	fmt_fprintf(stdout, "new client chunk address: %p\n", c);
#endif

	return c;
}

/* =========== server =========== */

static void server_handle(int epfd, server * serv)
{
	struct epoll_event ev;
	const error *err;
	int conn_sock;
	client *c;

	for (;;) {
		conn_sock = sys_accept4(serv->ls, nil, nil, sock_nonblock, &err);
		if (err != nil) {
			if (err->code == EAGAIN)
				break;
			else {
				fmt_fprintf(stderr, "server_handle: sys_accept4 failed: %s\n", err->msg);
				continue;
			}
		}

		c = session_new(epfd, serv);
		if (c == nil) {
			sys_write(conn_sock, limit_conn_msg, sizeof(limit_conn_msg), nil);
			sys_close(conn_sock);
			continue;
		}

		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = c;
		err = sys_epoll_ctl(epfd, epoll_ctl_add, conn_sock, &ev);
		if (err != nil) {
			fmt_fprintf(stderr, "server_handle: sys_epoll_ctl failed: %s\n", err->msg);
			session_close(c, serv);
			continue;
		}

		c->fd = conn_sock;
		c->buf_used = 0;
		c->name_used = 0;
		c->name_ok = false;

		sys_write(conn_sock, "Your name please (max 32 symbols): ", 36, nil);
	}
}

static int server_go(server * serv)
{
	struct epoll_event ev, evt[max_events];
	int epfd, i;
	client *c;
	const error *err;

	epfd = sys_epoll_create1(0, &err);
	if (err != nil) {
		fmt_fprintf(stderr, "server_go: sys_epoll_create1 failed: %s\n", err->msg);
		sys_close(serv->ls);
		return 1;
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = serv;
	err = sys_epoll_ctl(epfd, epoll_ctl_add, serv->ls, &ev);
	if (err != nil) {
		fmt_fprintf(stderr, "server_go: sys_epoll_ctl failed: %s\n", err->msg);
		sys_close(epfd);
		sys_close(serv->ls);
		return 2;
	}

	for (;;) {
		int ev_count = sys_epoll_wait(epfd, evt, max_events, -1, &err);
		if (err != nil) {
			if (err->code != EINTR)
				fmt_fprintf(stderr, "server_go: sys_epoll_wait failed: %s\n", err->msg);
			continue;
		}

		for (i = 0; i < ev_count; i++) {
			if (evt[i].data.ptr == serv)
				server_handle(epfd, serv);
			else {
				c = (client *) evt[i].data.ptr;
				if (c->name_ok == false)
					session_name_read(c, serv);
				else
					session_line_read(c, serv);
			}
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
		fmt_fprintf(stderr, "server_init: sys_socket failed: %s\n", err->msg);
		return 1;
	}

	/* Disable TIME_WAIT (block port) after incorrect program termination. */
	err = sys_setsockopt(serv->ls, sol_socket, so_reuseaddr, &enable, sizeof(enable));
	if (err != nil) {
		fmt_fprintf(stderr, "server_init: sys_setsockopt failed: %s\n", err->msg);
		return 2;
	}

	addr.sin_family = af_inet;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = hton(port);
	addr.sin_zero[0] = 0L;
	err = sys_bind(serv->ls, (struct sockaddr *) &addr, sizeof(addr));
	if (err != nil) {
		fmt_fprintf(stderr, "server_init: sys_bind failed: %s\n", err->msg);
		return 3;
	}

	err = sys_listen(serv->ls, backlog);
	if (err != nil) {
		fmt_fprintf(stderr, "server_init: sys_listen failed: %s\n", err->msg);
		return 4;
	}

	return 0;
}

void _start(void)
{
	server serv;
	arena a;

	arena_create(&a, max_pools * 8);
	serv.first_clp = arena_alloc(&a, max_pools * 8);
	serv.n_pls = 0;
	/* create pool for clients */
	session_new_clp(&serv);

	if (server_init(&serv, 7070))
		sys_exit(1);

#ifdef DEBUG_PRINT
	fmt_fprintf(stdout, "serv.first_clp: %p\n", serv.first_clp);
	fmt_fprintf(stdout, "serv.first_clp[0]: %p\n", serv.first_clp[0]);
	fmt_fprintf(stdout, "serv.first_clp[0]->p.buf: %p\n", serv.first_clp[0]->p.buf);
	fmt_fprintf(stdout, "serv.first_clp[0]->p.buf_len: %d\n", serv.first_clp[0]->p.buf_len);
	fmt_fprintf(stdout, "serv.first_clp[0]->p.chunk_size: %d\n", serv.first_clp[0]->p.chunk_size);
	fmt_fprintf(stdout, "serv.first_clp[0]->p.head: %p\n", serv.first_clp[0]->p.head);
#endif

	sys_exit(server_go(&serv));
}
