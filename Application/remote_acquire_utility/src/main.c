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

/******************************************************************************
 * Defines
 ******************************************************************************/
#define LOG(args...) fprintf(stderr, args)

/******************************************************************************
 * Typedefs
 ******************************************************************************/

/******************************************************************************
 * static function prototypes
 ******************************************************************************/
static int update_global_param_tbl(rp_app_params_t *p, int len);

/******************************************************************************
 * static variables
 ******************************************************************************/
static rp_app_params_t g_rp_param_tbl[] =
{
	{"ip_tup_a",    		 192 , 0, 0, 0, 255 },
	{"ip_tup_b",    		 168 , 0, 0, 0, 255 },
	{"ip_tup_c",    		 3   , 0, 0, 0, 255 },
	{"ip_tup_d",    		 2   , 0, 0, 0, 255 },
	{"port"	   ,    		 6669, 0, 0, 0, 0 },
	{"mode"	   ,    		 2   , 0, 0, 0, 5 }, //Default Mode: Server
	{"b_use_udp",   		 0   , 0, 0, 0, 1 },
	{"bytes_to_transfer", 	 0   , 0, 0, 0, 0 },
	{"b_report_rate", 		 0   , 0, 0, 0, 1 },
	{"scope_channel",        0   , 0, 0, 0, 0 },
	{"scope_decimation",     1024, 0, 0, 0, 0 }, //Default Frequency: 122kSps
	{"b_scope_no_equalizer", 0,    0, 0, 0, 1 },
	{"b_scope_hv",           0,    0, 0, 0, 1 },
	{"scope_no_shaping",     0,    0, 0, 0, 1 },
    { NULL,  		         0.0, -1,-1,0.0,0.0}
};

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
const char *rp_app_desc(void)
{
	LOG("%s: hit\n",__func__);
    return (const char *)"Red Pitaya Remote Acquisition Utility.\n";
}

int rp_app_init(void)
{
	int result;
	LOG("Loading Remote Acquisition Utility.\n");

	LOG("Loading FPGA image: ");
	result = system("cat /opt/www/apps/remote_acquire_utility/fpga.bit > /dev/xdevcfg");
	LOG("%s",(0==result)?"SUCESS\n":"FAIL\n");

	LOG("Loading Kernel Module: ");
	result = system("insmod /opt/www/apps/remote_acquire_utility/rpad.ko");
	LOG("%s",(0==result)?"SUCESS\n":"FAIL\n");

    return 0;
}

int rp_app_exit(void)
{
	LOG("Unloading Remote Acquisition Utility.\n");

    return 0;
}

int rp_set_params(rp_app_params_t *p, int len)
{
	int ret_val;

	LOG("%s was called.\n",__func__);
	LOG("Received %d params.\n",len);

	ret_val = update_global_param_tbl(p,len);
	if(ret_val < 0)
	{
		LOG("Problem updating param table.\n");
		return -1;
	}

	LOG("%s: %d config params were changed.\n",__func__,ret_val);

    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
	LOG("%s was called.\n",__func__);

    rp_app_params_t *p_copy = NULL;
    int i;
    int num_config_params = sizeof(g_rp_param_tbl)/sizeof(rp_app_params_t);

    p_copy = (rp_app_params_t *)malloc(sizeof(g_rp_param_tbl));
    if(p_copy == NULL)
    {
        return -1;
    }
    memcpy(p_copy,g_rp_param_tbl,sizeof(g_rp_param_tbl));

    //We can copy all of the values, but
    //we will have problems with the name,
    //on account of the fact that the data cmd
    //module will try to free, so we must allocate
    //new strings.
    for(i = 0; i < (num_config_params -1); i++)
    {
        p_copy[i].name = strdup(g_rp_param_tbl[i].name);
    }

    p_copy[num_config_params].name = NULL;

    *p = p_copy;

    return num_config_params-1;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
	LOG("%s was called.\n",__func__);

	/* We don't even try to pass signals back
	 * To the web application with this app.
	 * HTTP just can't handle the amount of data
	 * we are trying to push.
	 */
	*sig_num = 0;
	*sig_len = 0;

    return 0;
}

/******************************************************************************
 * static function definitions
 ******************************************************************************/
static int update_global_param_tbl(rp_app_params_t *p, int len)
{
	int i;
	int j;
    int num_global_cfg_params = sizeof(g_rp_param_tbl)/sizeof(rp_app_params_t);
    int num_updates = 0;

	for(i = 0; i<len; i++)
	{
		//Skip the last config parameter, because it's always NULL
		for(j=0; j<(num_global_cfg_params-1) ;j++)
		{
			if(0 == strcmp(p[i].name,g_rp_param_tbl[j].name))
			{
				if(g_rp_param_tbl[j].value != p[i].value)
				{
					g_rp_param_tbl[j].value = p[i].value;
					LOG("%s: %s->%d\n",__func__,p[i].name,(int)p[i].value);
					num_updates++;
				}
			}
		}
	}
	return num_updates;
}

