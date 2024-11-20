#include <stdio.h>
#include <sys/time.h>

#include "image.h"
#include "raycaster.h"

// Constants for timing
// How many times to iterate while timing
#define ITERATIONS 10
// How many lights to be raycast against, placed regularly around the image
#define LIGHT_COUNT 8
// How many threads to use
#define THREAD_COUNT 4
// Name of the image to read from
#define FILENAME (const char*)"images/medium.png"

// The function to measure. Uncomment the call you want to benchmark.
void timed_function(Image* image, Light* lights) {
    // raycast_sequential(image, lights, LIGHT_COUNT);
    raycast_parallel_lights(image, lights, LIGHT_COUNT, THREAD_COUNT);
    // raycast_parallel_rows(image, lights, LIGHT_COUNT, THREAD_COUNT);
}

// Constants for lights, strength and color shouldn't matter for timing
#define WHITE (Color){255, 255, 255}
#define STRENGTH 42.0

const unsigned int SECONDS_TO_MICRO = 1000000;

/*
 * Helper function to build an array of `X` lights
 */
Light* build_lights(unsigned int light_count, const char* filename) {
    // Technically inefficient, but it makes our life easier
    Image* image = read_image(filename);

    Light* lights = malloc(sizeof(Light) * light_count);

    PixelLocation position;

    // special case of 1
    if (light_count == 1) {
        position = (PixelLocation){image->width / 2, image->height / 2};
        lights[0] = (Light){WHITE, STRENGTH, position};
        return lights;
    }

    // Grid out the image with two columns of lights
    unsigned int row_count = light_count / 2;
    // we want to round up on odd numbers
    unsigned int row_size = image->height / ((light_count + 1) / 2);

    for (unsigned int i = 0; i < row_count; i++) {
        unsigned int index = i * 2;
        // add the row_size / 2 so we go in the "middle" of the grid
        unsigned int row = i * row_size + row_size / 2;

        position = (PixelLocation){image->width / 4, row};
        lights[index] = (Light){WHITE, STRENGTH, position};

        position = (PixelLocation){3 * image->width / 4, row};
        lights[index + 1] = (Light){WHITE, STRENGTH, position};
    }

    // Add an extra light in the middle-bottom on odd light count
    if (light_count % 2 == 1) {
        unsigned int row = row_count * row_size + row_size / 2;
        position = (PixelLocation){image->width / 2, row};
        lights[light_count - 1] = (Light){WHITE, STRENGTH, position};
    }

    return lights;
}

int main(void) {
    // Setup the lights
    Light* lights = build_lights(LIGHT_COUNT, FILENAME);

    // Setup the structs we need for timing
    struct timeval start;
    struct timeval end;

    struct timeval total_difference;
    total_difference.tv_sec = 0;
    total_difference.tv_usec = 0;

    for (unsigned int iteration = 0; iteration < ITERATIONS; iteration++) {
        Image* image = read_image(FILENAME);

        // Print to see partial output during long iterations
        printf("Iteration %d\n", iteration);

        // We need to get a fresh image each loop because we modify it
        image = read_image(FILENAME);

        // Start time
        gettimeofday(&start, NULL);

        timed_function(image, lights);

        // End time
        gettimeofday(&end, NULL);

        // Record time
        total_difference.tv_sec += end.tv_sec - start.tv_sec;
        total_difference.tv_usec += end.tv_usec - start.tv_usec;

        // Clean up at end of loop
        free_image(image);
    }

    // Print the results of timing
    printf("Timing %s with %d iterations.  Average time in seconds:\n",
           FILENAME, ITERATIONS);
    long double micro =
        (SECONDS_TO_MICRO * total_difference.tv_sec + total_difference.tv_usec);
    printf("%Lf\n", micro / (ITERATIONS * SECONDS_TO_MICRO));

    return 0;
}
