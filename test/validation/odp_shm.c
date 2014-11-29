/* Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

#include "odp.h"
#include "CUnit/Basic.h"
#include "odp_cunit_common.h"

#define ALIGE_SIZE  (128)
#define TESTNAME "cunit_test_shared_data"
#define TEST_SHARE_FOO (0xf0f0f0f0)
#define TEST_SHARE_BAR (0xf0f0f0f)

static void *run_shm_thread(void *arg)
{
	odp_shm_info_t  info;
	odp_shm_t shm;
	test_shared_data_t *test_shared_data;
	int thr;

	thr = odp_thread_id();
	printf("Thread %i starts\n", thr);

	shm = odp_shm_lookup(TESTNAME);
	CU_ASSERT(ODP_SHM_INVALID != shm);
	test_shared_data = odp_shm_addr(shm);
	CU_ASSERT(TEST_SHARE_FOO == test_shared_data->foo);
	CU_ASSERT(TEST_SHARE_BAR == test_shared_data->bar);
	CU_ASSERT(0 == odp_shm_info(shm, &info));
	CU_ASSERT(0 == strcmp(TESTNAME, info.name));
	CU_ASSERT(0 == info.flags);
	CU_ASSERT(test_shared_data == info.addr);
	CU_ASSERT(sizeof(test_shared_data_t) <= info.size);
	CU_ASSERT(odp_sys_page_size() == info.page_size);
	odp_shm_print_all();

	fflush(stdout);
	return arg;
}

static void test_odp_shm_sunnyday(void)
{
	pthrd_arg thrdarg;
	odp_shm_t shm;
	test_shared_data_t *test_shared_data;

	shm = odp_shm_reserve(TESTNAME,
			      sizeof(test_shared_data_t), ALIGE_SIZE, 0);
	CU_ASSERT(ODP_SHM_INVALID != shm);

	CU_ASSERT(0 == odp_shm_free(shm));
	CU_ASSERT(ODP_SHM_INVALID == odp_shm_lookup(TESTNAME));

	shm = odp_shm_reserve(TESTNAME,
			      sizeof(test_shared_data_t), ALIGE_SIZE, 0);
	CU_ASSERT(ODP_SHM_INVALID != shm);

	test_shared_data = odp_shm_addr(shm);
	CU_ASSERT(NULL != test_shared_data);
	test_shared_data->foo = TEST_SHARE_FOO;
	test_shared_data->bar = TEST_SHARE_BAR;

	thrdarg.numthrds = odp_sys_core_count();

	if (thrdarg.numthrds > MAX_WORKERS)
		thrdarg.numthrds = MAX_WORKERS;

	odp_cunit_thread_create(run_shm_thread, &thrdarg);
	odp_cunit_thread_exit(&thrdarg);
}

static int init(void)
{
	printf("\tODP API version: %s\n", odp_version_api_str());
	printf("\tODP implementation version: %s\n", odp_version_impl_str());
	return 0;
}

CU_TestInfo test_odp_shm[] = {
	{"test_odp_shm_creat",  test_odp_shm_sunnyday},
	CU_TEST_INFO_NULL,
};

CU_SuiteInfo suites[] = {
	{"odp_system", init, NULL, NULL, NULL, test_odp_shm},
	CU_SUITE_INFO_NULL,
};


int main(void)
{
	int ret;

	if (0 != odp_init_global(NULL, NULL)) {
		printf("odp_init_global fail.\n");
		return -1;
	}
	if (0 != odp_init_local()) {
		printf("odp_init_local fail.\n");
		return -1;
	}

	CU_set_error_action(CUEA_ABORT);

	CU_initialize_registry();
	CU_register_suites(suites);
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	ret = CU_get_number_of_failure_records();

	CU_cleanup_registry();

	odp_term_local();
	odp_term_global();

	return ret;
}
