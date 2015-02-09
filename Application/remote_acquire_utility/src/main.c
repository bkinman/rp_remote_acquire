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


const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya Remote Acquisition Utility.\n";
}

int rp_app_init(void)
{
    fprintf(stderr, "Loading Remote Acquisition Utility version %s-%s.\n", VERSION_STR, REVISION_STR);

    return 0;
}

int rp_app_exit(void)
{
    fprintf(stderr, "Unloading Remote Acuiqisition Utility version %s-%s.\n", VERSION_STR, REVISION_STR);

    return 0;
}

int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
	return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}
