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

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

static int send_buffer(int sock_fd, const void *buf, size_t len);

int connection_init(int tcp, const char *ip_addr, int ip_port)
{
	int sock_fd;
	struct sockaddr_in server;
	int rc;

	printf("create socket for %s\n", tcp ? "TCP" : "UDP");

	sock_fd = socket(PF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		printf("ERR: socket %d\n", errno);
		goto error;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_port = htons(ip_port);

	printf("connecting to %s:%i\n", ip_addr, ip_port);

	rc = connect(sock_fd, (struct sockaddr *)&server, sizeof(server));
	if (rc < 0) {
		printf("ERR: connect %d\n", errno);
		goto error_close;
	}

	return sock_fd;

error_close:
	close(sock_fd);
error:
	return -1;
}

int transfer_data(int sock_fd, int rpad_fd, size_t size, int report_rate)
{
	int retval = 0;
	size_t transferred = 0;
	unsigned long long duration = 0ULL;
	struct timeval start_time, end_time;
	int block_length;
	char buffer[16384];

	if (report_rate && gettimeofday(&start_time, NULL))
		report_rate = 0;

	while (!size || transferred < size) {
		if (!size)
			block_length = sizeof(buffer);
		else
			block_length = min(sizeof(buffer), size - transferred);

		block_length = read(rpad_fd, buffer, block_length);
		if (block_length < 0) {
			printf("ERR: read %d\n", errno);
			retval = -1;
			break;
		}

		if (send_buffer(sock_fd, buffer, block_length) < 0) {
			printf("ERR: write %d\n", errno);
			retval = -1;
			break;
		}

		transferred += block_length;
	}

	if (report_rate && gettimeofday(&end_time, NULL)) {
		duration = (unsigned long long)(end_time.tv_sec - start_time.tv_sec) * 1000ULL
			 + (unsigned long long)end_time.tv_usec   / 1000ULL
			 - (unsigned long long)start_time.tv_usec / 1000ULL;
		printf("transferred %luB in %llums (rate %uMB/s)\n",
		       transferred, duration,
		       (unsigned int)((1000ULL * transferred) / (duration * 1024 * 1024)));
	}

	return retval;
}

static int send_buffer(int sock_fd, const void *buf, size_t len)
{
	int retval = 0;
	int sent;
	unsigned int pos;

	for (pos = 0; pos < len; pos += sent) {
		sent = send(sock_fd, buf + pos, len - pos, 0);
		if (sent < 0) {
			retval = -1;
			break;
		}
	}

	return retval;
}
