#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_header.h"
#include "functions.h"

int main()
{
    char input_filename[] = "input.txt";

    FILE *in = fopen(input_filename, "rt");
    if (in == NULL) {
        fprintf(stderr, "ERROR: Can't open file %s\n", input_filename);
        return -1;
    }

    char *bmp_filename = malloc(50);
    fscanf(in, "%s", bmp_filename);

    int threshold;
    fscanf(in, "%d", &threshold);

    char *bin_filename = malloc(50);
    fscanf(in, "%s", bin_filename);

    fclose(in);

    FILE *bmp = fopen(bmp_filename, "rb");
    if (bmp == NULL) {
        fprintf(stderr, "ERROR: Can't open file %s\n", bmp_filename);
        free(bmp_filename);
        free(bin_filename);
        return -1;
    }

    fileheader bmp_fileheader;
    fread(&bmp_fileheader, sizeof(bmp_fileheader), 1, bmp);

    infoheader bmp_infoheader;
    fread(&bmp_infoheader, sizeof(bmp_infoheader), 1, bmp);

    signed int width = bmp_infoheader.width, height = bmp_infoheader.height;
    unsigned int offset = bmp_fileheader.imageDataOffset;

    pixel **original = get_img(width, height);

    if (original == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        fclose(bmp);
        free(bmp_filename);
        free(bin_filename);
        return -1;
    }

    fseek(bmp, offset, SEEK_SET);

    read_img(original, width, height, bmp);

    fclose(bmp);

    char *new_filename;
    pixel **img = copy_img(original, width, height);

    if (img == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        free(bmp_filename);
        free(bin_filename);
        return -1;
    }


    //Task 1

    new_filename = edit_filename(bmp_filename, "_black_white.bmp");

    write_img(bmp_fileheader, bmp_infoheader,
                black_white_filter(img, width, height),  
                width, height, new_filename);

    free(new_filename);


    //Task 2

    int mat[3][3];
    pixel **filtered_img;

    //F1

    mat[2][2] = 8;
    mat[1][1] = mat[1][2] = mat[1][3] = mat[2][1] = mat[2][3] = -1;
    mat[3][1] = mat[3][2] = mat[3][3] = -1;

    filtered_img = filter(img, width, height, mat);
    new_filename = edit_filename(bmp_filename, "_f1.bmp");

    write_img(bmp_fileheader, bmp_infoheader, filtered_img,
                width, height, new_filename);

    free_img(&filtered_img, height);
    free(new_filename);

    //F2

    mat[2][2] = -4;
    mat[1][1] = mat[1][3] = mat[3][1] = mat[3][3] = 0;
    mat[1][2] = mat[2][1] = mat[2][3] = mat[3][2] = 1;

    filtered_img = filter(img, width, height, mat);
    new_filename = edit_filename(bmp_filename, "_f2.bmp");

    write_img(bmp_fileheader, bmp_infoheader, filtered_img, 
                width, height, new_filename);

    free_img(&filtered_img, height);
    free(new_filename);

    //F3

    mat[1][1] = mat[3][3] = 1;
    mat[1][3] = mat[3][1] = -1;
    mat[1][2] = mat[2][1] = mat[2][2] = mat[2][3] = mat[3][2] = 0;

    filtered_img = filter(img, width, height, mat);
    new_filename = edit_filename(bmp_filename, "_f3.bmp");

    write_img(bmp_fileheader, bmp_infoheader, filtered_img, 
                width, height, new_filename);

    free_img(&filtered_img, height);
    free(new_filename);

    //Task 3

    int i, j, n = 1000, zone = 0;

    pixel *zone_colors = malloc(n * sizeof(pixel)); //the color of each zone
    if (zone_colors == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        free(bmp_filename);
        free(bin_filename);
        return -1;
    }

    int **zones = get_matrix(width, height);
    if (zones == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        free(bmp_filename);
        free(bin_filename);
        free(zone_colors);
        return -1;
    }

    coord start;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (zones[i][j] == 0) {
                zone++;

                if (zone > n - 1) {
                    n += 1000;
                    zone_colors = realloc(zone_colors, n * sizeof(pixel));
                    if (zone_colors == NULL) {
                        fprintf(stderr, "ERROR: Alloc failed\n");
                        free(bmp_filename);
                        free(bin_filename);
                        free(zone_colors);
                        free_matrix(&zones, height);
                        return -1;
                    }
                }

                zone_colors[zone] = original[i][j]; //saves new zone color

                start.x = i; start.y = j;

                assign_zone(original, zones, width, height, start, 
                            zone_colors[zone], zone, threshold);
            }
        }
    }

    pixel **compressed = get_img(width, height);
    if (compressed == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        free(bmp_filename);
        free(bin_filename);
        free(zone_colors);
        free_matrix(&zones, height);
        return -1;
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++){
            compressed[i][j] = zone_colors[zones[i][j]];
        }
    }

    //write_img(bmp_fileheader, bmp_infoheader, compressed, width, height, 
    //            "compressed.bmp");
    write_bin(bmp_fileheader, bmp_infoheader, compressed, width, height);

    free_matrix(&zones, height);
    free(zone_colors);
    free_img(&compressed, height);

    //Task 4

    decompress(bin_filename);


    free_img(&original, height);
    free_img(&img, height);

    free(bmp_filename);
    free(bin_filename);
    return 0;
}
