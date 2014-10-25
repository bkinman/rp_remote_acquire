/*
 * test.h
 *
 *  Created on: 9 Jun 2014
 *      Author: nils
 */

#ifndef TEST_H_
#define TEST_H_

int test_setup(int bTCP,const char *szRemoteAddr,int iRemotePort,unsigned int uBufSize,unsigned int uKiloBytes);
int test_test();
void test_cleanup();

#endif /* TEST_H_ */
