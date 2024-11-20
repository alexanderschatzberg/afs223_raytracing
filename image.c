#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Image* read_image(const char* filename) {
    int width, height, bpp;

    uint8_t* rgb_image = stbi_load(filename, &width, &height, &bpp, CHANNELS);

    Color* pixels = (Color*)malloc(sizeof(Color) * width * height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = i * width + j;
            pixels[index].red = rgb_image[CHANNELS * index + 0];
            pixels[index].green = rgb_image[CHANNELS * index + 1];
            pixels[index].blue = rgb_image[CHANNELS * index + 2];
        }
    }

    stbi_image_free(rgb_image);

    Image* image = (Image*)malloc(sizeof(Image));
    image->pixels = pixels;
    image->width = width;
    image->height = height;

    return image;
}

void write_image(const char* filename, Image* image) {
    uint8_t* rgb_image = (uint8_t*)malloc(sizeof(uint8_t) * image->width *
                                          image->height * CHANNELS);

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int index = i * image->width + j;
            rgb_image[CHANNELS * index + 0] =
                image->pixels[index].red;
            rgb_image[CHANNELS * index + 1] =
                image->pixels[index].green;
            rgb_image[CHANNELS * index + 2] =
                image->pixels[index].blue;
        }
    }

    stbi_write_png(filename, image->width, image->height, CHANNELS, rgb_image,
                   image->width * CHANNELS);

    free(rgb_image);
}

void free_image(Image* image) {
    free(image->pixels);
    free(image);
}

Image* new_image(int width, int height) {
    Color* pixels = (Color*)calloc(width * height, sizeof(Color));

    Image* image = (Image*)malloc(sizeof(Image));
    image->pixels = pixels;
    image->width = width;
    image->height = height;

    return image;
}

Color* image_pixel(Image* image, int x, int y) {
    return &image->pixels[y * image->width + x];
}

/*
 * Returns a component of a color given the normalized 0-1 representation
 */
uint8_t normalized_to_color(float component) {
    float clamped = fmin(1.0, fmax(0.0, component));
    return 255 * clamped;
}

Color add_colors(Color color1, Color color2) {
    float red = fmin(color1.red + color2.red, 255.0);
    float green = fmin(color1.green + color2.green, 255.0);
    float blue = fmin(color1.blue + color2.blue, 255.0);
    return (Color){red, green, blue};
}

Color mul_colors(Color color1, Color color2) {
    float r = color1.red / 255.0f * color2.red / 255.0f;
    float g = color1.green / 255.0f * color2.green / 255.0f;
    float b = color1.blue / 255.0f * color2.blue / 255.0f;
    uint8_t red = normalized_to_color(r);
    uint8_t green = normalized_to_color(g);
    uint8_t blue = normalized_to_color(b);
    return (Color){red, green, blue};
}

Color scale_color(Color color, float scale) {
    float red = fmin(color.red * scale, 255.0);
    float green = fmin(color.green * scale, 255.0);
    float blue = fmin(color.blue * scale, 255.0);
    return (Color){red, green, blue};
}
