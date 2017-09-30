#define _DEFAULT_SOURCE   // exposes u_short in glibc, needed by fts(3)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>	  // for MAXPATHLEN
#include <fts.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
	CMD_CUR,
	CMD_MAX,
	CMD_UNDEF
} CMD;

char backlight_interface[MAXPATHLEN];
char actual_brightness_value[MAXPATHLEN];
char maximum_brightness_value[MAXPATHLEN];

bool has_sysfs_backlight(void)
{
	bool found = false;
	char *path_argv[] = { "/sys/class/backlight", NULL };
	FTS *ftsp = fts_open(path_argv, FTS_LOGICAL | FTS_NOSTAT, NULL);
	if (ftsp != NULL) {
		fts_read(ftsp);

		struct stat statb;
		FTSENT *cur = fts_children(ftsp, FTS_NAMEONLY);
		while (!found && cur != NULL) {
			snprintf(actual_brightness_value,
					 sizeof (actual_brightness_value),
					 "%s%s/actual_brightness",
					 cur->fts_path,
					 cur->fts_name);
			if (stat(actual_brightness_value, &statb) == 0 &&
				S_ISREG(statb.st_mode)) {
				found = true;
				snprintf(maximum_brightness_value,
						 sizeof (maximum_brightness_value),
						 "%s%s/max_brightness",
						 cur->fts_path,
						 cur->fts_name);
				snprintf(backlight_interface,
						 sizeof (backlight_interface),
						 "%s%s/brightness",
						 cur->fts_path,
						 cur->fts_name);
			}
			cur = cur->fts_link;
		}
		fts_close(ftsp);
	}
	return found;
}

CMD parse_command(char *msg)
{
	struct {
		CMD id;
		char *name;
	} commands[] = {
		{ CMD_CUR, "current" },
		{ CMD_MAX, "maximum" },
		{ CMD_UNDEF, NULL }
	}, *cur = commands, *match = NULL;
	unsigned int nmatch = 0;
	while (cur->name != NULL) {
		if (strstr(cur->name, msg) == cur->name) {
			nmatch++;
			match = cur;
		}
		cur++;
	}
	return nmatch == 1 ? match->id : CMD_UNDEF;
}

bool query_max(int *v)
{
	int val;
	FILE *src = fopen(maximum_brightness_value, "r");
	if (src != NULL) {
		int nv = fscanf(src, "%i", &val);
		if (nv == 1)
			*v =  val;
		else
			return false;
		fclose(src);
	} else {
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	int exit_status = EXIT_SUCCESS;

	if (argc < 2) {
		fprintf(stderr, "usage: backlight c[urrent]|m[aximum]|[+|-]VALUE[%%]\n");
		exit_status = EXIT_FAILURE;
	} else if (has_sysfs_backlight()) {
		char *msg = argv[1];
		if (isalpha(msg[0])) {
			FILE *src = NULL;
			switch (parse_command(msg)) {
				case CMD_CUR:
					src = fopen(actual_brightness_value, "r");
					break;
				case CMD_MAX:
					src = fopen(maximum_brightness_value, "r");
					break;
				default:
					break;
			}
			if (src != NULL) {
				unsigned int val;
				int nv = fscanf(src, "%u", &val);
				if (nv == 1)
					printf("%u\n", val);
				else
					exit_status = EXIT_FAILURE;
				fclose(src);
			} else {
				exit_status = EXIT_FAILURE;
			}
		} else {
			FILE *src = fopen(actual_brightness_value, "r");
			FILE *dst = fopen(backlight_interface, "w");
			if (src != NULL && dst != NULL) {
				char p = 0;
				int next, max;
				int nn = sscanf(msg, "%i%c", &next, &p);
				if ((nn == 1 || (nn == 2 && p == '%')) && query_max(&max)) {
					if (msg[0] == '-' || msg[0] == '+') {
						int val;
						int nv = fscanf(src, "%i", &val);
						if (nv == 1) {
							val += next/100.0f*max;
							fprintf(dst, "%u", val > max ? max : val < 0 ? 0 : val);
						} else {
							exit_status = EXIT_FAILURE;
						}
					} else if (nn ==2 && p == '%') {
						int val;
						val = max/100.0f*next;
						fprintf(dst, "%u", val > max ? max : val < 0 ? 0 : val);
					}
					else {
						fprintf(dst, "%u", next > max ? max : next);
					}
				} else {
					exit_status = EXIT_FAILURE;
				}
			} else {
				if (src != NULL)
					fclose(src);
				if (dst != NULL)
					fclose(dst);
			}
		}
	} else {
		exit_status = EXIT_FAILURE;
	}

	return exit_status;
}
