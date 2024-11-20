#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "raycaster.h"

Image* raycast_sequential(Image* scene, Light* lights, int light_count) {
    // Task 1.
    return new_image(scene->width, scene->height);
}

Image* raycast_parallel_lights(Image* scene, Light* lights, int light_count,
                               int max_threads) {
    // Task 2.
    return new_image(scene->width, scene->height);
}

Image* raycast_parallel_rows(Image* scene, Light* lights, int light_count,
                             int max_threads) {
    // Task 3.
    return new_image(scene->width, scene->height);
}
