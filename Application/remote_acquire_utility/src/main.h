/******************************************************************************
 *
 * @brief Red Pitaya Remote Acquire main module.
 *
 * @Author Brandon Kinman <brandon@kinmantech.org>
 *
 *****************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

/** Structure which describes parameters supported by the application.
 * Each application includes an parameters table which includes the following
 * information:
 *    - name - name of the parameter, which can be accessed from the client
 *    - value - value of the parameter
 *    - fpga_update - whether this parameter changes the FPGA content
 *    - read_only - if this parameter is read only all changes from the client
 *                  will be ignored
 *    - min_val - minimal valid value for the parameter
 *    - max_val - maximal valid value for the parameter
 *
 * NOTE: During set_params() only value can be changed, other parameters are
 *       read-only and can not be changed from the client.
 * TODO: Make the header accessible to the both - application & webserver
 */
typedef struct rp_app_params_s {
	char  *name;
	float  value;
	int    fpga_update;
	int    read_only;
	float  min_val;
	float  max_val;
} rp_app_params_t;

/* module entry points */
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

#endif /*  __MAIN_H */
