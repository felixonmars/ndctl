// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2021, FUJITSU LIMITED. ALL rights reserved.

#include <errno.h>
#include <util/parse-configs.h>
#include <util/strbuf.h>
#include <util/iniparser.h>

int filter_conf_files(const struct dirent *dir)
{
	if (!dir)
		return 0;

	if (dir->d_type == DT_REG) {
		const char *ext = strrchr(dir->d_name, '.');
		if ((!ext) || (ext == dir->d_name))
			return 0;
		if (strcmp(ext, ".conf") == 0)
			return 1;
	}

	return 0;
}

static void set_str_val(const char **value, const char *val)
{
	struct strbuf buf = STRBUF_INIT;
	size_t len = *value ? strlen(*value) : 0;

	if (!val)
		return;

	if (len) {
		strbuf_add(&buf, *value, len);
		strbuf_addstr(&buf, " ");
	}
	strbuf_addstr(&buf, val);
	*value = strbuf_detach(&buf, NULL);
}

static int parse_config_file(const char *config_file,
			const struct config *configs)
{
	dictionary *dic;

	dic = iniparser_load(config_file);
	if (!dic)
		return -errno;

	for (; configs->type != CONFIG_END; configs++) {
		switch (configs->type) {
		case CONFIG_STRING:
			set_str_val((const char **)configs->value,
					iniparser_getstring(dic,
					configs->key, configs->defval));
			break;
		case MONITOR_CALLBACK:
		case CONFIG_END:
			break;
		}
	}

	iniparser_freedict(dic);
	return 0;
}

int parse_configs_prefix(const char *__config_files, const char *prefix,
				const struct config *configs)
{
	char *config_files, *save;
	const char *config_file;
	int rc;

	config_files = strdup(__config_files);
	if (!config_files)
		return -ENOMEM;

	for (config_file = strtok_r(config_files, " ", &save); config_file;
				config_file = strtok_r(NULL, " ", &save)) {

		if (strncmp(config_file, "./", 2) != 0)
			fix_filename(prefix, &config_file);

		if ((configs->type == MONITOR_CALLBACK) &&
				(strcmp(config_file, configs->key) == 0))
			rc = configs->callback(configs, configs->key);
		else
			rc = parse_config_file(config_file, configs);

		if (rc)
			goto end;
	}

 end:
	free(config_files);
	return rc;

}
