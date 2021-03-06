/* Copyright (c) 2018, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include <odp/api/version.h>
#include <odp_internal.h>
#include <odp_debug_internal.h>
#include <odp_libconfig_internal.h>
#include <odp_libconfig_config.h>

#define CONF_STR_NAME ((const char *)odp_linux_generic_conf)

extern struct odp_global_data_s odp_global_data;

int _odp_libconfig_init_global(void)
{
	const char *filename;
	const char *vers;
	const char *vers_rt;
	const char *ipml;
	const char *ipml_rt;
	config_t *config = &odp_global_data.libconfig_default;
	config_t *config_rt = &odp_global_data.libconfig_runtime;

	config_init(config);
	config_init(config_rt);

	if (!config_read_string(config, CONF_STR_NAME)) {
		ODP_ERR("Failed to read default config: %s(%d): %s\n",
			config_error_file(config), config_error_line(config),
			config_error_text(config));
		goto fail;
	}

	filename = getenv("ODP_CONFIG_FILE");
	if (filename == NULL)
		return 0;

	if (!config_read_file(config_rt, filename)) {
		ODP_ERR("Failed to read config file: %s(%d): %s\n",
			config_error_file(config_rt),
			config_error_line(config_rt),
			config_error_text(config_rt));
		goto fail;
	}

	/* Check runtime configuration's implementation name and version */
	if (!config_lookup_string(config, "odp_implementation", &ipml) ||
	    !config_lookup_string(config_rt, "odp_implementation", &ipml_rt)) {
		ODP_ERR("Configuration missing 'odp_implementation' field\n");
		goto fail;
	}
	if (!config_lookup_string(config, "config_file_version", &vers) ||
	    !config_lookup_string(config_rt, "config_file_version", &vers_rt)) {
		ODP_ERR("Configuration missing 'config_file_version' field\n");
		goto fail;
	}
	if (strcmp(vers, vers_rt) || strcmp(ipml, ipml_rt)) {
		ODP_ERR("Runtime configuration mismatch\n");
		goto fail;
	}

	return 0;
fail:
	config_destroy(config);
	config_destroy(config_rt);
	return -1;
}

int _odp_libconfig_term_global(void)
{
	config_destroy(&odp_global_data.libconfig_default);
	config_destroy(&odp_global_data.libconfig_runtime);

	return 0;
}

int _odp_libconfig_lookup_int(const char *path, int *value)
{
	int ret_def = CONFIG_FALSE;
	int ret_rt = CONFIG_FALSE;

	ret_def = config_lookup_int(&odp_global_data.libconfig_default, path,
				    value);

	/* Runtime option overrides default value */
	ret_rt = config_lookup_int(&odp_global_data.libconfig_runtime, path,
				   value);

	return  (ret_def == CONFIG_TRUE || ret_rt == CONFIG_TRUE) ? 1 : 0;
}
