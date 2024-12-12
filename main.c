#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "raycaster.h"
#include "raycaster_util.h"

int main(void) {
    // Scene setup identical to test_cool_lights
    Image* scene = read_image("images/large_empty.png");
    Light lights[4];
    lights[0] = (Light){ (Color) { 255, 0, 0 }, 100.0, (PixelLocation) { 0, 0 } };
    lights[1] = (Light){ (Color) { 0, 0, 255 }, 100.0, (PixelLocation) { 32, 32 } };
    lights[2] = (Light){ (Color) { 0, 255, 0 }, 100.0, (PixelLocation) { 128, 128 } };
    lights[3] = (Light){ (Color) { 255, 255, 255 }, 100.0, (PixelLocation) { 300, 300 } };

    int light_count = 4;

    Image* output = raycast_sequential(scene, lights, light_count);
    write_image("images/test_references/cool_lights_test.png", output);

    free_image(scene);
    free_image(output);

    return 0;
}