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

int scope_init(void **mapped_io)
{
	int rpad_fd;

	rpad_fd = open("/dev/rpad_scope0", O_RDWR);
	if (rpad_fd < 0) {
		fprintf(stderr, "open scope failed, %d\n", errno);
		return -1;
	}

	*mapped_io = mmap(NULL, 0x00100000UL, PROT_WRITE | PROT_READ,
	                  MAP_SHARED, rpad_fd, 0x40100000UL);
	if (*mapped_io == MAP_FAILED) {
		fprintf(stderr, "mmap scope failed (non-fatal), %d\n", errno);
		*mapped_io = NULL;
	}

	return rpad_fd;
}

void scope_cleanup(int rpad_fd, void *mapped_io)
{
	if (mapped_io)
		munmap(mapped_io, 0x00100000UL);

	close(rpad_fd);
}
