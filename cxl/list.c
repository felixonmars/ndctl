// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <util/log.h>
#include <util/json.h>
#include <util/filter.h>
#include <json-c/json.h>
#include <cxl/libcxl.h>
#include <util/parse-options.h>
#include <ccan/array_size/array_size.h>

static struct {
	bool memdevs;
	bool idle;
	bool human;
	bool verbose;
} list;

static unsigned long listopts_to_flags(void)
{
	unsigned long flags = 0;

	if (list.idle)
		flags |= UTIL_JSON_IDLE;
	if (list.human)
		flags |= UTIL_JSON_HUMAN;
	return flags;
}

static struct {
	const char *memdev;
} param;

static int did_fail;

#define fail(fmt, ...) \
do { \
	did_fail = 1; \
	fprintf(stderr, "cxl-%s:%s:%d: " fmt, \
			VERSION, __func__, __LINE__, ##__VA_ARGS__); \
} while (0)

static int num_list_flags(void)
{
	return list.memdevs;
}

struct cxl_cmd *memdev_identify(struct cxl_memdev *memdev)
{
	struct cxl_cmd *id;

	id = cxl_cmd_new_identify(memdev);
	if (!id)
		return NULL;

	if (cxl_cmd_submit(id) != 0)
		return NULL;
	return id;
}

int cmd_list(int argc, const char **argv, struct cxl_ctx *ctx)
{
	const struct option options[] = {
		OPT_STRING('d', "memdev", &param.memdev, "memory device name",
			   "filter by CXL memory device name"),
		OPT_BOOLEAN('D', "memdevs", &list.memdevs,
			    "include CXL memory device info"),
		OPT_BOOLEAN('i', "idle", &list.idle, "include idle devices"),
		OPT_BOOLEAN('u', "human", &list.human,
				"use human friendly number formats"),
		OPT_BOOLEAN('v', "verbose", &list.verbose,
				"enable verbose output"),
		OPT_END(),
	};
	const char * const u[] = {
		"cxl list [<options>]",
		NULL
	};
	struct json_object *jdevs = NULL;
	unsigned long list_flags;
	struct cxl_memdev *memdev;
	int i;

        argc = parse_options(argc, argv, options, u, 0);
	for (i = 0; i < argc; i++)
		error("unknown parameter \"%s\"\n", argv[i]);

	if (argc)
		usage_with_options(u, options);

	if (num_list_flags() == 0)
		list.memdevs = true;

	list_flags = listopts_to_flags();

	if (list.verbose)
		cxl_set_log_priority(ctx, LOG_DEBUG);

	cxl_memdev_foreach(ctx, memdev) {
		struct json_object *jdev = NULL;
		struct cxl_cmd *id;

		if (!util_cxl_memdev_filter(memdev, param.memdev))
			continue;

		if (list.memdevs) {
			id = memdev_identify(memdev);
			if (!jdevs) {
				jdevs = json_object_new_array();
				if (!jdevs) {
					fail("\n");
					cxl_cmd_unref(id);
					continue;
				}
			}

			jdev = util_cxl_memdev_to_json(memdev, id, list_flags);
			if (!jdev) {
				fail("\n");
				cxl_cmd_unref(id);
				continue;
			}
			json_object_array_add(jdevs, jdev);
			cxl_cmd_unref(id);
		}
	}

	if (jdevs)
		util_display_json_array(stdout, jdevs, list_flags);

	if (did_fail)
		return -ENOMEM;
	return 0;
}
