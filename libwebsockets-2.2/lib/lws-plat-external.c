#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include "private-libwebsockets.h"

#include <sys/time.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <sys/fcntl.h>
#include <kernel.h>

int select_no_block(int nfds, fd_set *readfds, fd_set *writefds)
{
	struct timeval no_block_timeval = { 0, 0 };
	return select(nfds, readfds, writefds, NULL, &no_block_timeval);
}

static int lws_poll(struct lws_pollfd* fd_array, int fd_count, int timeout_ms)
{
	// set up arguments for select
	fd_set temp_fd_set_in, temp_fd_set_out;
	FD_ZERO(&temp_fd_set_in);
	FD_ZERO(&temp_fd_set_out);
	int highestFD = -1;
	for (int i = 0; i < fd_count; ++i)
	{
		int fd = fd_array[i].fd;
		short events = fd_array[i].events;
		if (fd > highestFD)
		{
			highestFD = fd;
		}
		if (events & LWS_POLLIN)
		{
			FD_SET(fd, &temp_fd_set_in);
		}
		if (events & LWS_POLLOUT)
		{
			FD_SET(fd, &temp_fd_set_out);
		}
	}
	//FD_SET(wsi->desc.sockfd, &temp_fd_set_out);

	struct timeval timeout_struct = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
	
	int result = select(highestFD + 1, &temp_fd_set_in, &temp_fd_set_out, NULL, &timeout_struct);
	if (result <= 0)
		return result;

	// read out select results
	for (int i = 0; i < fd_count; ++i)
	{
		fd_array[i].revents = 0;		
		int fd = fd_array[i].fd;
		if (FD_ISSET(fd, &temp_fd_set_in))
		{
			fd_array[i].revents |= LWS_POLLIN;
		}
		if (FD_ISSET(fd, &temp_fd_set_out))
		{
			fd_array[i].revents |= LWS_POLLOUT;
		}
	}
	return result;
}

unsigned long long
time_in_microseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long)tv.tv_sec * 1000000LL) + tv.tv_usec;
}

LWS_VISIBLE int
lws_get_random(struct lws_context *context, void *buf, int len)
{
	return read(context->fd_random, (char *)buf, len);
}

LWS_VISIBLE int
lws_send_pipe_choked(struct lws *wsi)
{
	/* treat the fact we got a truncated send pending as if we're choked */
	if (wsi->trunc_len)
		return 1;

	fd_set temp_fd_set_out;
	FD_ZERO(&temp_fd_set_out);
	FD_SET(wsi->desc.sockfd, &temp_fd_set_out);

	int result = 1;
	int numberOfFds = select_no_block(1, NULL, &temp_fd_set_out);
	if (numberOfFds > 0 && FD_ISSET(wsi->desc.sockfd, &temp_fd_set_out))
	{
		/* okay to send another packet without blocking */
		result = 0;
	}

	return result;
}

LWS_VISIBLE int
lws_poll_listen_fd(struct lws_pollfd *fd)
{
	return lws_poll(fd, 1, 0);
}

/**
* lws_cancel_service_pt() - Cancel servicing of pending socket activity
*				on one thread
* @wsi:	Cancel service on the thread this wsi is serviced by
*
*	This function let a call to lws_service() waiting for a timeout
*	immediately return.
*/
LWS_VISIBLE void
lws_cancel_service_pt(struct lws *wsi)
{
	struct lws_context_per_thread *pt = &wsi->context->pt[(int)wsi->tsi];
	char buf = 0;

	if (write(pt->dummy_pipe_fds[1], &buf, sizeof(buf)) != 1)
		lwsl_err("Cannot write to dummy pipe");
}

/**
* lws_cancel_service() - Cancel ALL servicing of pending socket activity
* @context:	Websocket context
*
*	This function let a call to lws_service() waiting for a timeout
*	immediately return.
*/
LWS_VISIBLE void
lws_cancel_service(struct lws_context *context)
{
	struct lws_context_per_thread *pt = &context->pt[0];
	char buf = 0, m = context->count_threads;

	while (m--) {
		if (write(pt->dummy_pipe_fds[1], &buf, sizeof(buf)) != 1)
			lwsl_err("Cannot write to dummy pipe");
		pt++;
	}
}

