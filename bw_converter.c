/*
 * File: bw_converter.c
 * ---------------------------
 * Description:
 *   Shared C library implementation that converts color images
 *   into 1-bit black-and-white PNGs using Floyd–Steinberg error diffusion.
 *   Used by both CLI and GUI frontends.
 *
 * Author: Bahey Shalash
 * Version: 2.1.2
 * Date: 19/04/2025
 *
 * Dependencies:
 *   - stb_image.h
 *   - stb_image_write.h (https://github.com/nothings/stb)
 *
 * Compilation (as shared lib):
 *   gcc -O3 -fPIC -shared -o libbwconvert.so bw_converter.c
 *
 * Usage:
 *   Used via CLI or GUI by calling:
 *     int convert_image_bw(input_path, output_path, threshold, invert, verbose);
 */
#include "bw_converter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "stb_image_write.h"

#define DEFAULT_DPI 300
#define FS_RIGHT (7.0f / 16.0f)
#define FS_BOTTOM (5.0f / 16.0f)
#define FS_BOTTOM_L (3.0f / 16.0f)
#define FS_BOTTOM_R (1.0f / 16.0f)

typedef enum { ERR_OK = 0, ERR_LOAD, ERR_MEMORY, ERR_WRITE } ErrorCode;
typedef struct {
    int brightnessThreshold;
    bool invertOutput;
    bool verboseMode;
} BWConfig;

static ErrorCode loadGrayImage(const char *path, unsigned char **gray, int *w, int *h,
                               const BWConfig *cfg) {
    int channels;
    unsigned char *rgb = stbi_load(path, w, h, &channels, 3);
    if (!rgb)
        return ERR_LOAD;

    int total = (*w) * (*h);
    *gray = malloc(total);
    if (!*gray) {
        stbi_image_free(rgb);
        return ERR_MEMORY;
    }

    for (int i = 0; i < total; i++) {
        unsigned char r = rgb[3 * i], g = rgb[3 * i + 1], b = rgb[3 * i + 2];
        (*gray)[i] = (unsigned char)(0.2126f * r + 0.7152f * g + 0.0722f * b);
    }
    stbi_image_free(rgb);

    if (cfg->verboseMode)
        fprintf(stderr, "Loaded '%s' (%dx%d)\n", path, *w, *h);
    return ERR_OK;
}

static float *createErrorBuffer(const unsigned char *gray, int total) {
    float *err = malloc(total * sizeof(float));
    if (!err)
        return NULL;
    for (int i = 0; i < total; i++)
        err[i] = gray[i];
    return err;
}

static void disperseError(float *err, int idx, float e, int w, int h) {
    int row = idx / w, col = idx % w;
    if (col + 1 < w)
        err[idx + 1] += e * FS_RIGHT;
    if (row + 1 < h) {
        if (col > 0)
            err[idx + w - 1] += e * FS_BOTTOM_L;
        err[idx + w] += e * FS_BOTTOM;
        if (col + 1 < w)
            err[idx + w + 1] += e * FS_BOTTOM_R;
    }
}

static void diffuseImage(unsigned char *out, float *err, int w, int h,
                         const BWConfig *cfg) {
    int total = w * h;
    for (int i = 0; i < total; i++) {
        float old = err[i];
        float neu = old < cfg->brightnessThreshold ? 0.0f : 255.0f;
        out[i] = (unsigned char)neu;
        disperseError(err, i, old - neu, w, h);
        if (cfg->verboseMode && i % w == 0 && (i / w) % 50 == 0)
            fprintf(stderr, "Row %d/%d\n", i / w, h);
    }
}

static ErrorCode saveBWImage(const char *path, unsigned char *buf, int w, int h,
                             const BWConfig *cfg) {
    if (cfg->verboseMode) {
        fprintf(stderr, "Writing '%s'\n", path);
        float mmw = (w / (float)DEFAULT_DPI) * 25.4f;
        float mmh = (h / (float)DEFAULT_DPI) * 25.4f;
        fprintf(stderr, "Output: %d×%d px (~%.2f×%.2f mm)\n", w, h, mmw, mmh);
    }
    return stbi_write_png(path, w, h, 1, buf, w) ? ERR_OK : ERR_WRITE;
}

static ErrorCode convertToBW(const char *in, const char *out, const BWConfig *cfg) {
    unsigned char *gray = NULL;
    int w, h;
    ErrorCode r = loadGrayImage(in, &gray, &w, &h, cfg);
    if (r != ERR_OK)
        return r;

    int total = w * h;
    float *err = createErrorBuffer(gray, total);
    if (!err) {
        free(gray);
        return ERR_MEMORY;
    }

    diffuseImage(gray, err, w, h, cfg);
    free(err);

    if (cfg->invertOutput)
        for (int i = 0; i < total; i++)
            gray[i] = 255 - gray[i];

    r = saveBWImage(out, gray, w, h, cfg);
    free(gray);
    return r;
}

int convert_image_bw(const char *input_path, const char *output_path, int threshold,
                     int invert, int verbose) {
    BWConfig cfg = {.brightnessThreshold = threshold,
                    .invertOutput = (invert != 0),
                    .verboseMode = (verbose != 0)};
    return convertToBW(input_path, output_path, &cfg);
}