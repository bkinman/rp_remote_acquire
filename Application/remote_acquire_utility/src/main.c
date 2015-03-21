#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

#include "main.h"
#include "version.h"

#include "options.h"
#include "scope.h"
#include "transfer.h"
#include "worker.h"


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
static rp_app_params_t* param_search_by_name(rp_app_params_t *params,uint16_t params_len, char* name);
static int app_params_to_options(rp_app_params_t* param_tbl,
                                 int param_tbl_len,
                                 option_fields_t *po_option_fields);

static int update_global_param_tbl(rp_app_params_t *p, int len);

static int display_options(option_fields_t options_fields);

static int start_acquisition(option_fields_t options_fields);
static int stop_acquisition(void);


/******************************************************************************
 * static variables
 ******************************************************************************/
static rp_app_params_t g_rp_param_tbl[] =
{
	{"ip_tup_a",             192 , 0, 0, 0, 255 },
	{"ip_tup_b",             168 , 0, 0, 0, 255 },
	{"ip_tup_c",             3   , 0, 0, 0, 255 },
	{"ip_tup_d",             2   , 0, 0, 0, 255 },
	{"port"    ,             6669, 0, 0, 0, 0 },
	{"mode"    ,             2   , 0, 0, 1, 3 }, //Default Mode: Server
	{"b_use_udp",            0   , 0, 0, 0, 1 },
	{"bytes_to_transfer",    0   , 0, 0, 0, 0 },
	{"b_report_rate",        0   , 0, 0, 0, 1 },
	{"scope_channel",        0   , 0, 0, 0, 1 },
	{"scope_decimation",     1024, 0, 0, 0, 65536 }, //Default Frequency: 122kSps
	{"b_scope_no_equalizer", 0,    0, 0, 0, 1 },
	{"b_scope_hv",           0,    0, 0, 0, 1 },
	{"scope_no_shaping",     0,    0, 0, 0, 1 },
	{"is_started",           0,    0, 0, 0, 1 }, //Responsible for starting Acquisition
	{ NULL,                  0.0, -1,-1,0.0,0.0}
};

static option_fields_t g_options = {{0}};

static struct worker_state worker = {
	.mux = PTHREAD_MUTEX_INITIALIZER,
	.state = dead,
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

	/*LOG("Saving current FPGA image: ");
	result = system("cat /dev/xdevcfg >/tmp/orig_fpga.bin");
	LOG(!result ? "SUCCESS\n" : "FAIL\n");*/
	LOG("Loading FPGA image: ");
	result = system("cat /opt/www/apps/remote_acquire_utility/ddrdump.bin >/dev/xdevcfg");
	LOG((0==result)?"SUCCESS\n":"FAIL\n");

	LOG("Loading Kernel Module: ");
	result = system("insmod /opt/www/apps/remote_acquire_utility/rpad.ko");
	LOG((0==result)?"SUCCESS\n":"FAIL\n");

	result = app_params_to_options(g_rp_param_tbl,
	                               sizeof(g_rp_param_tbl)/sizeof(rp_app_params_t),
	                               &g_options);

	display_options(g_options);
	worker.options = &g_options;

	if(0 != result)
	{
		LOG("%s: Can't load app params into options structure.\n",__func__);
	}

	return 0;
}

int rp_app_exit(void)
{
	int result;

	LOG("Unloading Remote Acquisition Utility.\n");

	if (worker.state != dead)
		result = stop_acquisition();

	LOG("Unloading kernel module: ");
	result = system("rmmod rpad.ko");
	LOG(!result ? "SUCCESS\n" : "FAIL\n");

	/*LOG("Restoring FPGA image: ");
	result = system("cat /tmp/orig_fpga.bin >/dev/xdevcfg");
	LOG(!result ? "SUCCESS\n" : "FAIL\n");*/

	return 0;
}

