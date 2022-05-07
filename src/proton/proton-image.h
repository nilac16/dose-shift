#pragma once

#ifndef PROTON_IMAGE_H
#define PROTON_IMAGE_H

#if __cplusplus
extern "C" {
#endif


typedef struct _proton_image/* {
    long dim[2];
    long bufwidth;
    unsigned char buf[];
} */ProtonImage;


int proton_image_realloc(ProtonImage **img, long width, long height);
void proton_image_destroy(ProtonImage *img);

long proton_image_dimension(const ProtonImage *img, int dim);
unsigned char *proton_image_raw(ProtonImage *img);

int proton_image_empty(const ProtonImage *img);


#if __cplusplus
}
#endif

#endif /* PROTON_IMAGE_H */
