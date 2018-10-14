typedef struct {
    unsigned char b, g, r;
} pixel;

typedef struct {
    int x, y;
} coord;

int** get_matrix(signed int width, signed int height)
{
    int i, j;
    int **mat;

    mat = malloc(height * sizeof(int *));
    if (mat == NULL) {
        return NULL;
    }

    for (i = 0; i < height; i++) {
        mat[i] = calloc(width, sizeof(int));
        if (mat[i] == NULL) {
            for (j = 0; j < i; j++) {
                free(mat[j]);
            }

            free(mat);
            return NULL;
        }
    }

    return mat;
}

void free_matrix(int ***mat, int height)
{
    int i;

    for (i = 0; i < height; i++) {
        free((*mat)[i]);
    }

    free(*mat);
    *mat = NULL;
}

pixel** get_img(signed int width, signed int height)
{
    int i, j;
    pixel **img;

    img = malloc(height * sizeof(pixel *));
    if (img == NULL) {
        return NULL;
    }

    for (i = 0; i < height; i++) {
        img[i] = calloc(width, sizeof(pixel));
        if (img[i] == NULL) {
            for (j = 0; j < i; j++) {
                free(img[j]);
            }

            free(img);
            return NULL;
        }
    }

    return img;
}

void free_img(pixel ***img, int height)
{
    int i;

    for (i = 0; i < height; i++) {
        free((*img)[i]);
    }

    free(*img);
    *img = NULL;
}

void read_img(pixel **img, signed int width, signed int height, FILE *bmp)
{
    int i, j;

    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            fread(&(img[i][j]), sizeof(pixel), 1, bmp);

            //skip padding
            if ((j == width - 1) && (width % 4 != 0)) {
                fseek(bmp, width % 4, SEEK_CUR);
            }
        }
    }
}

void write_img(fileheader bmp_fileheader, infoheader bmp_infoheader, 
				pixel **img, signed int width, signed int height,
				char *new_filename)
{
    int i, j;
    int offset = bmp_fileheader.imageDataOffset;

    FILE *bmp = fopen(new_filename, "wb");

    fwrite(&bmp_fileheader, sizeof(fileheader), 1, bmp);
    fwrite(&bmp_infoheader, sizeof(infoheader), 1, bmp);

    fseek(bmp, offset, SEEK_SET);

    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            fwrite(&(img[i][j]), sizeof(pixel), 1, bmp);

            //write padding
            if ((j == width - 1) && (width % 4 != 0)) {
                fwrite("0", sizeof(unsigned char), width % 4, bmp);
            }
        }
    }

    fclose(bmp);
}

pixel **copy_img(pixel **original, signed int width, signed int height)
{
    int i, j;

    pixel **img = get_img(width, height);

    if (img == NULL) {
        return NULL;
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            img[i][j] = original[i][j];
        }
    }

    return img;
}

char* edit_filename(char* bmp_filename, char* name_addition)
{
    char *new_filename = malloc(50);
    strcpy(new_filename, bmp_filename);
    char *dot = strrchr(new_filename, '.');
    *dot = '\0';
    strcat(new_filename, name_addition);
    return new_filename;
}

pixel **black_white_filter(pixel **img, signed int width, signed int height)
{
    int i, j;
    unsigned char bw;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            bw = (img[i][j].r + img[i][j].g + img[i][j].b)/3;
            img[i][j].r = bw;
            img[i][j].g = bw;
            img[i][j].b = bw;
        }
    }

    return img;
}

