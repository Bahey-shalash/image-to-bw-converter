/*
 * File: NO_GUI.c
 * ---------------------------
 * Description:
 *   A versatile command-line tool to convert color images (PNG, JPEG, BMP, etc.)
 *   into a 1-bit black-and-white PNG using Floyd–Steinberg error diffusion,
 *   with correct inversion applied after diffusion.
 *
 *
 * Author: Bahey Shalash
 * Version: 2.1.2
 * Date: 19/04/2025
 *
 * Dependencies:
 *   - stb_image.h
 *   - stb_image_write.h (https://github.com/nothings/stb)
 *
 * Compilation:
 *  clang -O3 -Wall -Wextra -pedantic NO_GUI.c -o image_bw_converter
 *  or
 *   gcc -O3 -Wall -Wextra -pedantic NO_GUI.c -o image_bw_converter
 *
 * for basic compilation:
 *  gcc -O3 NO_GUI.c -o image_bw_converter
 * or
 *  clang -O3 NO_GUI.c -o image_bw_converter
 *
 * Usage:
 *   ./image_bw_converter [options] <input.(png|jpg|bmp)> <output.png>
 *
 * basic usage:
 *  ./image_bw_converter input.png output.png
 *
 * Options:
 *   -t threshold    brightness cutoff (0-255; default: 128)
 *   -i               invert black and white (applied after dithering)
 *   -v               verbose mode (shows pixel size & physical dimensions)
 *   -h               display help message
 *   --version        display version information
 */

#define DEFAULT_DPI 300

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "stb_image_write.h"

// Floyd–Steinberg weights
#define FS_RIGHT (7.0f / 16.0f)
#define FS_BOTTOM (5.0f / 16.0f)
#define FS_BOTTOM_L (3.0f / 16.0f)
#define FS_BOTTOM_R (1.0f / 16.0f)

// Error codes
typedef enum { ERR_OK = 0, ERR_LOAD, ERR_MEMORY, ERR_WRITE } ErrorCode;

// Configuration for conversion
typedef struct {
    int brightnessThreshold;
    bool invertOutput;
    bool verboseMode;
} BWConfig;

/* Function prototypes */
static void showUsage(const char *programName);
static void showVersion(void);
static ErrorCode loadGrayImage(const char *inputPath, unsigned char **grayBuffer,
                               int *imgWidth, int *imgHeight, const BWConfig *config);
static float *createErrorBuffer(const unsigned char *grayBuffer, int totalPixels);
static void disperseError(float *errorBuffer, int index, float error, int imgWidth,
                          int imgHeight);
static void diffuseImage(unsigned char *pixelBuffer, float *errorBuffer, int imgWidth,
                         int imgHeight, const BWConfig *config);
static ErrorCode saveBWImage(const char *outputPath, unsigned char *pixelBuffer,
                             int imgWidth, int imgHeight, const BWConfig *config);
static ErrorCode convertToBW(const char *inputPath, const char *outputPath,
                             const BWConfig *config);

