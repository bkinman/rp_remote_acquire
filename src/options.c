/*
 * options.c
 *
 *  Created on: 17 Nov 2014
 *      Author: bkinman, nils
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "options.h"

/******************************************************************************
 * static variables
 ******************************************************************************/
static struct option g_long_options[] =
{
    {"address",            required_argument, NULL, 'a'},
    {"port",               required_argument, NULL, 'p'},
    {"mode",               required_argument, NULL, 'm'},
    {"udp",                no_argument,       NULL, 'u'},
    {"kbytes_to_transfer", required_argument, NULL, 'k'},
    {"report-rate",        no_argument,       NULL, 'r'},
    {"help",               no_argument,       NULL, 'h'},
    {"scope-channel",      required_argument, NULL, 'c'},
    {"scope-decimation",   required_argument, NULL, 'd'},
    {"scope-HV",           no_argument,       NULL, 1  },
    {"scope-no-equalizer", no_argument,       NULL, 'e'},
    {"scope-no-shaping",   no_argument,       NULL, 's'},
    {NULL, 0, NULL, 0}
};

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
int handle_options(int argc, char *argv[], option_fields_t *options)
{
    int ch;
    int mode;

    while ((ch = getopt_long(argc, argv, "a:p:m:uk:rhc:d:es", g_long_options, NULL)) != -1)
    {
        // check to see if a single character or long option came through
        switch (ch)
        {
            case 'a': //Address
                strncpy(options->address, optarg ,sizeof(options->address));
                options->address[sizeof(options->address)-1]= '\0';
                break;
            case 'p': //Port
                options->port = atoi(optarg);
                break;
            case 'm': //Mode
                mode = atoi(optarg);
                if (!strcmp("client",optarg) || mode == 1)
                    options->mode = client;
                else if (!strcmp("server",optarg) || mode == 2)
                    options->mode = server;
                else if (!strcmp("file",optarg) || mode == 3)
                    options->mode = file;
                else if (!strcmp("on_the_fly_test",optarg) || mode == 4)
                    options->mode = on_the_fly_test;
                break;
            case 'u': //udp
                options->tcp = 0;
                break;
            case 'k': //number of kbytes to transfer
                options->kbytes_to_transfer = atol(optarg);
                break;
            case 'r': // report-rate
                options->report_rate = 1;
                break;
            case 'c': // Channel
                if (optarg[0] == 'A' || optarg[0] == 'a')
                    options->scope_chn = 0;
                else if (optarg[0] == 'B' || optarg[0] == 'b')
                    options->scope_chn = 1;
                else
                    options->scope_chn = (atoi(optarg) == 0) ? 0 : 1;
                break;
            case 'd': // Decimation
                options->scope_dec = atoi(optarg);
                break;
            case 1: // enable HV input settings
                options->scope_hv = 1;
                break;
            case 'e': // disable equalization filter
                options->scope_equalizer = 0;
                break;
            case 's': // disable shaping filter
                options->scope_shaping = 0;
                break;
            case '?':
            case 'h':
            default:
                return 1;
        }
    }

    return check_options(options);
}

int check_options(option_fields_t *options)
{
    if(options->mode == client || options->mode == server)
    {
	    if (options->port <1) {
		    fprintf(stderr,"Port number invalid, or not selected (use -p)\n");
		    return 1;
	    }
    }

    if(options->mode == client)
    {
		if(0 == strncmp(options->address, "", sizeof(options->address)) )
		{
			fprintf(stderr,"No ip address provided (use -a)\n");
			return 1;
		}
    }
    else
    {
    	strcpy(options->address,"127.0.0.1");
    }

    return 0;
}

void usage(void)
{
    printf("Usage:  rp_remote_acquire [-apmukrhcdes]\n");
    printf(
             "  -a  --address ip_address target address in client mode\n"
             "  -p  --port port_num port number\n"
             "  -m  --mode ((1|client)|(2|server)|(3|file)|(4|on_the_fly_test))\n"
             "  -u  --udp indicates tool should use udp mode\n"
             "  -k  --kbytes_to_transfer num_kbytes number of kilobytes to transfer\n"
             "  -r  --report-rate turn on rate reporting\n"
             "  -h  --help Display this usage information\n"
             "  -c  --scope-channel channel ((0|A)|(1|B))\n"
             "  -d  --scope-decimation decimation 0,1,2,4,..,65536\n"
             "  -e  --scope-no-equalizer disable equalization filter\n"
             "      --scope-HV enable HV equalizer settings\n"
             "  -s  --scope-no-shaping disable shaping filter\n"
           "\n");
    printf("Examples:\n");
    printf("\t Insert example here\n");
}