/* epic.asf - not sure what to do here */
#if defined(_DEBUG)
static const char emit_syslog_string_debug[] = "LOG_DEBUG";
static const char emit_syslog_string_err[] = "LOG_ERR";
static const char emit_syslog_string_warning[] = "LOG_WARNING";
static const char emit_syslog_string_notice[] = "LOG_NOTICE";
static const char emit_syslog_string_info[] = "LOG_INFO";
#endif
LWS_VISIBLE void lwsl_emit_syslog(int level, const char *line)
{
#if defined(_DEBUG)
	const char* syslog_level = emit_syslog_string_debug;

	switch (level) {
	case LLL_ERR:
		syslog_level = emit_syslog_string_err;
		break;
	case LLL_WARN:
		syslog_level = emit_syslog_string_warning;
		break;
	case LLL_NOTICE:
		syslog_level = emit_syslog_string_notice;
		break;
	case LLL_INFO:
		syslog_level = emit_syslog_string_info;
		break;
	}
	printf("%s: %s", syslog_level, line);
#endif
}

LWS_VISIBLE int
_lws_plat_service_tsi(struct lws_context *context, int timeout_ms, int tsi)
{
	struct lws_context_per_thread *pt = &context->pt[tsi];
	int n, m, c;
	char buf;

	/* stay dead once we are dead */

	if (!context)
		return 1;

	lws_libev_run(context, tsi);
	lws_libuv_run(context, tsi);

	if (!context->service_tid_detected) {
		struct lws _lws;

		memset(&_lws, 0, sizeof(_lws));
		_lws.context = context;

		context->service_tid_detected = context->vhost_list->
			protocols[0].callback(&_lws, LWS_CALLBACK_GET_THREAD_ID,
						  NULL, NULL, 0);
	}
	context->service_tid = context->service_tid_detected;

	timeout_ms = lws_service_adjust_timeout(context, timeout_ms, tsi);

	n = lws_poll(pt->fds, pt->fds_count, timeout_ms);

#ifdef LWS_OPENSSL_SUPPORT
	if (!pt->rx_draining_ext_list &&
	    !lws_ssl_anybody_has_buffered_read_tsi(context, tsi) && !n) {
#else
	if (!pt->rx_draining_ext_list && !n) /* poll timeout */ {
#endif
		lws_service_fd_tsi(context, NULL, tsi);
		return 0;
	}

	m = lws_service_flag_pending(context, tsi);
	if (m)
		c = -1; /* unknown limit */
	else
		if (n < 0) {
			if (LWS_ERRNO != LWS_EINTR)
				return -1;
			return 0;
		} else
			c = n;

	/* any socket with events to service? */
	for (n = 0; n < pt->fds_count && c; n++) {
		if (!pt->fds[n].revents)
			continue;

		c--;

		if (pt->fds[n].fd == pt->dummy_pipe_fds[0]) {
			if (read(pt->fds[n].fd, &buf, 1) != 1)
				lwsl_err("Cannot read from dummy pipe.");
			continue;
		}

		m = lws_service_fd_tsi(context, &pt->fds[n], tsi);
		if (m < 0)
			return -1;
		/* if something closed, retry this slot */
		if (m)
			n--;
	}

	return 0;
}

LWS_VISIBLE int
lws_plat_service(struct lws_context *context, int timeout_ms)
{
	return lws_plat_service_tsi(context, timeout_ms, 0);
}

LWS_VISIBLE void
lws_plat_drop_app_privileges(struct lws_context_creation_info *info)
{
}

LWS_VISIBLE int
lws_plat_context_early_init(void)
{
	return 0;
}

LWS_VISIBLE void
lws_plat_context_early_destroy(struct lws_context *context)
{
}

LWS_VISIBLE void
lws_plat_context_late_destroy(struct lws_context *context)
{
	struct lws_context_per_thread *pt = &context->pt[0];
	int m = context->count_threads;

	if (context->lws_lookup)
		lws_free(context->lws_lookup);

	while (m--) {
		close(pt->dummy_pipe_fds[0]);
		close(pt->dummy_pipe_fds[1]);
		pt++;
	}
	close(context->fd_random);
}

/* cast a struct sockaddr_in6 * into addr for ipv6 */

LWS_VISIBLE int
lws_interface_to_sa(int ipv6, const char *ifname, struct sockaddr_in *addr,
		    size_t addrlen)
{
	return -1;
}

LWS_VISIBLE void
lws_plat_insert_socket_into_fds(struct lws_context *context, struct lws *wsi)
{
	struct lws_context_per_thread *pt = &context->pt[(int)wsi->tsi];

	lws_libev_io(wsi, LWS_EV_START | LWS_EV_READ);
	lws_libuv_io(wsi, LWS_EV_START | LWS_EV_READ);

	pt->fds[pt->fds_count++].revents = 0;
}

LWS_VISIBLE void
lws_plat_delete_socket_from_fds(struct lws_context *context,
						struct lws *wsi, int m)
{
	struct lws_context_per_thread *pt = &context->pt[(int)wsi->tsi];
	pt->fds_count--;
}

LWS_VISIBLE void
lws_plat_service_periodic(struct lws_context *context)
{
}

LWS_VISIBLE int
lws_plat_change_pollfd(struct lws_context *context,
		      struct lws *wsi, struct lws_pollfd *pfd)
{
	return 0;
}

LWS_VISIBLE const char *
lws_plat_inet_ntop(int af, const void *src, char *dst, int cnt)
{
	return 0;
}

#if defined(IMPLEMENT_FILESYSTEM) 

static lws_filefd_type
_lws_plat_file_open(struct lws *wsi, const char *filename,
		    unsigned long *filelen, int flags)
{
	struct stat stat_buf;
	int ret = open(filename, flags, 0664);

	if (ret < 0)
		return LWS_INVALID_FILE;

	if (fstat(ret, &stat_buf) < 0) {
		close(ret);
		return LWS_INVALID_FILE;
	}
	*filelen = stat_buf.st_size;
	return ret;
}

static int
_lws_plat_file_close(struct lws *wsi, lws_filefd_type fd)
{
	return close(fd);
}

unsigned long
_lws_plat_file_seek_cur(struct lws *wsi, lws_filefd_type fd, long offset)
{
	return lseek(fd, offset, SEEK_CUR);
}

static int
_lws_plat_file_read(struct lws *wsi, lws_filefd_type fd, unsigned long *amount,
		    unsigned char *buf, unsigned long len)
{
	long n;

	n = read((int)fd, buf, len);
	if (n == -1) {
		*amount = 0;
		return -1;
	}

	*amount = n;

	return 0;
}

static int
_lws_plat_file_write(struct lws *wsi, lws_filefd_type fd, unsigned long *amount,
		     unsigned char *buf, unsigned long len)
{
	long n;

	n = write((int)fd, buf, len);
	if (n == -1) {
		*amount = 0;
		return -1;
	}

	*amount = n;

	return 0;
}

#endif // #if defined(IMPLEMENT_FILESYSTEM) 

LWS_VISIBLE int
lws_plat_init(struct lws_context *context,
	      struct lws_context_creation_info *info)
{
	struct lws_context_per_thread *pt = &context->pt[0];
	int n = context->count_threads, fd;

	/* master context has the global fd lookup array */
	context->lws_lookup = lws_zalloc(sizeof(struct lws *) *
					 context->max_fds);
	if (context->lws_lookup == NULL) {
		lwsl_err("OOM on lws_lookup array for %d connections\n",
			 context->max_fds);
		return 1;
	}

	lwsl_notice(" mem: platform fd map: %5u bytes\n",
		    (unsigned int)(sizeof(struct lws *) * context->max_fds));
	fd = open(SYSTEM_RANDOM_FILEPATH, O_RDONLY);

	context->fd_random = fd;
	if (context->fd_random < 0) {
		lwsl_err("Unable to open random device %s %d\n",
			 SYSTEM_RANDOM_FILEPATH, context->fd_random);
		return 1;
	}

	if (!lws_libev_init_fd_table(context) &&
	    !lws_libuv_init_fd_table(context)) {
		/* otherwise libev handled it instead */

		while (n--) {
			/* use the read end of pipe as first item */
			pt->fds[0].fd = pt->dummy_pipe_fds[0];
			pt->fds[0].events = LWS_POLLIN;
			pt->fds[0].revents = 0;
			pt->fds_count = 1;
			pt++;
		}
	}

#if defined(IMPLEMENT_FILESYSTEM) 
	context->fops.open	= _lws_plat_file_open;
	context->fops.close	= _lws_plat_file_close;
	context->fops.seek_cur	= _lws_plat_file_seek_cur;
	context->fops.read	= _lws_plat_file_read;
	context->fops.write	= _lws_plat_file_write;
#endif // #if defined(IMPLEMENT_FILESYSTEM) 

	return 0;
}

void lws_freeaddrinfo(struct lws_addrinfo *res)
{
	lws_free(res);
}
