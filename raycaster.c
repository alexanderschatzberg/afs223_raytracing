#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "raycaster.h"

Image* raycast_sequential(Image* scene, Light* lights, int light_count) {
    // Create a new image of the same size as the scene
    Image* cast = new_image(scene->width, scene->height);

    // Iterate over every pixel in the scene
    for (int y = 0; y < scene->height; y++) {
        for (int x = 0; x < scene->width; x++) {
            Color orig = *image_pixel(scene, x, y);
            // If it's an obstacle pixel, it remains unchanged (no illumination passes through)
            if (is_obstacle(orig)) {
                *image_pixel(cast, x, y) = orig;
                continue;
            }

            // Accumulate illumination from all lights
            Color total_illum = (Color){ 0, 0, 0 };

            for (int l = 0; l < light_count; l++) {
                Light current_light = lights[l];

                // If the pixel is the light source itself, it is always illuminated by that light.
                if (x == current_light.pixel.x && y == current_light.pixel.y) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                    continue;
                }

                // Determine the direction from pixel (x,y) to the light source (lx,ly)
                PixelLocation start = { x, y };
                PixelLocation end = { current_light.pixel.x, current_light.pixel.y };
                Pair direction = direction_pair(start, end);

                // We'll "walk" from the pixel towards the light, checking for obstacles
                Pair pos;
                pos.x = (double)x;
                pos.y = (double)y;

                int occluded = 0;
                // We step until we reach the light or hit an obstacle
                // We must be careful to stop the loop once we have passed the light
                // The direction_pair and step logic should guide us.
                while (1) {
                    PixelLocation next_pixel = step(&pos, direction);

                    // If we have reached or passed the light
                    if ((direction.x > 0 && next_pixel.x > end.x) ||
                        (direction.x < 0 && next_pixel.x < end.x) ||
                        (direction.y > 0 && next_pixel.y > end.y) ||
                        (direction.y < 0 && next_pixel.y < end.y) ||
                        (next_pixel.x == end.x && next_pixel.y == end.y)) {
                        // Reached the light without hitting an obstacle
                        break;
                    }

                    // Check if the new pixel is an obstacle
                    Color step_color = *image_pixel(scene, next_pixel.x, next_pixel.y);
                    if (is_obstacle(step_color)) {
                        occluded = 1;
                        break;
                    }
                }

                // If not occluded, add this light's contribution
                if (!occluded) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                }
            }

            // Multiply original pixel color by the total illumination
            *image_pixel(cast, x, y) = mul_colors(total_illum, orig);
        }
    }
    return cast;
}

typedef struct {
    Image* scene;
    Light* lights;
    int start_light;
    int end_light;
    Image* partial_illum;
} ThreadDataLights;

// Thread function that computes illumination from a subset of lights
static void* parallel_lights_worker(void* arg) {
    ThreadDataLights* data = (ThreadDataLights*)arg;
    Image* scene = data->scene;
    Light* lights = data->lights;
    Image* partial = data->partial_illum;

    for (int y = 0; y < scene->height; y++) {
        for (int x = 0; x < scene->width; x++) {
            Color orig = *image_pixel(scene, x, y);
            // Obstacle pixels get no illumination
            if (is_obstacle(orig)) {
                *image_pixel(partial, x, y) = (Color){ 0, 0, 0 };
                continue;
            }

            Color total_illum = (Color){ 0, 0, 0 };
            for (int l = data->start_light; l < data->end_light; l++) {
                Light current_light = lights[l];

                // If pixel is the light source, it's always illuminated
                if (x == current_light.pixel.x && y == current_light.pixel.y) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                    continue;
                }

                // Compute direction from pixel to light
                PixelLocation start = { x, y };
                PixelLocation end = { current_light.pixel.x, current_light.pixel.y };
                Pair direction = direction_pair(start, end);

                Pair pos = { (double)x, (double)y };
                int occluded = 0;

                // Trace the ray towards the light
                while (1) {
                    PixelLocation next_pixel = step(&pos, direction);

                    if ((direction.x > 0 && next_pixel.x >= end.x && direction.y > 0 && next_pixel.y >= end.y) ||
                        (direction.x > 0 && next_pixel.x >= end.x && direction.y <= 0 && next_pixel.y <= end.y) ||
                        (direction.x <= 0 && next_pixel.x <= end.x && direction.y > 0 && next_pixel.y >= end.y) ||
                        (direction.x <= 0 && next_pixel.x <= end.x && direction.y <= 0 && next_pixel.y <= end.y) ||
                        (next_pixel.x == end.x && next_pixel.y == end.y)) {
                        break;
                    }

                    // Check for obstacle in the stepped pixel
                    Color step_color = *image_pixel(scene, next_pixel.x, next_pixel.y);
                    if (is_obstacle(step_color)) {
                        occluded = 1;
                        break;
                    }
                }

                if (!occluded) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                }
            }

            *image_pixel(partial, x, y) = total_illum;
        }
    }
    return NULL;
}

