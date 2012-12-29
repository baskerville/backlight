#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define SYSFS_BACKLIGHT  "/sys/class/backlight/apple_backlight"
#define BACKLIGHT_INTERFACE  SYSFS_BACKLIGHT"/brightness"
#define ACTUAL_BRIGHTNESS_VALUE  SYSFS_BACKLIGHT"/actual_brightness"
#define MAXIMUM_BRIGHTNESS_VALUE  SYSFS_BACKLIGHT"/max_brightness"

int percent(int *v)
{
    unsigned int val;
    FILE *src = fopen(MAXIMUM_BRIGHTNESS_VALUE, "r");
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
        fprintf(stderr, "usage: backlight c|m|[+|-]VALUE[%%]\n");
        exit_status = EXIT_FAILURE;
    } else {
        char *msg = argv[1];
        if (isalpha(msg[0])) {
            FILE *src = NULL;
            if (strcmp(msg, "c") == 0)
                src = fopen(ACTUAL_BRIGHTNESS_VALUE, "r");
            else if (strcmp(msg, "m") == 0)
                src = fopen(MAXIMUM_BRIGHTNESS_VALUE, "r");
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
            FILE *src = fopen(ACTUAL_BRIGHTNESS_VALUE, "r");
            FILE *dst = fopen(BACKLIGHT_INTERFACE, "w");
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
    }

    return exit_status;
}