int main(int argc, char *argv[]) {
    BWConfig config = {
        .brightnessThreshold = 128, .invertOutput = false, .verboseMode = false};
    struct option longOpts[] = {{"version", no_argument, 0, 'V'}, {0, 0, 0, 0}};
    int opt;
    while ((opt = getopt_long(argc, argv, "t:ivh", longOpts, NULL)) != -1) {
        switch (opt) {
            case 't':
                config.brightnessThreshold = atoi(optarg);
                break;
            case 'i':
                config.invertOutput = true;
                break;
            case 'v':
                config.verboseMode = true;
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
    return convertToBW(argv[optind], argv[optind + 1], &config);
}

static void showUsage(const char *programName) {
    fprintf(
        stderr,
        "Usage: %s [options] <input> <output>\n"
        "Options:\n"
        "  -t threshold    brightness cutoff (0-255, default: 128)\n"
        "  -i               invert black/white (after dithering)\n"
        "  -v               verbose mode (shows pixel size & physical dimensions)\n"
        "  -h               display this help message\n"
        "  --version        display version information\n",
        programName);
}

static void showVersion(void) {
    printf("image_bw_converter version 2.1.2 (19/04/2025) by Bahey Shalash\n");
}

static ErrorCode loadGrayImage(const char *inputPath, unsigned char **grayBuffer,
                               int *imgWidth, int *imgHeight, const BWConfig *config) {
    int channels;
    unsigned char *rgb = stbi_load(inputPath, imgWidth, imgHeight, &channels, 3);
    if (!rgb) {
        fprintf(stderr, "Error: cannot load '%s'\n", inputPath);
        return ERR_LOAD;
    }
    int totalPixels = (*imgWidth) * (*imgHeight);
    *grayBuffer = malloc(totalPixels);
    if (!*grayBuffer) {
        fprintf(stderr, "Error: out of memory allocating gray buffer\n");
        stbi_image_free(rgb);
        return ERR_MEMORY;
    }
    for (int i = 0; i < totalPixels; ++i) {
        int r = rgb[3 * i], g = rgb[3 * i + 1], b = rgb[3 * i + 2];
        (*grayBuffer)[i] = (unsigned char)(0.2126f * r + 0.7152f * g + 0.0722f * b);
    }
    stbi_image_free(rgb);
    if (config->verboseMode)
        fprintf(stderr, "Loaded '%s' (%dx%d)\n", inputPath, *imgWidth, *imgHeight);
    return ERR_OK;
}

static float *createErrorBuffer(const unsigned char *grayBuffer, int totalPixels) {
    float *errorBuffer = malloc(totalPixels * sizeof(float));
    if (!errorBuffer)
        return NULL;
    for (int i = 0; i < totalPixels; ++i)
        errorBuffer[i] = grayBuffer[i];
    return errorBuffer;
}

static void disperseError(float *errorBuffer, int index, float error, int imgWidth,
                          int imgHeight) {
    int row = index / imgWidth;
    int col = index % imgWidth;
    if (col + 1 < imgWidth)
        errorBuffer[index + 1] += error * FS_RIGHT;
    if (row + 1 < imgHeight) {
        if (col > 0)
            errorBuffer[index + imgWidth - 1] += error * FS_BOTTOM_L;
        errorBuffer[index + imgWidth] += error * FS_BOTTOM;
        if (col + 1 < imgWidth)
            errorBuffer[index + imgWidth + 1] += error * FS_BOTTOM_R;
    }
}

static void diffuseImage(unsigned char *pixelBuffer, float *errorBuffer, int imgWidth,
                         int imgHeight, const BWConfig *config) {
    int totalPixels = imgWidth * imgHeight;
    for (int idx = 0; idx < totalPixels; ++idx) {
        float oldVal = errorBuffer[idx];
        float newVal = (oldVal < config->brightnessThreshold) ? 0.0f : 255.0f;
        float quantError = oldVal - newVal;
        pixelBuffer[idx] = (unsigned char)newVal;
        disperseError(errorBuffer, idx, quantError, imgWidth, imgHeight);
        if (config->verboseMode && (idx % imgWidth) == 0 &&
            ((idx / imgWidth) % 50) == 0)
            fprintf(stderr, "Processing row %d/%d\n", idx / imgWidth, imgHeight);
    }
}

static ErrorCode saveBWImage(const char *outputPath, unsigned char *pixelBuffer,
                             int imgWidth, int imgHeight, const BWConfig *config) {
    if (config->verboseMode) {
        fprintf(stderr, "Writing '%s'\n", outputPath);
        float w_mm = (imgWidth / (float)DEFAULT_DPI) * 25.4f;
        float h_mm = (imgHeight / (float)DEFAULT_DPI) * 25.4f;
        fprintf(stderr, "Output: %d×%d px (~%.2fmm × %.2fmm at %ddpi)\n", imgWidth,
                imgHeight, w_mm, h_mm, DEFAULT_DPI);
    }
    if (!stbi_write_png(outputPath, imgWidth, imgHeight, 1, pixelBuffer, imgWidth)) {
        fprintf(stderr, "Error: cannot write '%s'\n", outputPath);
        return ERR_WRITE;
    }
    return ERR_OK;
}

static ErrorCode convertToBW(const char *inputPath, const char *outputPath,
                             const BWConfig *config) {
    unsigned char *grayBuffer = NULL;
    int imgWidth, imgHeight;
    ErrorCode result =
        loadGrayImage(inputPath, &grayBuffer, &imgWidth, &imgHeight, config);
    if (result != ERR_OK)
        return result;

    int totalPixels = imgWidth * imgHeight;
    float *errorBuffer = createErrorBuffer(grayBuffer, totalPixels);
    if (!errorBuffer) {
        fprintf(stderr, "Error: out of memory allocating error buffer\n");
        free(grayBuffer);
        return ERR_MEMORY;
    }

    diffuseImage(grayBuffer, errorBuffer, imgWidth, imgHeight, config);
    free(errorBuffer);

    // Apply inversion after dithering if requested
    if (config->invertOutput) {
        for (int i = 0; i < totalPixels; ++i)
            grayBuffer[i] = 255 - grayBuffer[i];
    }

    result = saveBWImage(outputPath, grayBuffer, imgWidth, imgHeight, config);
    free(grayBuffer);
    return result;
}
