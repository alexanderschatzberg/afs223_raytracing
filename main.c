#include <stdio.h>

#include "image.h"
#include "raycaster.h"

// Constants to help with trying stuff out
#define WHITE (Color){255, 255, 255}
#define RED (Color){255, 0, 0}
#define GREEN (Color){0, 255, 0}
#define BLUE (Color){0, 0, 255}

int main(void) {
    Image* image = read_image("images/tiny.png");
    printf("Image size: %d %d\n", image->width, image->height);

    PixelLocation loc = (PixelLocation){5, 5};
    Light light = (Light){WHITE, 70000.0, loc};

    Light lights[] = {light};
    Image* out = raycast_sequential(image, lights, 1);

    write_image("raycast.png", out);
    free_image(image);
    free_image(out);

    printf("Finished\n");

    return 0;
}
