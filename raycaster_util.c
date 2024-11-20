#include <math.h>

#include "raycaster_util.h"

int is_obstacle(Color color) {
    return (color.blue + color.red + color.green < 10) ? 1 : 0;
}

// Compute the absolute difference between unsigned numbers.
unsigned int absdiff(unsigned int a, unsigned int b) {
    return a > b ? a - b : b - a;
}

int adjacent_pixels(PixelLocation pixel1, PixelLocation pixel2) {
    return absdiff(pixel1.x, pixel2.x) <= 1 && absdiff(pixel1.y, pixel2.y) <= 1
               ? 1
               : 0;
}

Pair center_point(int x, int y) {
    return (Pair){x + 0.5, y + 0.5};
}

// helper function for in_pixel logic
// returns 1 if the test is "within" the pixel value
int within_one(double test, int value) {
    return (test >= value - EPS && test <= value + 1 + EPS) ? 1 : 0;
}

int in_pixel(Pair point, PixelLocation pixel) {
    int result = 0;
    result += within_one(point.x, pixel.x);
    result += within_one(point.y, pixel.y);
    return (result == 2) ? 1 : 0;
}

Pair direction_pair(PixelLocation start, PixelLocation end) {
    if (start.x == end.x) {
        return (Pair){0.0, end.y < start.y ? -1.0 : 1.0};
    }
    if (start.y == end.y) {
        return (Pair){end.x < start.x ? -1.0 : 1.0, 0.0};
    }
    Pair spair = center_point(start.x, start.y);
    Pair epair = center_point(end.x, end.y);
    double direction = atan2((epair.y - spair.y), (epair.x - spair.x));
    return (Pair){cos(direction), sin(direction)};
}

// The `step` function implementation is subtle. Here is some pseudocode for
// the high-level idea:
//
// fun step(x, y, direction) {
//   // Assume the pixel boundaries are integers, so ceil and floor find
//   // these edges.
//   x_bound = if direction.x > 0 then ceil(x) else floor(x)
//   y_bound = if direction.y > 0 then ceil(y) else floor(y)
//   if abs(x - x_bound) * direction.x < abs(y - y_bound) * direction.y
//     x = x_bound
//     y += direction.y / abs((x - x_bound) * direction.x)
//   else
//     x += direction.x / abs((y - y_bound) * direction.y)
//     y = y_bound
// }

PixelLocation step(Pair* pos, Pair direction) {
    int direction_x_sign = (direction.x < 0) ? -1 : 1;
    int direction_y_sign = (direction.y < 0) ? -1 : 1;

    double x_adjust = pos->x;
    double y_adjust = pos->y;
    // step forward just slightly to distance from the integer representation
    if (fabs(x_adjust - round(x_adjust)) < EPS) {
        x_adjust += direction_x_sign * (EPS * 2);
    }
    if (fabs(y_adjust - round(y_adjust)) < EPS) {
        y_adjust += direction_y_sign * (EPS * 2);
    }

    // Calculate the difference between the x and y and the next pixel boundary
    int x_bound = (direction.x < 0) ? floor(x_adjust) : ceil(x_adjust);
    double x_gap = x_bound - x_adjust;

    int y_bound = (direction.y < 0) ? floor(y_adjust) : ceil(y_adjust);
    double y_gap = y_bound - y_adjust;

    // Very small x and y direction calculations
    if (fabs(direction.x) < EPS) {
        pos->y = y_bound;
        return (PixelLocation){(int)pos->x, pos->y};
    }
    if (fabs(direction.y) < EPS) {
        pos->x = x_bound;
        return (PixelLocation){pos->x, (int)pos->y};
    }

    // find the closer boundary and step to it
    // note that we need to return the current pixel for the other component
    if (fabs(x_gap / direction.x) < fabs(y_gap / direction.y)) {
        pos->x = x_bound;
        pos->y += direction.y * fabs(x_gap / direction.x);

        // case of the pixel diagonal
        // slightly larger epsilon bound to make angles "feel right"
        // also avoids issues with the adjustments made earlier
        if (fabs(pos->y - round(pos->y)) < EPS * 8) {
            pos->y = y_bound;
            return (PixelLocation){pos->x, pos->y};
        }
        return (PixelLocation){pos->x, y_bound - direction_y_sign};
    } else {
        pos->x += direction.x * fabs(y_gap / direction.y);
        pos->y = y_bound;
        if (fabs(pos->x - round(pos->x)) < EPS * 8) {
            pos->x = x_bound;
            return (PixelLocation){pos->x, pos->y};
        }
        return (PixelLocation){x_bound - direction_x_sign, pos->y};
    }
}

Color illuminate(Light light, int x, int y) {
    int x_dist = x - light.pixel.x;
    x_dist = x_dist * x_dist;

    int y_dist = y - light.pixel.y;
    y_dist = y_dist * y_dist;

    double illumination = exp(-(x_dist + y_dist) / light.strength);

    return scale_color(light.color, illumination);
}
