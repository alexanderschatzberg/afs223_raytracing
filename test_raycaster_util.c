#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "raycaster_util.h"

// Utility functions

/*
 * Returns 1 (true) if two numbers are "almost equal" (within .0001)
 *  Otherwise prints an error and returns 0
 *
 * Note that we assume fairly small numbers in these tests
 *  so this function is written to work best with small-ish floats
 *  large floating-point values will not work at all
 */
int double_almost_equal(const char* context, double expected, double result) {
    // special case stuff
    if (expected == result) {
        return 0;
    }

    // use a larger error threshold than normal for "almost equal"
    if (fabs(expected - result) < 1e-4) {
        return 0;
    }

    printf("%s: expected around %lf, got %lf\n", context, expected, result);
    return 1;
}

/*
 * Returns 0 if expected and result are almost equal, and a number >0 otherwise
 * Differences between expected and result are printed via double_almost_equal
 */
int pair_almost_equal(const char* context, Pair expected, Pair result) {
    int errors = 0;
    errors += double_almost_equal(context, expected.x, result.x);
    errors += double_almost_equal(context, expected.y, result.y);
    return errors;
}

/*
 * Returns 0 if expected and result are almost equal, and a number >0 otherwise
 * Expects a precisely equal result
 */
int pixel_location_equal(const char* context, PixelLocation expected,
                         PixelLocation result) {
    if (expected.x == result.x && expected.y == result.y) {
        return 0;
    }
    printf("%s: expected Location (%d, %d), got (%d, %d)\n", context,
           expected.x, expected.y, result.x, result.y);
    return 1;
}

/*
 * Returns 0 if expected and result are almost equal, and a number >0 otherwise
 * Allows for small differences to account for implementation details changing
 */
int color_almost_equal(const char* context, Color expected, Color result) {
    int total_diff = 0;
    total_diff += abs(expected.red - result.red);
    total_diff += abs(expected.green - result.green);
    total_diff += abs(expected.blue - result.blue);
    if (total_diff < 6) // average difference of 2 per component
    {
        return 0;
    }
    printf("%s: expected Color around (%d, %d, %d), got (%d, %d, %d)\n",
           context, expected.red, expected.green, expected.blue, result.red,
           result.green, result.blue);
    return 1;
}

// Tests

/*
 * Helper function to make error counting easier for in_pixel
 */
int is_obstacle_check(int test, int expected, Color color) {
    int result = is_obstacle(color);

    if (result == expected) {
        return 0;
    }
    printf("Test %d for is_obstacle: expected %d, got %d\n", test, expected,
           result);
    return 1;
}

/*
 * Tests is_obstacle
 */
int test_is_obstacle(void) {
    int errors = 0;

    errors += is_obstacle_check(0, 1, (Color){0, 0, 0});
    errors += is_obstacle_check(1, 1, (Color){3, 2, 1});
    errors += is_obstacle_check(2, 1, (Color){0, 0, 9});
    errors += is_obstacle_check(3, 0, (Color){3, 4, 3});
    errors += is_obstacle_check(4, 0, (Color){0, 10, 0});
    errors += is_obstacle_check(5, 0, (Color){255, 0, 0});

    return errors;
}

/*
 * Helper function to make error counting easier for adjacent_pixel
 */
int adjacent_pixels_check(int test, int expected, PixelLocation pixel1,
                          PixelLocation pixel2) {
    int result = adjacent_pixels(pixel1, pixel2);

    if (result == expected) {
        return 0;
    }
    printf("Test %d for adjacent_pixel: expected %d, got %d\n", test, expected,
           result);
    return 1;
}

/*
 * Tests adjacent_pixel
 */
int test_adjacent_pixel(void) {
    int errors = 0;

    // Adjacent pixels
    errors += adjacent_pixels_check(0, 1, (PixelLocation){7, 10},
                                    (PixelLocation){7, 10});
    errors += adjacent_pixels_check(1, 1, (PixelLocation){0, 1},
                                    (PixelLocation){1, 1});
    errors += adjacent_pixels_check(2, 1, (PixelLocation){2, 1},
                                    (PixelLocation){1, 1});
    errors += adjacent_pixels_check(3, 1, (PixelLocation){1, 1},
                                    (PixelLocation){1, 0});
    errors += adjacent_pixels_check(3, 1, (PixelLocation){1, 1},
                                    (PixelLocation){1, 2});

    // Not adjacent pixels
    errors += adjacent_pixels_check(4, 1, (PixelLocation){4, 5},
                                    (PixelLocation){5, 4});
    errors += adjacent_pixels_check(5, 1, (PixelLocation){1, 1},
                                    (PixelLocation){0, 0});
    errors += adjacent_pixels_check(6, 0, (PixelLocation){1, 0},
                                    (PixelLocation){1, 2});

    return errors;
}

