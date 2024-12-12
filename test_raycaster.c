#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "raycaster.h"
#include "raycaster_util.h"

// Color definitions

#define WHITE (Color){255, 255, 255}
#define YELLOW (Color){255, 255, 50}
#define BLUE (Color){0, 0, 255}
#define MAGENTA (Color){230, 50, 220}

// Utility functions

/*
 * Utility structure to store results of test case setup
 */
typedef struct {
    Image* image;
    Light* lights;
    int light_count;
    Image* expected;
    const char* expected_filename;
    const char* out_filename;
} RaycastTest;

/**
 * Create a new RaycastTest. The `lights` array is allocated but remains
 * uninitialized.
 */
RaycastTest* make_test(const char* input_image, const char* expected_image,
    const char* filename, int light_count) {
    RaycastTest* test = malloc(sizeof(RaycastTest));
    test->image = read_image(input_image);
    test->lights = malloc(sizeof(Light) * light_count);
    test->light_count = light_count;
    test->expected = read_image(expected_image);
    test->expected_filename = expected_image;
    test->out_filename = filename;
    return test;
}

/**
 * Deallocate a RaycastTest.
 */
void free_test(RaycastTest* test) {
    free_image(test->expected);
    free_image(test->image);
    free(test->lights);
    free(test);
}

/**
 * Compute the absolute difference between two `uint8_t`s.
 */
uint8_t absdiff8(uint8_t a, uint8_t b) {
    return a > b ? a - b : b - a;
}

/*
 * Returns 0 if the two given colors are "almost equal", and 1 otherwise
 *  Also prints an error if the two colors are _not_ almost equal
 *
 * We consider component distance to be the sum of absolute difference of two
 * colors Any distance of 10 or less are considered to be "equal" colors
 */
char colors_almost_equal(int test, int x, int y, Color expected, Color result) {
    int red = absdiff8(expected.red, result.red);
    int green = absdiff8(expected.green, result.green);
    int blue = absdiff8(expected.blue, result.blue);
    if ((red + green + blue) < 10) {
        return 0;
    }
    return 1;
}

/*
 * Check that an actual image output is "almost equal" to an expected image.
 * Return 0 if the images match; return 1 *and print an error* otherwise.
 *
 * Two images are considered almost equal if 90% of pixels are almost equal.
 * Pixels are almost equal according to `colors_almost_equal`.
 *
 * The expected image is taken from `info`; the actual image is a separate
 * parameter. The test number * (`test`) and expected filename are used only for
 * printing error messages.
 */
char image_almost_equal(RaycastTest* info, int test, Image* actual, const char* actual_path) {
    Image* expected = info->expected;
    if (expected->height != actual->height ||
        expected->width != actual->width) {
        printf("Test %d: ", test);
        printf("expected an image of size %dx%d, ", expected->width,
            expected->height);
        printf("got an image of size %dx%d", actual->width, actual->height);
        return 1;
    }

    // make the mismatch count a long to avoid overflow in extreme cases
    unsigned long mismatch_count = 0;
    for (unsigned int y = 0; y < expected->height; y++) {
        for (unsigned int x = 0; x < expected->width; x++) {
            Color expected_color = *image_pixel(expected, x, y);
            Color actual_color = *image_pixel(actual, x, y);
            mismatch_count += colors_almost_equal(test, x, y, expected_color, actual_color);
        }
    }
    unsigned long pixel_count = expected->width * info->expected->height;
    // We consider an image incorrect if more than 10% of pixels are not almost
    // equal
    if (mismatch_count * 10 > pixel_count) {
        printf("Test %d failed: %ld/%ld pixels differ\n", test, mismatch_count,
            pixel_count);
        printf("  expected: %s\n", info->expected_filename);
        printf("  actual:   %s\n", actual_path);
        return 1;
    }
    return 0;
}

// Test Case Setups

/*
 * Test for a tiny image with one light
 */
RaycastTest* test_tiny(void) {
    RaycastTest* result =
        make_test("images/tiny.png", "images/test_references/tiny_test.png",
            "tiny_test", 1);
    result->lights[0] = (Light){ WHITE, 50.0, (PixelLocation) { 8, 2 } };
    return result;
}

/*
 * Test for a small image with one light
 */
RaycastTest* test_small(void) {
    RaycastTest* result =
        make_test("images/small.png", "images/test_references/small_test.png",
            "small_test", 1);
    result->lights[0] = (Light){ YELLOW, 400.0, (PixelLocation) { 1, 30 } };
    return result;
}

/*
 * Test for a small image with two semi-overlapping lights
 */
RaycastTest* test_small_2_light(void) {
    RaycastTest* result = make_test(
        "images/small.png", "images/test_references/small_test_2_light.png",
        "small_test_2_light", 2);
    result->lights[0] = (Light){ BLUE, 200.0, (PixelLocation) { 30, 2 } };
    result->lights[1] = (Light){ MAGENTA, 300.0, (PixelLocation) { 15, 20 } };
    return result;
}

