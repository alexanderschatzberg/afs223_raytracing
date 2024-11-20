#ifndef __RAYCAST_UTIL_H__
#define __RAYCAST_UTIL_H__

#include "image.h"

// Small constant to check for floating point similarity (especially with zero)
#define EPS 1e-8

// Constant all-zero color, which may be useful when implementing a raytracer
#define BLACK (Color){0, 0, 0};

/*
 * Represents a pair of doubles
 * Can be used either as a point or a direction
 */
typedef struct {
    double x;
    double y;
} Pair;

/*
 * Represents the location of a pixel, as fixed to integer indices
 * Corresponds to the (row, column) of the `Image` representation
 */
typedef struct {
    unsigned int x;
    unsigned int y;
} PixelLocation;

/*
 * Represents a light source with an color, strength, and location
 * Note that a light source is associated with a pixel, even though the light
 * itself is not drawn Note also that a light must have a very large `strength`
 * value to illuminate more than a few pixels
 */
typedef struct {
    Color color;
    double strength;
    PixelLocation pixel;
} Light;

/*
 * Returns 1 if the given color is considered an obstacle, and 0 otherwise
 * A color is an obstacle if its components sum to a number less than 10
 */
int is_obstacle(Color color);

/*
 * Returns 1 if the two pixels are are adjacent (including diagonal), and 0
 * otherwise
 */
int adjacent_pixels(PixelLocation pixel1, PixelLocation pixel2);

/*
 * Returns the center point associated with the given pixel for raycasting
 * We start at the center point of a pixel to help avoid initial error when
 * raycasting
 */
Pair center_point(int x, int y);

/*
 * Returns 1 if the given point is considered to be within the given pixel
 * location Otherwise, returns 0
 */
int in_pixel(Pair point, PixelLocation pixel);

/*
 * Returns the direction from the given "start" point to the given "end" point
 * This direction is given by a pair of x_direction and y_direction
 * Note that sqrt(x_dir**2 + y_dir**2) == 1
 */
Pair direction_pair(PixelLocation start, PixelLocation end);

/*
 * Get the next pixel in the given `direction` (an angle given as an (x, y) pair)
 * starting from the given image position, `pos`.
 *
 * Counter-intuitively, `pos` is a *floating-point* image position. So it can
 * represent sub-pixel locations (i.e., different positions within a single pixel).
 * It is an in/out parameter: this function mutates `pos` to move it one pixel's
 * distance in the given direction. This is important so that subsequent calls to
 * `step` will continue from the correct position. (Imagine, for example, a line
 * that is almost, but not quite, horizontal: we will need to `step` several times
 * in the x direction before we ever increment the y-coordinate.)
 *
 * The return value is the (rounded) integer pixel position of the next pixel.
 *
 * A good way to use this function is to first initialize `pos` by converting an
 * integer pixel position to a floating-point `Pair`. Then, repeatedly call `step`
 * to get the next pixel. Each time, use the *return value* (not the mutated `pos`)
 * to track the appropriate pixel. Just thread `pos` back to the next `step` call.
 */
PixelLocation step(Pair* pos, Pair direction);

/*
 * Given a light source and a location to illuminate
 * Returns the color contribution from this light source
 */
Color illuminate(Light light, int x, int y);

#endif // __RAYCAST_UTIL_H__
