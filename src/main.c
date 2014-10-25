/*
 * main.c
 *
 *  Created on: 7 Jun 2014
 *      Author: nils
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"

static void parseArgs(int argv,char **argc);

static int          mbTCP = 0;
static char        *mszRemoteAddr = "192.168.10.1";
static int          miRemotePort = 14000;
static unsigned int muBufSize = 16384;
static unsigned int muKiloBytes = 500000;


int main(int argv,char **argc) {
  int trc = 0;

  parseArgs(argv,argc);

  if( test_setup(mbTCP,mszRemoteAddr,miRemotePort,muBufSize,muKiloBytes) != 0 ) {
    return -1;
  }

  trc = test_test();

  printf("rc: %d\n",trc);

  test_cleanup();

  return 0;
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