/*
 * Test for a small image with four lights
 */
RaycastTest* test_small_4_light(void) {
    RaycastTest* result = make_test(
        "images/small.png", "images/test_references/small_test_4_light.png",
        "small_test_4_light", 4);
    result->lights[0] = (Light){ WHITE, 100.0, (PixelLocation) { 1, 12 } };
    result->lights[1] = (Light){ BLUE, 50.0, (PixelLocation) { 30, 30 } };
    result->lights[2] = (Light){ YELLOW, 400.0, (PixelLocation) { 8, 28 } };
    result->lights[3] = (Light){ MAGENTA, 200.0, (PixelLocation) { 27, 4 } };
    return result;
}

/*
 * Test for a long image (non-square)
 */
RaycastTest* test_long(void) {
    RaycastTest* result =
        make_test("images/long.png", "images/test_references/long_test.png",
            "long_test", 4);
    result->lights[0] = (Light){ YELLOW, 7000.0, (PixelLocation) { 100, 12 } };
    result->lights[1] = (Light){ MAGENTA, 4000.0, (PixelLocation) { 50, 30 } };
    result->lights[2] = (Light){ YELLOW, 5000.0, (PixelLocation) { 8, 70 } };
    result->lights[3] = (Light){ MAGENTA, 5000.0, (PixelLocation) { 170, 90 } };
    return result;
}

/*
MY TEST CASES
*/

// single pixel 

RaycastTest* test_single_pixel(void) {
    RaycastTest* result = make_test("images/single_pixel.png",
        "images/test_references/single_pixel_test.png",
        "single_pixel_test",
        1);
    result->lights[0] = (Light){ (Color) { 255, 255, 255 }, 100.0, (PixelLocation) { 0, 0 } };
    return result;
}

// single pixel with obstacle
RaycastTest* test_single_pixel_obstacle(void) {
    RaycastTest* result = make_test("images/single_pixel_obstacle.png",
        "images/test_references/single_pixel_obstacle_test.png",
        "single_pixel_obstacle_test",
        1);
    result->lights[0] = (Light){ (Color) { 255, 255, 255 }, 100.0, (PixelLocation) { 0, 0 } };
    return result;
}

// Image with no lights 
RaycastTest* test_no_lights(void) {
    RaycastTest* result = make_test("images/small.png",
        "images/test_references/small_test_dark.png",
        "no_lights",
        0);
    return result;
}

// cool lights, 4 lights 
RaycastTest* test_cool_lights(void) {
    RaycastTest* result = make_test("images/large_empty.png",
        "images/test_references/cool_lights_test.png",
        "cool_shape",
        4);
    result->lights[0] = (Light){ (Color) { 255, 0, 0 }, 100.0, (PixelLocation) { 0, 0 } };
    result->lights[1] = (Light){ (Color) { 0, 0, 255 }, 100.0, (PixelLocation) { 32, 32 } };
    result->lights[2] = (Light){ (Color) { 0, 255, 0 }, 100.0, (PixelLocation) { 128, 128 } };
    result->lights[3] = (Light){ (Color) { 255, 255, 255 }, 100.0, (PixelLocation) { 300, 300 } };
    return result;
}

// another cool shape 
RaycastTest* test_cool_shape(void) {
    RaycastTest* result = make_test("timingimgs/simplicity2.png",
        "images/test_references/simplicty2_output.png",
        "cool_shape",
        4);
    result->lights[0] = (Light){ (Color) { 255, 0, 0 }, 100.0, (PixelLocation) { 0, 0 } };
    result->lights[1] = (Light){ (Color) { 0, 0, 255 }, 100.0, (PixelLocation) { 32, 32 } };
    result->lights[2] = (Light){ (Color) { 0, 255, 0 }, 100.0, (PixelLocation) { 128, 128 } };
    result->lights[3] = (Light){ (Color) { 255, 255, 255 }, 100.0, (PixelLocation) { 300, 300 } };
    return result;
}


// Tests

/*
 * Helper function for accumulating sequential raycast cases
 * Also writes each case to the given result file
 */
char raycast_sequential_check(int test, RaycastTest* info) {
    Image* out = raycast_sequential(info->image, info->lights, info->light_count);

    char out_name[64];
    snprintf(out_name, 64, "images/sequential_results/%s.png",
        info->out_filename);
    char error = image_almost_equal(info, test, out, out_name);
    write_image(out_name, out);
    free_image(out);

    free_test(info);

    if (!error) {
        printf("raycast_sequential test %d passed\n", test);
    }

    return error;
}

/*
 * Test all sequential raycast cases
 */
