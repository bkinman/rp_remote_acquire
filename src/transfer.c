/*
 * transfer.c
 *
 *  Created on: 9 Jun 2014
 *      Author: nils
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 bkinman, Nils Roos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* _GNU_SOURCE provides splice() and vmsplice(), which are Linux kernel
 * specific portions of GNU's libc runtime.
 */
#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/uio.h>

#include "options.h"
#include "scope.h"

#include <stdlib.h>
#include <string.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define min(x, y) (((x) < (y)) ? (x) : (y))

/******************************************************************************
 * Typedefs
 ******************************************************************************/

/******************************************************************************
 * static function prototypes
 ******************************************************************************/
static void int_handler(int sig);
static u_int64_t transfer_readwrite(int sock_fd, struct scope_parameter *param,
                                    option_fields_t *options);
static u_int64_t transfer_mmap(int sock_fd, struct scope_parameter *param,
                               option_fields_t *options);
static u_int64_t transfer_mmapfile(struct scope_parameter *param,
                                   option_fields_t *options);
static u_int64_t transfer_mmappipe(int sock_fd, struct scope_parameter *param,
                                   option_fields_t *options);
static int send_buffer(int sock_fd, const char *buf, size_t len);

/******************************************************************************
 * static variables
 ******************************************************************************/
static volatile int interrupted = 0;
static int sa_set = 0;
static struct sigaction oldsa;
static struct sigaction sa = {
	.sa_handler = int_handler,
};