int rp_set_params(rp_app_params_t *p, int len)
{
	int ret_val;
	int have_new_options;
	rp_app_params_t *p_is_started_param;

	LOG("%s was called.\n",__func__);
	LOG("Received %d params.\n",len);

	ret_val = update_global_param_tbl(p,len);
	if(ret_val < 0)
	{
		LOG("Problem updating param table.\n");
		return -1;
	}

	have_new_options = (ret_val>0)?1:0;

	LOG("%s: %d config params were changed.\n",__func__,ret_val);

	ret_val = app_params_to_options(g_rp_param_tbl,
	                                sizeof(g_rp_param_tbl)/sizeof(rp_app_params_t),
	                                &g_options);
	if(ret_val != 0)
	{
		LOG("%s: problem converting app parameters to options.\n",__func__);
	}

	if(have_new_options)
	{
		LOG("%s: Displaying new options\n",__func__);
		display_options(g_options);
	}

	p_is_started_param = param_search_by_name(g_rp_param_tbl,
	                                          sizeof(g_rp_param_tbl)/sizeof(rp_app_params_t),
	                                          "is_started");

	if(NULL != p_is_started_param)
	{
		if (p_is_started_param->value && worker.state != running) {
			if (start_acquisition(g_options))
				LOG("%s: Unable to start acquisition.\n",__func__);
		}
		else if (!p_is_started_param->value && worker.state == running) {
			if (stop_acquisition())
				LOG("%s: Unable to stop acquisition.\n",__func__);
		}
	}

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
static rp_app_params_t* param_search_by_name(rp_app_params_t *params,uint16_t params_len ,char* name)
{
	int i;

	if(NULL == params || NULL == name)
	{
		LOG("%s: called with NULL param\n",__func__);
		return NULL;
	}

	for(i=0;i<params_len;i++)
	{
		if(NULL != params[i].name)
		{
			if(0 == strcmp(params[i].name,name))
			{
				return &params[i];
			}
		}
	}

	return NULL;
}

static int app_params_to_options(rp_app_params_t* param_tbl,
                                 int param_tbl_len,
                                 option_fields_t *po_option_fields)
{
	int i;
	int tup_a = -1;
	int tup_b = -1;
	int tup_c = -1;
	int tup_d = -1;

	if(NULL == param_tbl || NULL == po_option_fields)
	{
		LOG("%s: called with NULL parameter.\n",__func__);
		return -1;
	}

	//Cruise through each entry
	//But ignore the last entry, because it's a sentinel.
	for(i=0;i<param_tbl_len-1;i++)
	{
		if(0 == strcmp("ip_tup_a",param_tbl[i].name))
		{
			tup_a = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("ip_tup_b",param_tbl[i].name))
		{
			tup_b = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("ip_tup_c",param_tbl[i].name))
		{
			tup_c = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("ip_tup_d",param_tbl[i].name))
		{
			tup_d = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("port",param_tbl[i].name))
		{
			po_option_fields->port = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("mode",param_tbl[i].name))
		{
			int val = (int)param_tbl[i].value;
			enum mode_e mode;

			switch(val)
			{
				case 1:
					mode = client;
					break;
				case 2:
					mode = server;
					break;
				case 3:
					mode = file;
					break;
				default:
					LOG("%s: unrecognized mode (%d)\n",__func__,val);
					return -1;
			}

			po_option_fields->mode = mode;
			continue;
		}

		if(0 == strcmp("b_use_udp",param_tbl[i].name))
		{
			po_option_fields->tcp = ((int)param_tbl[i].value)?0:1;
			continue;
		}

		if(0 == strcmp("bytes_to_transfer",param_tbl[i].name))
		{
			po_option_fields->kbytes_to_transfer = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("scope_channel",param_tbl[i].name))
		{
			po_option_fields->scope_chn = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("scope_decimation",param_tbl[i].name))
		{
			po_option_fields->scope_dec = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("b_scope_no_equalizer",param_tbl[i].name))
		{
			po_option_fields->scope_equalizer = ((int)param_tbl[i].value)?0:1;
			continue;
		}

		if(0 == strcmp("b_scope_hv",param_tbl[i].name))
		{
			po_option_fields->scope_hv = (int)param_tbl[i].value;
			continue;
		}

		if(0 == strcmp("scope_no_shaping",param_tbl[i].name))
		{
			po_option_fields->scope_shaping = ((int)param_tbl[i].value)?0:1;
			continue;
		}
	}

	if((tup_a>=0) && (tup_b>=0) && (tup_c>=0) && (tup_d>=0))
	{
		snprintf(po_option_fields->address,
		         sizeof(po_option_fields->address),
		         "%d.%d.%d.%d",tup_a,tup_b,tup_c,tup_d);
	}

	return 0;
}

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

static int display_options(option_fields_t options_fields)
{
	LOG("OPTIONS:\n");
	LOG("\taddress: %s\n",options_fields.address);
	LOG("\tport: %d\n",options_fields.port);
	LOG("\ttcp: %d\n",options_fields.tcp);
	LOG("\tmode: %d\n",(int)options_fields.mode);
	LOG("\tkbytes_to_transfer: %d\n",(int)options_fields.kbytes_to_transfer);
	LOG("\tfname: %s\n",options_fields.fname);
	LOG("\treport_rate: %d\n",options_fields.report_rate);
	LOG("\tscope_chn: %d\n",options_fields.scope_chn);
	LOG("\tscope_dec: %d\n",options_fields.scope_dec);
	LOG("\tscope_hv: %d\n",options_fields.scope_hv);
	LOG("\tscope_equalizer: %d\n",options_fields.scope_equalizer);
	LOG("\tscope_shaping: %d\n",options_fields.scope_shaping);

	return 0;
}

static int start_acquisition(option_fields_t options_fields)
{
	LOG("%s: starting acquisition!\n",__func__);

	if (acq_worker_init(&worker)) {
		LOG("%s: problem initializing worker thread.\n",__func__);
		return -1;
	}

	return 0;
}

static int stop_acquisition(void)
{
	LOG("%s: stopping acquisition!\n",__func__);

	if (acq_worker_exit(&worker)) {
		LOG("%s: problem exiting worker thread.\n",__func__);
		return -1;
	}

	return 0;
}
