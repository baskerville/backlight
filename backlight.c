#define _DEFAULT_SOURCE // expose u_short in glibc, needed by fts(3)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // learn MAXPATHLEN
#include <fts.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { COMMAND_C, COMMAND_M, COMMAND_UNDEF } COMMAND;

char backlight_interface[MAXPATHLEN];
char actual_brightness_value[MAXPATHLEN];
char maximum_brightness_value[MAXPATHLEN];

COMMAND determine_command(char *msg)
{
    struct {
        COMMAND id;
        char *word;
    } commands[] = {
        { COMMAND_C, "current" },
        { COMMAND_M, "maximum" },
        { COMMAND_UNDEF, NULL }
    }, *curr = commands, *match = NULL;
    unsigned int nmatch = 0;
    while (curr->word != NULL) {
        if (strstr(curr->word, msg) == curr->word) {
            nmatch++;
            match = curr;
        }
        curr++;
    }
    return nmatch == 1 ? match->id : COMMAND_UNDEF;
}

int find_sysfs_backlight(void)
{
    bool found = false;
    char *path_argv[] = { "/sys/class/backlight", NULL };
    FTS *ftsp = fts_open(path_argv, FTS_LOGICAL | FTS_NOSTAT, NULL);
    if (ftsp != NULL) {
        fts_read(ftsp);

        struct stat statb;
        FTSENT *curr = fts_children(ftsp, FTS_NAMEONLY);
        while (!found && curr != NULL) {
            snprintf(actual_brightness_value,
                     sizeof (actual_brightness_value),
                     "%s%s/actual_brightness",
                     curr->fts_path,
                     curr->fts_name);
            if (stat(actual_brightness_value, &statb) == 0 &&
                S_ISREG(statb.st_mode)) {
                found = true;
                snprintf(maximum_brightness_value,
                         sizeof (maximum_brightness_value),
                         "%s%s/max_brightness",
                         curr->fts_path,
                         curr->fts_name);
                snprintf(backlight_interface,
                         sizeof (backlight_interface),
                         "%s%s/brightness",
                         curr->fts_path,
                         curr->fts_name);
            }
            curr = curr->fts_link;
        }
        fts_close(ftsp);
    }
    return found ? 0 : -1;
}

int percent(int *v)
{
    unsigned int val;
    FILE *src = fopen(maximum_brightness_value, "r");
    if (src != NULL) {
        int nv = fscanf(src, "%u", &val);
        if (nv == 1)
            *v = (*v / 100.0) * val;
        else
            return -1;
        fclose(src);
    } else {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int exit_status = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr, "usage: backlight current|maximum|[+|-]VALUE[%%]\n");
        exit_status = EXIT_FAILURE;
    } else if (find_sysfs_backlight() == 0) {
        char *msg = argv[1];
        if (isalpha(msg[0])) {
            FILE *src = NULL;
            switch (determine_command(msg)) {
                case COMMAND_C:
                    src = fopen(actual_brightness_value, "r");
                    break;
                case COMMAND_M:
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
                int next;
                int nn = sscanf(msg, "%i%c", &next, &p);
                if (nn == 1 || (nn == 2 && p == '%' && percent(&next) == 0)) {
                    if (msg[0] == '-' || msg[0] == '+') {
                        unsigned int val;
                        int nv = fscanf(src, "%u", &val);
                        if (nv == 1) {
                            fprintf(dst, "%u", val + next);
                        } else {
                            exit_status = EXIT_FAILURE;
                        }
                    } else {
                        fprintf(dst, "%u", next);
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
