/*
 * File: bw_converter.h
 * ---------------------------
 * Description:
 *   Header file for the image-to-black-and-white converter library.
 *   Exposes a single C API for use in CLI or Python GUI frontends.
 *
 * Author: Bahey Shalash
 * Version: 1.0
 * Date: 19/04/2025
 *
 * Function:
 *   int convert_image_bw(const char *input_path,
 *                        const char *output_path,
 *                        int threshold,
 *                        int invert,
 *                        int verbose);
 */
#ifndef BW_CONVERTER_H
#define BW_CONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a color image to 1‑bit black-and-white PNG using
 * Floyd–Steinberg dithering.
 *
 * @param input_path   path to input PNG/JPEG/BMP/etc.
 * @param output_path  path for output PNG
 * @param threshold    brightness cutoff (0–255)
 * @param invert       nonzero to invert output after dithering
 * @param verbose      nonzero for verbose log messages
 * @return 0 on success, nonzero error code on failure
 */
int convert_image_bw(const char *input_path, const char *output_path, int threshold,
                     int invert, int verbose);

#ifdef __cplusplus
}
#endif

#endif /* BW_CONVERTER_H */