int test_raycast_sequential(void) {
    int errors = 0;
    errors += raycast_sequential_check(0, test_tiny());
    errors += raycast_sequential_check(1, test_small());
    errors += raycast_sequential_check(2, test_small_2_light());
    errors += raycast_sequential_check(3, test_small_4_light());
    errors += raycast_sequential_check(4, test_long());

    errors += raycast_sequential_check(5, test_single_pixel());
    errors += raycast_sequential_check(6, test_single_pixel_obstacle());
    errors += raycast_sequential_check(7, test_no_lights());
    errors += raycast_sequential_check(8, test_cool_lights());
    errors += raycast_sequential_check(9, test_cool_shape());
    return errors;
}

/*
 * Helper function for accumulating sequential raycast cases
 * Also writes each case to the give result file
 */
char raycast_parallel_light_check(int test, RaycastTest* info,
    int thread_count) {
    Image* out = raycast_parallel_lights(info->image, info->lights, info->light_count,
        thread_count);

    char out_name[64];
    snprintf(out_name, 64, "images/parallel_light_results/%s.png",
        info->out_filename);
    char error = image_almost_equal(info, test, out, out_name);
    write_image(out_name, out);
    free_image(out);

    free_test(info);

    if (!error) {
        printf("raycast_parallel_light test %d passed\n", test);
    }

    return error;
}

/*
 * Test all parallel light raycast cases
 */
int test_raycast_parallel_light(void) {
    int errors = 0;
    errors += raycast_parallel_light_check(0, test_tiny(), 1);
    errors += raycast_parallel_light_check(1, test_tiny(), 2);
    errors += raycast_parallel_light_check(2, test_small(), 1);
    errors += raycast_parallel_light_check(3, test_small_2_light(), 2);
    errors += raycast_parallel_light_check(4, test_small_4_light(), 4);
    errors += raycast_parallel_light_check(5, test_small_4_light(), 2);
    errors += raycast_parallel_light_check(6, test_long(), 4);

    errors += raycast_parallel_light_check(7, test_single_pixel(), 1);
    errors += raycast_parallel_light_check(8, test_single_pixel_obstacle(), 1);
    errors += raycast_parallel_light_check(9, test_no_lights(), 1);
    errors += raycast_parallel_light_check(10, test_cool_lights(), 4);
    errors += raycast_parallel_light_check(11, test_cool_shape(), 4);

    return errors;
}

/*
 * Helper function for accumulating sequential raycast cases
 * Also writes each case to the give result file
 */
char raycast_parallel_row_check(int test, RaycastTest* info, int thread_count) {
    Image* out = raycast_parallel_rows(info->image, info->lights, info->light_count,
        thread_count);

    char out_name[64];
    snprintf(out_name, 64, "images/parallel_row_results/%s.png",
        info->out_filename);
    char error = image_almost_equal(info, test, out, out_name);
    write_image(out_name, out);
    free_image(out);

    free_test(info);

    if (!error) {
        printf("raycast_parallel_row test %d passed\n", test);
    }

    return error;
}

/*
 * Test all parallel row raycast cases
 */
int test_raycast_parallel_row(void) {
    int errors = 0;
    errors += raycast_parallel_row_check(0, test_tiny(), 1);
    errors += raycast_parallel_row_check(1, test_tiny(), 2);
    errors += raycast_parallel_row_check(2, test_small(), 2);
    errors += raycast_parallel_row_check(3, test_small_2_light(), 4);
    errors += raycast_parallel_row_check(4, test_small_4_light(), 4);
    errors += raycast_parallel_row_check(5, test_small_4_light(), 2);
    errors += raycast_parallel_row_check(6, test_long(), 6);

    errors += raycast_parallel_row_check(7, test_single_pixel(), 1);
    errors += raycast_parallel_row_check(8, test_single_pixel_obstacle(), 1);
    errors += raycast_parallel_row_check(9, test_no_lights(), 1);
    errors += raycast_parallel_row_check(10, test_cool_lights(), 4);
    errors += raycast_parallel_row_check(11, test_cool_shape(), 4);

    return errors;
}

// Run all three test suites.
int main(void) {
    int errors;

    // Test the sequential implementation.
    printf("testing raycast_sequential:\n");
    errors = test_raycast_sequential();
    if (errors == 0) {
        printf("all tests passed\n");
    }
    else {
        printf("failed %d tests\n", errors);
    }

    // Test the light-parallel implementation.
    printf("\ntesting raycast_parallel_light:\n");
    errors = test_raycast_parallel_light();
    if (errors == 0) {
        printf("all tests passed\n");
    }
    else {
        printf("failed %d tests\n", errors);
    }

    // Test the row-parallel implementation.
    printf("\ntesting raycast_parallel_row:\n");
    errors = test_raycast_parallel_row();
    if (errors == 0) {
        printf("all tests passed\n");
    }
    else {
        printf("failed %d tests\n", errors);
    }
}
