/*
 * scope.c
 *
 *  Created on: 26 Oct 2014
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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "options.h"
#include "scope.h"

int scope_init(struct scope_parameter *param, option_fields_t *options)
{
	off_t buf_a_addr, buf_b_addr;
	int decimation = options->scope_dec;

	param->channel = options->scope_chn;
	param->scope_fd = open("/dev/rpad_scope0", O_RDWR);
	if (param->scope_fd < 0) {
		fprintf(stderr, "open scope failed, %d\n", errno);
		return -1;
	}

	param->mapped_io = mmap(NULL, 0x00100000UL, PROT_WRITE | PROT_READ,
	                        MAP_SHARED, param->scope_fd, 0x40100000UL);
	if (param->mapped_io == MAP_FAILED) {
		fprintf(stderr, "mmap scope io failed (non-fatal), %d\n",
		        errno);
		param->mapped_io = NULL;
	}

	if (param->channel == 0) {
		/* TODO get phys addr and size from sysfs */
		buf_a_addr = 0x00000000;
		param->buf_a_size = 0x00200000;
		param->mapped_buf_a = mmap(NULL, param->buf_a_size, PROT_READ,
					   MAP_SHARED, param->scope_fd,
					   buf_a_addr);
		if (param->mapped_buf_a == MAP_FAILED) {
			fprintf(stderr,
			        "mmap scope ddr a failed (non-fatal), %d\n",
				errno);
			param->mapped_buf_a = NULL;
		}
	} else {
		/* TODO get phys addr and size from sysfs */
		buf_b_addr = 0x00200000;
		param->buf_b_size = 0x00200000;
		param->mapped_buf_b = mmap(NULL, param->buf_b_size, PROT_READ,
					   MAP_SHARED, param->scope_fd,
					   buf_b_addr);
		if (param->mapped_buf_b == MAP_FAILED) {
			fprintf(stderr,
			        "mmap scope ddr b failed (non-fatal), %d\n",
				errno);
			param->mapped_buf_b = NULL;
		}
	}

	for (param->decimation = 1; decimation; decimation >>= 1)
		param->decimation <<= 1;
	param->decimation >>= 1;

	if (param->mapped_io) {
		// set up scope decimation
		*(unsigned long *)(param->mapped_io + 0x14) = param->decimation;
		if (param->decimation)
			*(unsigned long *)(param->mapped_io + 0x28) = 1;
	}

	return 0;
}

void scope_cleanup(struct scope_parameter *param)
{
	if (param->mapped_io)
		munmap(param->mapped_io, 0x00100000UL);

	if (param->mapped_buf_a)
		munmap(param->mapped_buf_a, param->buf_a_size);

	if (param->mapped_buf_b)
		munmap(param->mapped_buf_b, param->buf_b_size);

	close(param->scope_fd);
}