static int sock_fd = -1;
static int client_sock_fd = -1;

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
int connection_init(option_fields_t *options)
{
	struct sockaddr_in server, cli_addr;
	int rc;

	printf("Creating %s %s\n", options->tcp ? "TCP" : "UDP",
	       (options->mode == client || options->mode == c_pipe) ? "CLIENT"
	                                                            : "SERVER");

	sock_fd = socket(PF_INET, options->tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

	if (sock_fd < 0) {
		fprintf(stderr, "create socket failed, %s\n", strerror(errno));
		goto error;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(options->address);
	server.sin_port = htons(options->port);

	if (options->mode == client || options->mode == c_pipe) {
		printf("connecting to %s:%i\n", options->address, options->port);

		rc = connect(sock_fd, (struct sockaddr *)&server, sizeof(server));
		if (rc < 0)
		{
			fprintf(stderr, "connect failed, %s\n", strerror(errno));
			goto error_close;
		}
	}
	else
	{
		 socklen_t clilen;
		 server.sin_addr.s_addr = INADDR_ANY;
	     if (bind(sock_fd, (struct sockaddr *) &server, sizeof(server)) < 0)
	     {
	    	 fprintf(stderr, "bind failed, %s\n",strerror(errno));
	         goto error_close;
	     }
	     if(listen(sock_fd,5) < 0)
	     {
	    	 fprintf(stderr, "listen failed, %s\n",strerror(errno));
	    	 goto error_close;
	     }

	     clilen = sizeof(cli_addr);
	     client_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &clilen);
	     if(client_sock_fd < 0)
	     {
	    	 fprintf(stderr, "accept failed, %s\n",strerror(errno));
	    	 goto error_close;
	     }
	}

	if (!sigaction(SIGINT, &sa, &oldsa))
		sa_set = 1;
	else
		fprintf(stderr, "configuring signals failed (non-fatal), %s\n", strerror(errno));

	if (options->mode == client || options->mode == c_pipe)
		return sock_fd;
	else
		return client_sock_fd;

error_close:
	if (sock_fd >= 0)
		close(sock_fd);
	if (client_sock_fd >= 0)
		close(client_sock_fd);
error:
	return -1;
}

void connection_cleanup(int sock_fd)
{
	if (sa_set)
		sigaction(SIGINT, &oldsa, NULL);
	if (sock_fd >= 0)
		close(sock_fd);
	if (client_sock_fd >= 0)
		close(client_sock_fd);
}

int transfer_data(int sock_fd, struct scope_parameter *param,
                  option_fields_t *options)
{
	int retval = 0;
	unsigned long duration = 0UL;
	u_int64_t transferred = 0ULL;
	struct timeval start_time, end_time;
	int report_rate = options->report_rate;

	if (report_rate && gettimeofday(&start_time, NULL))
		report_rate = 0;

	if (options->mode == client || options->mode == server) {
		if (0)
			transferred = transfer_readwrite(sock_fd, param, options);
		else
			transferred = transfer_mmap(sock_fd, param, options);
	} else if (options->mode == file) {
		transferred = transfer_mmapfile(param, options);
	} else if (options->mode == c_pipe || options->mode == s_pipe) {
		if (0)
			transferred = transfer_mmappipe(sock_fd, param, options);
	}

	if (report_rate && !gettimeofday(&end_time, NULL)) {
		duration = 1000UL * (end_time.tv_sec - start_time.tv_sec)
			 + (unsigned long)end_time.tv_usec   / 1000UL
			 - (unsigned long)start_time.tv_usec / 1000UL;
		printf("transferred %lluB in %lums (rate %.2fMB/s)\n",
		       transferred, duration,
		       (double)(1000ULL * transferred) / (1024ULL * 1024ULL * duration));
	}

	return retval;
}

/******************************************************************************
 * static function definitions
 ******************************************************************************/
static void int_handler(int sig)
{
	interrupted = 1;
}

static u_int64_t transfer_readwrite(int sock_fd, struct scope_parameter *param,
                                    option_fields_t *options)
{
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	int block_length;
	char buffer[16384];

	while (!size || transferred < size) {
		if (!size)
			block_length = sizeof(buffer);
		else
			block_length = min(sizeof(buffer), size - transferred);

		block_length = read(param->scope_fd, buffer, block_length);
		if (block_length < 0) {
			if (!interrupted) {
				fprintf(stderr, "rpad read failed, %s\n", strerror(errno));
			}
			break;
		}

		if (send_buffer(sock_fd, buffer, block_length) < 0) {
			if (!interrupted) {
				fprintf(stderr, "socket write failed, %s\n", strerror(errno));
			}
			break;
		}

		transferred += block_length;
	}

	return transferred;
}

static u_int64_t transfer_mmap(int sock_fd, struct scope_parameter *param,
                               option_fields_t *options)
{
	const int CHUNK = 8 * 4096;
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	unsigned long pos;
	size_t len;
	unsigned long curr;
	unsigned long *curr_addr;
	unsigned long base;
	void *mapped_base;
	size_t buf_size;
	void *buf;

	if (!(buf = malloc(CHUNK))) {
		fprintf(stderr, "no memory for temp buffer\n");
		return 0ULL;
	}

	curr_addr = param->mapped_io + (options->scope_chn ? 0x118 : 0x114);
	base = *(unsigned long *)(param->mapped_io +
                                  (options->scope_chn ? 0x10c : 0x104));
	mapped_base = options->scope_chn ? param->mapped_buf_b
	                                 : param->mapped_buf_a;
	buf_size = options->scope_chn ? param->buf_b_size : param->buf_a_size;

	pos = 0;
	while (!size || transferred < size) {
		if (pos == buf_size)
			pos = 0;

		curr = *curr_addr - base;

		if (pos + CHUNK <= curr) {
			len = CHUNK;
		} else if (pos > curr) {
			if (pos + CHUNK <= buf_size) {
				len = CHUNK;
			} else {
				len = buf_size - pos;
			}
		} else {
			continue;
		}

		/* copy to cacheable buffer, shortens socket overhead by ~75% */
		memcpy(buf, mapped_base + pos, len);

		if (send_buffer(sock_fd, buf, len) < 0) {
			if (!interrupted) {
				fprintf(stderr, "socket write failed, %s\n", strerror(errno));
			}
			break;
		}

		pos += len;
		transferred += len;
	}

	free(buf);

	return transferred;
}

static u_int64_t transfer_mmapfile(struct scope_parameter *param,
                                   option_fields_t *options)
{
	const int CHUNK = 8 * 4096;
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	unsigned long pos;
	size_t len;
	unsigned long curr;
	unsigned long *curr_addr;
	unsigned long base;
	void *mapped_base;
	size_t buf_size;
	FILE *f;
	void *buf;

	if (!(buf = malloc(CHUNK))) {
		fprintf(stderr, "no memory for temp buffer\n");
		return 0ULL;
	}

	if (!(f = fopen(options->fname, "wb"))) {
		fprintf(stderr, "file open failed, %s\n", strerror(errno));
		free(buf);
		return 0ULL;
	}

	curr_addr = param->mapped_io + (options->scope_chn ? 0x118 : 0x114);
	base = *(unsigned long *)(param->mapped_io +
                                  (options->scope_chn ? 0x10c : 0x104));
	mapped_base = options->scope_chn ? param->mapped_buf_b
	                                 : param->mapped_buf_a;
	buf_size = options->scope_chn ? param->buf_b_size : param->buf_a_size;

	pos = 0;
	while (!size || transferred < size) {
		if (pos == buf_size)
			pos = 0;

		curr = *curr_addr - base;

		if (pos + CHUNK <= curr) {
			len = CHUNK;
		} else if (pos > curr) {
			if (pos + CHUNK <= buf_size) {
				len = CHUNK;
			} else {
				len = buf_size - pos;
			}
		} else {
			continue;
		}

		/* copy to cacheable buffer, shortens file overhead */
		memcpy(buf, mapped_base + pos, len);

		len = fwrite(buf, 1, len, f);
		if (len <= 0)
			break;

		pos += len;
		transferred += len;
	}

	fclose(f);
	free(buf);

	return transferred;
}

static u_int64_t transfer_mmappipe(int sock_fd, struct scope_parameter *param,
                                   option_fields_t *options)
{
	const int CHUNK = 16 * 4096;
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	unsigned long pos;
	size_t len;
	ssize_t slen;
	unsigned long curr;
	unsigned long *curr_addr;
	unsigned long base;
	void *mapped_base;
	size_t buf_size;
	int pipe_fd[2];
	struct iovec iov;

	curr_addr = param->mapped_io + (options->scope_chn ? 0x118 : 0x114);
	base = *(unsigned long *)(param->mapped_io +
                                  (options->scope_chn ? 0x10c : 0x104));
	mapped_base = options->scope_chn ? param->mapped_buf_b
	                                 : param->mapped_buf_a;
	buf_size = options->scope_chn ? param->buf_b_size : param->buf_a_size;

	if (pipe(pipe_fd)) {
		fprintf(stderr, "create pipe failed, %s\n", strerror(errno));
		return 0ULL;
	}

	pos = 0UL;
	while (!size || transferred < size) {
		if (pos == buf_size)
			pos = 0UL;

		curr = *curr_addr - base;

		if (pos + CHUNK <= curr) {
			len = CHUNK;
		} else if (pos > curr) {
			if (pos + CHUNK <= buf_size) {
				len = CHUNK;
			} else {
				len = buf_size - pos;
			}
		} else {
			continue;
		}

		iov.iov_base = mapped_base + pos;
		iov.iov_len = len;
		slen = vmsplice(pipe_fd[1], &iov, 1, 0/*SPLICE_F_GIFT*/);
		if (slen != len) {
			fprintf(stderr,
			        "vmsplice failed, %d %s, on %p+%lx %d\n",
			        slen, strerror(errno), mapped_base, pos, len);
			break;
		}

		slen = splice(pipe_fd[0], NULL, sock_fd, NULL, len,
		              SPLICE_F_MOVE | SPLICE_F_MORE);
		if (slen != len) {
			fprintf(stderr, "splice failed, %d %s\n", slen, strerror(errno));
			break;
		}

		pos += len;
		transferred += len;
	}

	close(pipe_fd[0]);
	close(pipe_fd[1]);

	return transferred;
}

static int send_buffer(int sock_fd, const char *buf, size_t len)
{
	int retval = 0;
	int sent;
	unsigned int pos;

	for (pos = 0; pos < len; pos += sent) {
		sent = send(sock_fd, buf + pos, len - pos, 0);
		if (sent < 0 || interrupted) {
			retval = -1;
			break;
		}
	}

	return retval;
}