pixel **filter(pixel **img, signed int width, signed int height, int mat[3][3])
{
    pixel **bordered_img = get_img(width + 2, height + 2);
    pixel **filtered_img = get_img(width, height);
    int r, g, b;

    int i, j;

    for (i = 1; i <= height; i++) {
        for (j = 1; j <= width; j++) {
            bordered_img[i][j] = img[i - 1][j - 1];
        }
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            r = mat[1][1] * bordered_img[i][j].r +
            	mat[1][2] * bordered_img[i][j + 1].r +
            	mat[1][3] * bordered_img[i][j + 2].r +
                mat[2][1] * bordered_img[i + 1][j].r + 
                mat[2][2] * bordered_img[i + 1][j + 1].r +
                mat[2][3] * bordered_img[i + 1][j + 2].r +
                mat[3][1] * bordered_img[i + 2][j].r +
                mat[3][2] * bordered_img[i + 2][j + 1].r +
                mat[3][3] * bordered_img[i + 2][j + 2].r;
            if (r > 255) {
                filtered_img[i][j].r = 255;
            }
            else {
                if (r < 0) {
                    filtered_img[i][j].r = 0;
                }
                else {
                    filtered_img[i][j].r = r;
                }
            }

            g = mat[1][1] * bordered_img[i][j].g +
            	mat[1][2] * bordered_img[i][j + 1].g +
            	mat[1][3] * bordered_img[i][j + 2].g +
                mat[2][1] * bordered_img[i + 1][j].g +
                mat[2][2] * bordered_img[i + 1][j + 1].g +
                mat[2][3] * bordered_img[i + 1][j + 2].g +
                mat[3][1] * bordered_img[i + 2][j].g +
                mat[3][2] * bordered_img[i + 2][j + 1].g +
                mat[3][3] * bordered_img[i + 2][j + 2].g;
            if (g > 255) {
                filtered_img[i][j].g = 255;
            }
            else {
                if (g < 0) {
                    filtered_img[i][j].g = 0;
                }
                else {
                    filtered_img[i][j].g = g;
                }
            }

            b = mat[1][1] * bordered_img[i][j].b +
            	mat[1][2] * bordered_img[i][j + 1].b +
            	mat[1][3] * bordered_img[i][j + 2].b +
                mat[2][1] * bordered_img[i + 1][j].b +
                mat[2][2] * bordered_img[i + 1][j + 1].b +
                mat[2][3] * bordered_img[i + 1][j + 2].b +
                mat[3][1] * bordered_img[i + 2][j].b +
                mat[3][2] * bordered_img[i + 2][j + 1].b +
                mat[3][3] * bordered_img[i + 2][j + 2].b;
            if (b > 255) {
                filtered_img[i][j].b = 255;
            }
            else {
                if (b < 0) {
                    filtered_img[i][j].b = 0;
                }
                else {
                    filtered_img[i][j].b = b;
                }
            }
        }
    }

    free_img(&bordered_img, height + 2);

    return filtered_img;
}

int in_zone(pixel RGB, pixel R1G1B1, int threshold)
{
    if ((abs(RGB.r - R1G1B1.r) + abs(RGB.g - R1G1B1.g) +
        abs(RGB.b - R1G1B1.b)) <= threshold)
        return 1;
    else
        return 0;
}

void assign_zone(pixel **img, int **zones, int width, int height, coord start, 
					pixel zone_color, int zone, int threshold)
{
    int queue_size = 0, current = 0;
    coord *queue = malloc(width * height * sizeof(coord)), n;

    if (queue == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        return;
    }

    queue[queue_size] = start;
    queue_size++;

    zones[start.x][start.y] = zone;

    while (current < queue_size) {
        n = queue[current];
        current++;

        //South
        if (n.x > 0) {
            if (zones[n.x - 1][n.y] == 0) {
                if (in_zone(zone_color, img[n.x - 1][n.y], threshold)) {
                    queue[queue_size].x = n.x - 1;
                    queue[queue_size].y = n.y;
                    queue_size++;

                    zones[n.x - 1][n.y] = zone;
                }
            }
        }

        //North
        if (n.x < height - 1) {
            if (zones[n.x + 1][n.y] == 0) {
                if (in_zone(zone_color, img[n.x + 1][n.y], threshold)) {
                    queue[queue_size].x = n.x + 1;
                    queue[queue_size].y = n.y;
                    queue_size++;

                    zones[n.x + 1][n.y] = zone;
                }
            }
        }

        //West
        if (n.y > 0) {
            if (zones[n.x][n.y - 1] == 0) {
                if (in_zone(zone_color, img[n.x][n.y - 1], threshold)) {
                    queue[queue_size].x = n.x;
                    queue[queue_size].y = n.y - 1;
                    queue_size++;

                    zones[n.x][n.y - 1] = zone;
                }
            }
        }

        //East
        if (n.y < width - 1) {
            if (zones[n.x][n.y + 1] == 0) {
                if (in_zone(zone_color, img[n.x][n.y + 1], threshold)) {
                    queue[queue_size].x = n.x;
                    queue[queue_size].y = n.y + 1;
                    queue_size++;

                    zones[n.x][n.y + 1] = zone;
                }
            }
        }
    }
    free(queue);
    return;
}

