/*
 * File: image_bw_converter_altium.c
 * ---------------------------
 * Description:
 *   A command-line frontend to the libbwconvert shared library.
 *   Parses user input and invokes the C backend converter function.
 *
 * Author: Bahey Shalash
 * Version: 2.1.2
 * Date: 19/04/2025
 *
 * Compilation:
 *   gcc -O3 image_bw_converter_altium.c -L. -lbwconvert -o image_bw_converter
 *
 * Usage:
 *   ./image_bw_converter [options] <input> <output>
 *
 * Options:
 *   -t threshold    brightness cutoff (0-255; default: 128)
 *   -i               invert after dithering
 *   -v               verbose mode
 *   -h               show this message
 *   --version        show version info
 */
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bw_converter.h"

static void showUsage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [options] <input> <output>\n"
            "Options:\n"
            "  -t threshold    brightness cutoff (0-255; default:128)\n"
            "  -i               invert after dithering\n"
            "  -v               verbose mode\n"
            "  -h               show this message\n"
            "  --version        show version\n",
            prog);
}

static void showVersion(void) {
    printf("image_bw_converter version 2.1.2 (19/04/2025)\n");
}

int main(int argc, char *argv[]) {
    int threshold = 128;
    int invert = 0;
    int verbose = 0;

    struct option longOpts[] = {{"version", no_argument, 0, 'V'}, {0, 0, 0, 0}};
    int opt;
    while ((opt = getopt_long(argc, argv, "t:ivh", longOpts, NULL)) != -1) {
        switch (opt) {
            case 't':
                threshold = atoi(optarg);
                break;
            case 'i':
                invert = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                showUsage(argv[0]);
                return EXIT_FAILURE;
            case 'V':
                showVersion();
                return EXIT_SUCCESS;
            default:
                showUsage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (optind + 2 != argc) {
        showUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *in = argv[optind];
    const char *out = argv[optind + 1];
    return convert_image_bw(in, out, threshold, invert, verbose);
}