/*
 * Helper function to make error counting easier for center_point
 */
int center_point_check(int test, Pair expected, int x, int y) {
    char context[30];
    snprintf(context, 30, "Test %d for center_pooint", test);

    Pair result = center_point(x, y);

    int errors = pair_almost_equal(context, expected, result);
    return errors;
}

/*
 * Tests center_point
 */
int test_center_point(void) {
    int errors = 0;

    errors += center_point_check(0, (Pair){1.5, 1.5}, 1, 1);
    errors += center_point_check(1, (Pair){17.5, 0.5}, 17, 0);

    return errors;
}

/*
 * Helper function to make error counting easier for in_pixel
 */
int in_pixel_check(int test, int expected, Pair point, PixelLocation pixel) {
    int result = in_pixel(point, pixel);

    if (result == expected) {
        return 0;
    }
    printf("Test %d for in_pixel: expected %d, got %d\n", test, expected,
           result);
    return 1;
}

/*
 * Tests in_pixel
 */
int test_in_pixel(void) {
    int errors = 0;

    // Valid points
    in_pixel_check(0, 1, (Pair){1.5, 1.5}, (PixelLocation){1, 1});
    in_pixel_check(1, 1, (Pair){10.7, 7.01}, (PixelLocation){10, 7});
    in_pixel_check(2, 1, (Pair){4.99999999999, 5.000000001},
                   (PixelLocation){5, 4});

    // Invalid points
    in_pixel_check(3, 0, (Pair){4.7, 5.3}, (PixelLocation){5, 4});
    in_pixel_check(4, 0, (Pair){3.01, 5.5}, (PixelLocation){2, 5});
    in_pixel_check(5, 0, (Pair){3.5, 2.99}, (PixelLocation){3, 3});
    in_pixel_check(6, 0, (Pair){4.99999999999, 5.000000001},
                   (PixelLocation){6, 5});

    return errors;
}

/*
 * Helper function to make error counting easier for direction
 */
int direction_pair_check(int test, Pair expected, PixelLocation start,
                         PixelLocation end) {
    char context[30];
    snprintf(context, 30, "Test %d for direction pair", test);

    Pair result = direction_pair(start, end);

    int errors = pair_almost_equal(context, expected, result);
    return errors == 0 ? 0 : 1;
}

/*
 * Tests direction_pair
 */
int test_direction_pair(void) {
    int errors = 0;

    // quadrants
    errors +=
        direction_pair_check(0, (Pair){0.707107, 0.707107},
                             (PixelLocation){0, 0}, (PixelLocation){1, 1});
    errors +=
        direction_pair_check(1, (Pair){-0.707107, -0.707107},
                             (PixelLocation){1, 1}, (PixelLocation){0, 0});
    errors +=
        direction_pair_check(2, (Pair){-0.707107, 0.707107},
                             (PixelLocation){1, 0}, (PixelLocation){0, 1});
    errors +=
        direction_pair_check(3, (Pair){0.707107, -0.707107},
                             (PixelLocation){0, 1}, (PixelLocation){1, 0});

    // straight lines
    errors += direction_pair_check(4, (Pair){0.0, 1.0}, (PixelLocation){1, 0},
                                   (PixelLocation){1, 1});
    errors += direction_pair_check(5, (Pair){0.0, -1.0}, (PixelLocation){1, 2},
                                   (PixelLocation){1, 1});
    errors += direction_pair_check(6, (Pair){1.0, 0.0}, (PixelLocation){0, 2},
                                   (PixelLocation){1, 2});
    errors += direction_pair_check(7, (Pair){-1.0, 0.0}, (PixelLocation){3, 2},
                                   (PixelLocation){2, 2});

    // Angles
    errors +=
        direction_pair_check(8, (Pair){-0.894427, -0.447214},
                             (PixelLocation){2, 1}, (PixelLocation){0, 0});
    errors +=
        direction_pair_check(9, (Pair){0.948683, -0.316228},
                             (PixelLocation){0, 1}, (PixelLocation){3, 0});
    errors +=
        direction_pair_check(10, (Pair){-0.351123, 0.936329},
                             (PixelLocation){45, 0}, (PixelLocation){30, 40});

    return errors;
}

/*
 * Helper function to make error counting easier for step
 */
