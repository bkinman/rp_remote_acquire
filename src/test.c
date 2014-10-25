/*
 * test.c
 *
 *  Created on: 9 Jun 2014
 *      Author: nils
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

static int sendBuffer(int iSockFd,const void *pBuffer,unsigned int uLen);

static int          miSockFd = -1;
static void        *mpBuffer1 = NULL;
//static void        *mpBuffer2 = NULL;
//static unsigned int muBufSize;
//static unsigned int muBufCount;

static int rpfd = -1;

int test_setup(int bTCP,const char *szRemoteAddr,int iRemotePort,unsigned int uBufSize,unsigned int uKiloBytes) {
  int rc = 0;
  struct sockaddr_in server;
  int trc;

  printf("create socket for %s\n",bTCP ? "TCP" : "UDP");

  miSockFd = socket(PF_INET,bTCP ? SOCK_STREAM : SOCK_DGRAM,0);
  if( miSockFd < 0 ) {
    printf("ERR: socket rc:%d errno %d\n",miSockFd,errno);
    rc = -1;
    goto test_setup_exit;
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(szRemoteAddr);
  server.sin_port = htons(iRemotePort);

  printf("connect to %s:%i\n",szRemoteAddr,iRemotePort);

  trc = connect(miSockFd,(struct sockaddr *)&server,sizeof(server));
  if( trc < 0 ) {
    printf("ERR: connect rc:%d errno %d\n",trc,errno);
    rc = -1;
    goto test_setup_exit;
  }

  rpfd = open("/dev/rpad_scope0", O_RDWR);
  if( rpfd < 0 ){
	    printf("ERR: open scope\n");
	    rc = -1;
	    goto test_setup_exit;
  }

  mpBuffer1 = mmap(NULL, 0x00100000UL, PROT_WRITE | PROT_READ, MAP_SHARED,
		   rpfd, 0x40100000UL);
  if( mpBuffer1 == MAP_FAILED ){
	    printf("ERR: mmap rpad_scope0\n");
	    mpBuffer1 = NULL;
	    rc = -1;
	    goto test_setup_exit;
  }

test_setup_exit:
  return rc;
}


int test_test() {
  int rc = 0;
  unsigned int i;
  int drc,trc;
  struct timeval tvStart,tvEnd;
  unsigned long long duration = 0ULL;
  char buffer[16384];
  int bytes = 0;

  printf("spam");

  drc = gettimeofday(&tvStart,NULL);

  for (i = 0; i < 1024; i++) {
    trc = read(rpfd, buffer, sizeof(buffer));
    if (trc < 0) {
      printf("ERR: read rc:%d errno %d\n",trc,errno);
      rc = -1;
      goto test_test_exit;
    }
    bytes += trc;
    trc = sendBuffer(miSockFd, buffer, trc);
    if( trc < 0 ) {
      printf("ERR: test_sendBuffer rc:%d errno %d\n",trc,errno);
      rc = -1;
      goto test_test_exit;
    }
  }

  if( drc == 0 ) {
    drc = gettimeofday(&tvEnd,NULL);
  }

  printf(" done\n");

  if( drc == 0 ) {
    duration = (unsigned long long)(tvEnd.tv_sec - tvStart.tv_sec) * 1000ULL
             + (unsigned long long)tvEnd.tv_usec   / 1000ULL
             - (unsigned long long)tvStart.tv_usec / 1000ULL;
    printf("duration: %llu\n",duration);
    printf("rate: %lluMB/s\n",(1000ULL * bytes) / (duration * 1024 * 1024));
  }

test_test_exit:
  return rc;
}


void test_cleanup() {
  printf("cleanup\n");

  if( miSockFd >= 0 ) {
    close(miSockFd);
    miSockFd = -1;
  }
  if(mpBuffer1 != NULL) {
    munmap(mpBuffer1, 0x00100000UL);
    mpBuffer1 = NULL;
  }
  if( rpfd >= 0 ) {
    close(rpfd);
    rpfd = -1;
  }
}


static int sendBuffer(int iSockFd,const void *pBuffer,unsigned int uLen) {
  int rc = 0;
  int trc;
  unsigned int pos;

  for( pos = 0; pos < uLen; pos += trc ) {
    trc = send(iSockFd,pBuffer + pos,uLen - pos,0);
    if( trc < 0 ) {
      rc = -1;
      break;
    }
  }

  return rc;
}


//int test_setup(int bTCP,const char *szRemoteAddr,int iRemotePort,unsigned int uBufSize,unsigned int uKiloBytes) {
//  int rc = 0;
//  struct sockaddr_in server;
//  int trc;
//
//  if( uBufSize < 1024 ) {
//    muBufSize = 1024;
//  } else {
//    muBufSize = uBufSize;
//  }
//  muBufCount = uKiloBytes / (muBufSize / 1024);
//
//  printf("setup for %u blocks of size %u\n",muBufCount,muBufSize);
//
//  mpBuffer1 = malloc(muBufSize);
//  mpBuffer2 = malloc(muBufSize);
//  if( mpBuffer1 == NULL || mpBuffer2 == NULL ) {
//    printf("ERR: malloc failed, errno %d\n",errno);
//    rc = -1;
//    goto test_setup_exit;
//  }
//
//  printf("create socket for %s\n",bTCP ? "TCP" : "UDP");
//
//  miSockFd = socket(PF_INET,bTCP ? SOCK_STREAM : SOCK_DGRAM,0);
//  if( miSockFd < 0 ) {
//    printf("ERR: socket rc:%d errno %d\n",miSockFd,errno);
//    rc = -1;
//    goto test_setup_exit;
//  }
//
//  server.sin_family = AF_INET;
//  server.sin_addr.s_addr = inet_addr(szRemoteAddr);
//  server.sin_port = htons(iRemotePort);
//
//  printf("connect to %s:%i\n",szRemoteAddr,iRemotePort);
//
//  trc = connect(miSockFd,(struct sockaddr *)&server,sizeof(server));
//  if( trc < 0 ) {
//    printf("ERR: connect rc:%d errno %d\n",trc,errno);
//    rc = -1;
//    goto test_setup_exit;
//  }
//
//test_setup_exit:
//  return rc;
//}


//int test_test() {
//  int rc = 0;
//  unsigned int i;
//  int drc,trc;
//  struct timeval tvStart,tvEnd;
//  unsigned long long duration = 0ULL;
//
//  printf("spam");
//
//  drc = gettimeofday(&tvStart,NULL);
//
//  for( i = 0; i < muBufCount; i++ ) {
//    //printf(".");
//    trc = sendBuffer(miSockFd,(i & 1) ? mpBuffer2 : mpBuffer1,muBufSize);
//    if( trc < 0 ) {
//      printf("\n");
//      printf("ERR: test_sendBuffer rc:%d errno %d\n",trc,errno);
//      rc = -1;
//      goto test_test_exit;
//    }
//  }
//
//  if( drc == 0 ) {
//    drc = gettimeofday(&tvEnd,NULL);
//  }
//
//  printf(" done\n");
//
//  if( drc == 0 ) {
//    duration = (unsigned long long)(tvEnd.tv_sec - tvStart.tv_sec) * 1000ULL
//             + (unsigned long long)tvEnd.tv_usec   / 1000ULL
//             - (unsigned long long)tvStart.tv_usec / 1000ULL;
//    printf("duration: %llu\n",duration);
//    printf("rate: %lluMB/s\n",(1000ULL * muBufCount * muBufSize) / (duration * 1024 * 1024));
//  }
//
//test_test_exit:
//  return rc;
//}


//void test_cleanup() {
//  printf("cleanup\n");
//
//  if( mpBuffer1 != NULL ) {
//    free(mpBuffer1);
//    mpBuffer1 = NULL;
//  }
//  if( mpBuffer2 != NULL ) {
//    free(mpBuffer2);
//    mpBuffer2 = NULL;
//  }
//  if( miSockFd >= 0 ) {
//    close(miSockFd);
//    miSockFd = -1;
//  }
//  if(mpBuffer1 != NULL) {
//		  munmap(mpBuffer1, muBufSize);
//  }
//  if( rpfd >= 0 ) {
//    close(rpfd);
//    rpfd = -1;
//  }
//
//}


