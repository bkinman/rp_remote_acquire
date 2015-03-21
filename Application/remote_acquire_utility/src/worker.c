/*
 * worker.c
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

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "transfer.h"
#include "worker.h"

static void *acq_worker_thread(void *arg);


int acq_worker_init(struct worker_state *worker)
{
	int ret_val;

	if (pthread_mutex_lock(&worker->mux) < 0)
		return -1;

	if (worker->state == running) {
		ret_val = 0;
		goto unlock_mux;
	}

	if (pthread_create(&worker->thread_handle, NULL, acq_worker_thread, worker) < 0) {
		fprintf(stderr, "couldn't start worker thread, %s\n", strerror(errno));
		ret_val = -1;
		goto unlock_mux;
	}

	worker->state = running;
	ret_val = 0;

unlock_mux:
	pthread_mutex_unlock(&worker->mux);

	return ret_val;
}

int acq_worker_exit(struct worker_state *worker)
{
	if (pthread_mutex_lock(&worker->mux) < 0)
		return -1;

	if (worker->state == dead) {
		pthread_mutex_unlock(&worker->mux);
		return 0;
	}

	if (worker->state == running) {
		if (pthread_cancel(worker->thread_handle) < 0) {
			fprintf(stderr, "couldn't cancel worker thread, %s\n", strerror(errno));
			goto error_unlock;
		}
		worker->state = cancelled;
	}

	pthread_mutex_unlock(&worker->mux);

	if (pthread_join(worker->thread_handle, NULL)) {
		fprintf(stderr, "couldn't join worker thread, %s\n", strerror(errno));
		goto error;
	}

	worker->state = dead; /* the worker sets itself dead, but better be sure */

	return 0;

error_unlock:
	pthread_mutex_unlock(&worker->mux);
error:
	return -1;
}

static void acq_worker_cleaner1(void *arg)
{
	struct worker_state *worker = (struct worker_state *)arg;

	signal_exit();

	if (pthread_mutex_lock(&worker->mux) < 0)
		return;
	worker->state = dead;
	pthread_mutex_unlock(&worker->mux);
}

static void acq_worker_cleaner2(void *arg)
{
	struct scope_parameter *param = (struct scope_parameter *)arg;

	connection_cleanup();

	scope_cleanup(param);
}

static void *acq_worker_thread(void *arg)
{
	struct worker_state *worker = (struct worker_state *)arg;
	struct scope_parameter param;
	int sock_fd = -1;
	int ret_val;

	pthread_cleanup_push(acq_worker_cleaner1, (void *)worker);

	signal_init();

	if (scope_init(&param, worker->options)) {
		fprintf(stderr, "scope_init failed, %s\n", strerror(errno));
		goto error_dead;
	}

	pthread_cleanup_push(acq_worker_cleaner2, (void *)&param);

	if (connection_init(worker->options))
		goto error_cleanup;

	while (!transfer_interrupted()) {
		if ((sock_fd = connection_start(worker->options)) < 0) {
			fprintf(stderr, "connection_start failed, %s\n", strerror(errno));
			continue;
		}

		ret_val = transfer_data(sock_fd, &param, worker->options);
		if (ret_val && !transfer_interrupted())
			fprintf(stderr, "error during transfer_data, %s\n", strerror(errno));

		connection_stop();
	}

error_cleanup:
	pthread_cleanup_pop(1);

error_dead:
	pthread_cleanup_pop(1);

	return NULL;
}
