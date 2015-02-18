#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "main.h"
#include "version.h"


#define LOG(args...) fprintf(stderr, args)


const char *rp_app_desc(void)
{
	LOG("%s: hit\n",__func__);
    return (const char *)"Red Pitaya Remote Acquisition Utility.\n";
}

int rp_app_init(void)
{
	LOG("Loading Remote Acquisition Utility.\n");

    return 0;
}

int rp_app_exit(void)
{
	LOG("Unloading Remote Acquisition Utility.\n");

    return 0;
}

int rp_set_params(rp_app_params_t *p, int len)
{
	LOG("%s was called.\n",__func__);
	LOG("Received %d params.\n",len);

	int i;
	for(i=0;i<len;i++)
	{
		LOG("Name: %s\n",p[i].name);
	}

    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
	LOG("%s was called.\n",__func__);
	return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
	LOG("%s was called.\n",__func__);
    return 0;
}
