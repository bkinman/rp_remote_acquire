/*
 * worker.h
 *
 *  Created on: 20 Mar 2015
 *      Author: nils
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 bkinman, Nils Roos
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

#ifndef WORKER_H_
#define WORKER_H_

#include <pthread.h>

#include "options.h"

struct worker_state {
	pthread_mutex_t mux;
	pthread_t       thread_handle;
	enum {running, cancelled, dead} state;
	struct option_fields_ *options;
};

int acq_worker_init(struct worker_state *);
int acq_worker_exit(struct worker_state *);

#endif /* WORKER_H_ */
