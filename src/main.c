/*
 * main.c
 *
 *  Created on: 7 Jun 2014
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
#include <stdio.h>
#include <string.h>

#include "scope.h"
#include "transfer.h"

static void parseArgs(int argv, char **argc);

static int	tcp = 0;
static char	*ip_addr = "192.168.10.1";
static int	ip_port = 14000;
static size_t	size = 1024 * 1024;

int main(int argv, char **argc)
{
	int retval;
	int sock_fd, rpad_fd;
	void *mapped_io;

	parseArgs(argv,argc); /* TODO getopt all the parameters */

	if ((rpad_fd = scope_init(&mapped_io)) < 0)
		return -1;

	/* TODO set up scope with acquisition parameters */

	if ((sock_fd = connection_init(tcp, ip_addr, ip_port)) < 0) {
		retval = -1;
		goto cleanup;
	}

	retval = transfer_data(sock_fd, rpad_fd, size, 1);

cleanup:
	scope_cleanup(rpad_fd, mapped_io);
	close(sock_fd);

	return retval;
}

static void parseArgs(int argv,char **argc) {
  unsigned int i;

  for( i = 1; i < argv; i++ ) {
    if( argc[i] != NULL ) {
      if(        strstr(argc[i],"-a") != NULL && i + 1 < argv && argc[i + 1] != NULL ) {
        mszRemoteAddr = argc[++i];
      } else if( strstr(argc[i],"-p") != NULL && i + 1 < argv && argc[i + 1] != NULL ) {
        miRemotePort = atoi(argc[++i]);
      } else if( strstr(argc[i],"-b") != NULL && i + 1 < argv && argc[i + 1] != NULL ) {
        muBufSize = (unsigned int)atoi(argc[++i]);
      } else if( strstr(argc[i],"-k") != NULL && i + 1 < argv && argc[i + 1] != NULL ) {
        muKiloBytes = (unsigned int)atoi(argc[++i]);
      } else if( strstr(argc[i],"-u") != NULL ) {
        mbTCP = 0;
      } else if( strstr(argc[i],"-t") != NULL ) {
        mbTCP = 1;
      }
    }
  }
}
