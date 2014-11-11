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
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>

#include "scope.h"
#include "transfer.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Typedefs
 ******************************************************************************/
static struct option g_long_options[] =
{
    {"address",            optional_argument, NULL, 'a'},
    {"port",               optional_argument, NULL, 'p'},
    {"mode",			   optional_argument, NULL, 'm'},
    {"udp",			       no_argument,       NULL, 'u'},
    {"kbytes_to_transfer", optional_argument, NULL, 'k'},
    {"help",               no_argument,       NULL, 'h'},
    {NULL, 0, NULL, 0}
};

struct option_fields_
{
    char address[16];
    int port;
    int is_client;
    size_t kbytes_to_transfer;
    uint8_t tcp;
};
typedef struct option_fields_ option_fields_t;

/******************************************************************************
 * static function prototypes
 ******************************************************************************/
static int handle_options(int argc, char *argv[]);
static int check_options(void);
static void usage(void);

/******************************************************************************
 * static variables
 ******************************************************************************/

static option_fields_t g_options =
{
		/* Setting defaults */
		.address = "",
		.port = 14000,
		.is_client = 1,
		.kbytes_to_transfer = 0,
		.tcp = 1,
};

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
int main(int argc, char **argv)
{
	int retval;
	int sock_fd, rpad_fd;
	void *mapped_io;

    if(0 != handle_options(argc,argv))
    {
        usage();
        exit(1);
    }


	if ((rpad_fd = scope_init(&mapped_io)) < 0)
		return -1;

	/* TODO set up scope with acquisition parameters */

	if ((sock_fd = connection_init(g_options.tcp, g_options.is_client, g_options.address, g_options.port)) < 0) {
		retval = -1;
		goto cleanup;
	}

	retval = transfer_data(sock_fd, rpad_fd, g_options.kbytes_to_transfer*1024, 1);
	connection_cleanup(sock_fd);

cleanup:
	scope_cleanup(rpad_fd, mapped_io);

	return retval;
}

/******************************************************************************
 * static function definitions
 ******************************************************************************/
static int handle_options(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt_long(argc, argv, "a:p:uk:h", g_long_options, NULL)) != -1)
    {
        // check to see if a single character or long option came through
        switch (ch)
        {
            case 'a': //Address
                strncpy(g_options.address, optarg ,sizeof(g_options.address));
                g_options.address[sizeof(g_options.address)-1]= '\0';
                break;
            case 'p': //Port
                g_options.port = atoi(optarg);
                break;
            case 'm': //Mode
            	g_options.is_client = strcmp("client",optarg)?0:1;
            	break;
            case 'u': //udp
            	g_options.tcp = 0;
                break;
            case 'k': //number of bytes to transfer
            	g_options.kbytes_to_transfer = atoll(optarg);
            	break;
            case '?':
            case 'h':
                usage();
                break;
        }
    }

    return check_options();
}

static int check_options(void)
{
    if(g_options.port <1)
    {
    	fprintf(stderr,"Port number invalid, or not selected (use -p)\n");
    	return 1;
    }

    if(g_options.is_client)
    {
		if(0 == strncmp(g_options.address, "", sizeof(g_options.address)) )
		{
			fprintf(stderr,"No ip address provided (use -a)\n");
			return 1;
		}
    }
    else
    {
    	strcpy(g_options.address,"127.0.0.1");
    }

    return 0;
}

static void usage(void)
{
    printf("Usage:  rp_remote_acquire [-habpu]\n");
    printf(
             "  -h  --help Display this usage information\n"
             "  -a  --address [ip_address]\n"
             "  -b  --bytes_to_transfer [num_bytes] number of bytes to transfer\n"
    		 "  -p  --port [port_num] port number\b"
             "  -u  --udp indicates tool should use udp mode\n"
           "\n");
    printf("Examples:\n");
    printf("\t Insert example here\n");
}
