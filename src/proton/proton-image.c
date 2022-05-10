#include <stdlib.h>
#include "proton-image.h"


struct _proton_image {
    long dim[2];
    long bufwidth;
    unsigned char buf[];
};

static ProtonImage *proton_image_flexible_alloc(long N)
{
    struct _proton_image *img;
    img = malloc(sizeof *img + sizeof *img->buf * N);
    if (img) {
        img->bufwidth = N;
    }
    return img;
}

static void proton_image_dim_set(ProtonImage *img, long width, long height)
{
    img->dim[0] = width;
    img->dim[1] = height;
}

int proton_image_realloc(ProtonImage **img, long width, long height)
{
    const long N = 3UL * width * height;
    const long N_old = (*img) ? (*img)->bufwidth : 0;
    if (N > N_old) {
        free(*img);
        *img = proton_image_flexible_alloc((3 * N) / 2);
        if (!*img) {
            return 1;
        }
    }
    proton_image_dim_set(*img, width, height);
    return 0;
}

void proton_image_destroy(ProtonImage *img)
{
    free(img);
}

long proton_image_dimension(const ProtonImage *img, int dim)
{
    return img->dim[dim];
}

unsigned char *proton_image_raw(ProtonImage *img)
{
    return img->buf;
}

int proton_image_empty(const ProtonImage *img)
{
    return (img->dim[0] == 0) || (img->dim[1] == 0);
}