int same_pixel(pixel a, pixel b)
{
    if (a.r != b.r || a.g != b.g || a.b != b.b)
        return 0;
    return 1;
}

void write_bin(fileheader bmp_fileheader, infoheader bmp_infoheader,
				pixel **img,  signed int width, signed int height)
{
    short int i, j;
    char* bin_filename = "compressed.bin";
    unsigned int offset = bmp_fileheader.imageDataOffset;;

    FILE *bin = fopen(bin_filename, "wb");

    fwrite(&bmp_fileheader, sizeof(fileheader), 1, bin);
    fwrite(&bmp_infoheader, sizeof(infoheader), 1, bin);

    fseek(bin, offset, SEEK_SET);

    for (i = 1; i <= height; i++) {
        for (j = 1; j <= width; j++) {
            if (i == 1 || j == 1 || i == height || j == width) {
                fwrite(&i, sizeof(short int), 1, bin);
                fwrite(&j, sizeof(short int), 1, bin);
                fwrite(&(img[i - 1][j - 1].r), sizeof(unsigned char), 1, bin);
                fwrite(&(img[i - 1][j - 1].g), sizeof(unsigned char), 1, bin);
                fwrite(&(img[i - 1][j - 1].b), sizeof(unsigned char), 1, bin);
            }
            else {
                if (!(same_pixel(img[i][j - 1], img[i - 1][j - 1])) ||
                	!(same_pixel(img[i - 1][j - 2], img[i - 1][j - 1])) ||
                    !(same_pixel(img[i - 1][j], img[i - 1][j - 1])) ||
                    !(same_pixel(img[i - 2][j - 1], img[i - 1][j - 1]))) {

                    fwrite(&i, sizeof(short int), 1, bin);
                    fwrite(&j, sizeof(short int), 1, bin);
                    fwrite(&(img[i - 1][j - 1].r), sizeof(unsigned char), 1, bin);
                    fwrite(&(img[i - 1][j - 1].g), sizeof(unsigned char), 1, bin);
                    fwrite(&(img[i - 1][j - 1].b), sizeof(unsigned char), 1, bin);
                }
            }
        }
    }

    fclose(bin);
}

void decompress(char* bin_filename)
{
    fileheader bmp_fileheader;
    infoheader bmp_infoheader;

    pixel** decompressed;
    int **margins; //matrix element is 1 if margin, 0 if not

    FILE *bin = fopen(bin_filename, "rb");

    fread(&bmp_fileheader, sizeof(fileheader), 1, bin);
    fread(&bmp_infoheader, sizeof(infoheader), 1, bin);

    signed int width = bmp_infoheader.width, height = bmp_infoheader.height;
    unsigned int offset = bmp_fileheader.imageDataOffset;

    decompressed = get_img(width, height);
    if (decompressed == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        return;
    }

    margins = get_matrix(width, height);
    if (margins == NULL) {
        fprintf(stderr, "ERROR: Alloc failed\n");
        return;
    }

    short int i = 0, j = 0, m, n;
    unsigned char r, g, b;

    fseek(bin, offset, SEEK_SET);

    do {
        fread(&m, sizeof(short int), 1, bin);
        fread(&n, sizeof(short int), 1, bin);
        fread(&r, sizeof(unsigned char), 1, bin);
        fread(&g, sizeof(unsigned char), 1, bin);
        fread(&b, sizeof(unsigned char), 1, bin);

        decompressed[m - 1][n - 1].r = r;
        decompressed[m - 1][n - 1].g = g;
        decompressed[m - 1][n - 1].b = b;

        margins[m - 1][n - 1] = 1;

    } while(!feof(bin));

    fclose(bin);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (margins[i][j] == 0) {
                decompressed[i][j].r = decompressed[i][j - 1].r;
                decompressed[i][j].g = decompressed[i][j - 1].g;
                decompressed[i][j].b = decompressed[i][j - 1].b;
            }
        }
    }

    write_img(bmp_fileheader, bmp_infoheader, decompressed, width, height, 
    			"decompressed.bmp");

    free_img(&decompressed, height);
    free_matrix(&margins, height);
}
