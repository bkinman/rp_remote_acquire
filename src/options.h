/*
 * options.h
 *
 *  Created on: 17 Nov 2014
 *      Author: bkinman
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

#ifndef OPTIONS_H_
#define OPTIONS_H_

enum mode_e {
	server = 0,
	client,
	file,
	on_the_fly_test,
};

struct option_fields_
{
    char address[16];
    int port;
    int tcp;
    enum mode_e mode;
    size_t kbytes_to_transfer;
    int report_rate;
    int scope_chn;
    int scope_dec;
    int scope_hv;
    int scope_equalizer;
    int scope_shaping;
};
typedef struct option_fields_ option_fields_t;

int handle_options(int argc, char *argv[], option_fields_t *options);
int check_options(option_fields_t *options);
void usage(void);

#endif /* OPTIONS_H_ */