int step_check(int test, PixelLocation pixel_exp, Pair loc_exp, Pair start,
               Pair direction) {
    char context[20];
    snprintf(context, 20, "Test %d for step", test);

    PixelLocation result = step(&start, direction);

    int errors = 0;
    errors += pixel_location_equal(context, pixel_exp, result);
    errors += pair_almost_equal(context, loc_exp, start);
    return errors == 0 ? 0 : 1;
}

/*
 * Tests step
 */
int test_step(void) {
    int errors = 0;

    // quadrants
    errors += step_check(0, (PixelLocation){0, 1}, (Pair){0, 1}, (Pair){0, 0},
                         (Pair){0, 1});
    errors += step_check(1, (PixelLocation){0, 2}, (Pair){0, 2}, (Pair){0, 2.5},
                         (Pair){0, -1});
    errors += step_check(2, (PixelLocation){1, 1}, (Pair){1, 1.3},
                         (Pair){.4, 1.3}, (Pair){1, 0});
    errors += step_check(3, (PixelLocation){2, 1}, (Pair){2, 1.9},
                         (Pair){2.99, 1.9}, (Pair){-1, 0});

    // Variety of cases
    errors += step_check(4, (PixelLocation){1, 0}, (Pair){1, 0.4}, (Pair){0, 0},
                         (Pair){0.928477, 0.371391});
    errors += step_check(5, (PixelLocation){0, 1}, (Pair){0.5, 1},
                         (Pair){0, 1.2}, (Pair){0.928477, -0.371391});
    errors += step_check(6, (PixelLocation){4, 4}, (Pair){4, 4}, (Pair){5, 5},
                         (Pair){-0.707107, -0.707107});
    errors += step_check(7, (PixelLocation){11, 15}, (Pair){10.192893, 15},
                         (Pair){10.9, 14}, (Pair){-0.57735027, 0.81649658});

    return errors;
}

/*
 * Helper function to make error counting easier for direction
 */
int illuminate_check(int test, Color expected, Light light, int x, int y) {
    char context[30];
    snprintf(context, 30, "Test %d for illuminate", test);

    Color result = illuminate(light, x, y);

    int errors = color_almost_equal(context, expected, result);
    return errors == 0 ? 0 : 1;
}

/*
 * Test illuminate
 */
int test_illuminate(void) {
    int errors = 0;

    // No distance
    illuminate_check(
        0, (Color){255, 255, 255},
        (Light){(Color){255, 255, 255}, 1., (PixelLocation){5, 10}}, 5, 10);
    illuminate_check(1, (Color){128, 0, 0},
                     (Light){(Color){128, 0, 0}, 1., (PixelLocation){30, 10}},
                     30, 10);

    // Long distance, weak light
    illuminate_check(
        2, (Color){0, 0, 0},
        (Light){(Color){255, 255, 255}, 100., (PixelLocation){1000, 0}}, 0,
        1000);
    illuminate_check(3, (Color){0, 0, 0},
                     (Light){(Color){255, 255, 255}, 5., (PixelLocation){0, 0}},
                     100, 100);
    illuminate_check(
        4, (Color){0, 0, 0},
        (Light){(Color){255, 255, 255}, 5., (PixelLocation){100, 100}}, 0, 100);

    // Close light
    illuminate_check(
        5, (Color){190, 95, 0},
        (Light){(Color){200, 100, 0}, 20., (PixelLocation){10, 20}}, 10, 21);
    illuminate_check(6, (Color){18, 9, 27},
                     (Light){(Color){20, 10, 30}, 20., (PixelLocation){1, 0}},
                     0, 1);

    // Far light, strong
    illuminate_check(
        7, (Color){71, 71, 181},
        (Light){(Color){100, 100, 255}, 10000., (PixelLocation){100, 100}}, 150,
        130);
    illuminate_check(
        8, (Color){184, 144, 184},
        (Light){(Color){255, 200, 255}, 100000., (PixelLocation){200, 200}}, 50,
        300);

    return errors;
}

int main(void) {
    int errors;
    errors = test_is_obstacle();
    printf("\n");
    printf("test_is_obstacle %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
    errors = test_adjacent_pixel();
    printf("\n");
    printf("test_adjacent_pixel %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
    errors = test_center_point();
    printf("\n");
    printf("test_center_point %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
    errors = test_direction_pair();
    printf("\n");
    printf("test_direction_pair %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
    errors = test_step();
    printf("\n");
    printf("test_step %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
    errors = test_illuminate();
    printf("\n");
    printf("test_illuminate %s with %d failing tests\n",
           errors == 0 ? "passed" : "failed", errors);
    printf("\n");
}