Image* raycast_parallel_lights(Image* scene, Light* lights, int light_count, int max_threads) {
    if (light_count == 0) {
        return new_image(scene->width, scene->height);
    }

    int num_threads = (max_threads < light_count) ? max_threads : light_count;

    int lights_per_thread = light_count / num_threads;
    int remainder = light_count % num_threads;

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadDataLights* thread_data = malloc(num_threads * sizeof(ThreadDataLights));

    for (int i = 0; i < num_threads; i++) {
        int start_light = i * lights_per_thread + (i < remainder ? 1 : 0);
        int end_light = start_light + lights_per_thread + (i < remainder ? 1 : 0);

        Image* partial = new_image(scene->width, scene->height);

        thread_data[i] = (ThreadDataLights){
            .scene = scene,
            .lights = lights,
            .start_light = start_light,
            .end_light = end_light,
            .partial_illum = partial
        };

        pthread_create(&threads[i], NULL, parallel_lights_worker, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Combine partial illumination results
    Image* final_illum = new_image(scene->width, scene->height);
    for (int y = 0; y < scene->height; y++) {
        for (int x = 0; x < scene->width; x++) {
            *image_pixel(final_illum, x, y) = (Color){ 0, 0, 0 };
        }
    }

    for (int i = 0; i < num_threads; i++) {
        Image* partial = thread_data[i].partial_illum;
        for (int y = 0; y < scene->height; y++) {
            for (int x = 0; x < scene->width; x++) {
                Color combined = add_colors(*image_pixel(final_illum, x, y), *image_pixel(partial, x, y));
                *image_pixel(final_illum, x, y) = combined;
            }
        }
        free(partial->pixels);
        free(partial);
    }

    // Multiply by the original scene colors
    Image* result = new_image(scene->width, scene->height);
    for (int y = 0; y < scene->height; y++) {
        for (int x = 0; x < scene->width; x++) {
            Color orig = *image_pixel(scene, x, y);
            Color illum = *image_pixel(final_illum, x, y);
            *image_pixel(result, x, y) = mul_colors(illum, orig);
        }
    }

    // Clean up
    free(final_illum->pixels);
    free(final_illum);
    free(threads);
    free(thread_data);

    return result;
}

typedef struct {
    Image* scene;
    Light* lights;
    int light_count;
    int start_row;
    int end_row;   // end_row is exclusive
    Image* result; // Each thread writes directly into this final image
} ThreadDataRows;

static void* parallel_rows_worker(void* arg) {
    ThreadDataRows* data = (ThreadDataRows*)arg;
    Image* scene = data->scene;
    Light* lights = data->lights;
    int light_count = data->light_count;
    Image* result = data->result;

    for (int y = data->start_row; y < data->end_row; y++) {
        for (int x = 0; x < scene->width; x++) {
            Color orig = *image_pixel(scene, x, y);
            if (is_obstacle(orig)) {
                // obstacle pixels remain unchanged
                *image_pixel(result, x, y) = orig;
                continue;
            }

            // accumulate illumination from all lights
            Color total_illum = (Color){ 0, 0, 0 };
            for (int l = 0; l < light_count; l++) {
                Light current_light = lights[l];

                // If the pixel is the light source itself, it's always illuminated
                if (x == current_light.pixel.x && y == current_light.pixel.y) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                    continue;
                }

                // determine direction from pixel to the light
                PixelLocation start = { x, y };
                PixelLocation end = { current_light.pixel.x, current_light.pixel.y };
                Pair direction = direction_pair(start, end);

                Pair pos = { (double)x, (double)y };
                int occluded = 0;

                // trace the ray towards the light
                while (1) {
                    PixelLocation next_pixel = step(&pos, direction);

                    // check if we have reached or passed the light
                    if ((direction.x > 0 && next_pixel.x > end.x) ||
                        (direction.x < 0 && next_pixel.x < end.x) ||
                        (direction.y > 0 && next_pixel.y > end.y) ||
                        (direction.y < 0 && next_pixel.y < end.y) ||
                        (next_pixel.x == end.x && next_pixel.y == end.y)) {
                        // reached the light
                        break;
                    }

                    // Check if new pixel is an obstacle
                    Color step_color = *image_pixel(scene, next_pixel.x, next_pixel.y);
                    if (is_obstacle(step_color)) {
                        occluded = 1;
                        break;
                    }
                }

                // if not occluded, add light's contribution
                if (!occluded) {
                    Color illum = illuminate(current_light, x, y);
                    total_illum = add_colors(total_illum, illum);
                }
            }

            // multiply original pixel color by total illumination
            *image_pixel(result, x, y) = mul_colors(total_illum, orig);
        }
    }

    return NULL;
}

Image* raycast_parallel_rows(Image* scene, Light* lights, int light_count, int max_threads) {
    if (light_count == 0) {
        return new_image(scene->width, scene->height);
    }

    int num_threads = (max_threads < scene->height) ? max_threads : scene->height;
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadDataRows* thread_data = malloc(num_threads * sizeof(ThreadDataRows));

    Image* result = new_image(scene->width, scene->height);

    int rows_per_thread = scene->height / num_threads;
    int remainder = scene->height % num_threads;

    int current_start = 0;
    for (int i = 0; i < num_threads; i++) {
        int rows_for_this_thread = rows_per_thread + (i < remainder ? 1 : 0);
        int start_row = current_start;
        int end_row = start_row + rows_for_this_thread;
        current_start = end_row;

        thread_data[i] = (ThreadDataRows){
            .scene = scene,
            .lights = lights,
            .light_count = light_count,
            .start_row = start_row,
            .end_row = end_row,
            .result = result
        };

        pthread_create(&threads[i], NULL, parallel_rows_worker, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(thread_data);

    return result;
}
