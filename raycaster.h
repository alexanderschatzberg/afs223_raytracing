#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

#include "image.h"
#include "raycaster_util.h"

/*
 * Run the 2D raycasting algorithm on the given scene with the given lights,
 * returning a rendered image of the same size.
 *
 * This is a sequential implementation of this algorithm.
 */
Image* raycast_sequential(Image* scene, Light* lights, int light_count);

/*
 * Run the 2D raycasting algorithm on the given scene with the given lights,
 * returning a rendered image of the same size.
 *
 * This is a parallel implementation that can use up to `threads` threads.
 * Each thread handles a subset of the lights.
 */
Image* raycast_parallel_lights(Image* scene, Light* lights, int light_count,
                               int max_threads);

/*
 * Run the 2D raycasting algorithm on the given scene with the given lights,
 * returning a rendered image of the same size.
 *
 * This is a parallel implementation that can use up to `threads` threads.
 * Each thread handles a subset of the image's rows (y-coordinates).
 */
Image* raycast_parallel_rows(Image* image, Light* lights, int light_count,
                             int max_threads);

#endif // __RAYCASTER_H__
