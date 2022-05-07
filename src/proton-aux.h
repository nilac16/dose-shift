#pragma once

#ifndef PROTON_AUXILIARY_H
#define PROTON_AUXILIARY_H

#if __cplusplus
extern "C" {
#endif


void proton_colormap(float x, unsigned char px[]);

void proton_affine_matmul(const double mat1[], double mat2[]);


#if __cplusplus
}
#endif

#endif /* PROTON_COLORMAP_H */
