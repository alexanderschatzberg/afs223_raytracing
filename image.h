#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>
#include <stdlib.h>

#define CHANNELS 3

/**
 * An RGB color value.
 */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;

/**
 * A 2D image.
 *
 * Pixels are stored in a row-major array: the pixel at (x, y) is stored in the
 * array at `pixels[y * width + x]`.
 */
typedef struct {
    Color* pixels;
    int width;
    int height;
} Image;

/**
 * Load an image from a PNG file.
 */
Image* read_image(const char* filename);

/**
 * Same an image to a PNG file.
 */
void write_image(const char* filename, Image* image);

/**
 * Deallocate an image.
 */
void free_image(Image* image);

/**
 * Get a pointer to one pixel in an image.
 */
Color* image_pixel(Image* image, int x, int y);

/**
 * Create a new, empty image. All pixels are initialized to black (0, 0, 0).
 */
Image* new_image(int width, int height);

/*
 * Adds two colors together component-wise
 * Each component is clamped to [0, 255] without overflow
 * For example, if adding would produce 280, 255 is returned instead
 */
Color add_colors(Color color1, Color color2);

/*
 * Multiplies two colors component-wise
 * Note that multiplication here applies to normalized values (0.0-1.0)
 * Each component is clamped to [0, 255] without overflow
 * For example, if multiplying would produce 280, 255 is returned instead
 */
Color mul_colors(Color color1, Color color2);

/*
 * Multiplies a color by a float scale
 * Each component is clamped to [0, 255] without overflow
 * For example, if multiplying would produce 280, 255 is returned instead
 */
Color scale_color(Color pixel, float scale);

#endif // __IMAGE_H__
