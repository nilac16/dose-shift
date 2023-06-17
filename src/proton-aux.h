#pragma once

#ifndef PROTON_AUXILIARY_H
#define PROTON_AUXILIARY_H

#if __cplusplus
extern "C" {
#endif


void proton_colormap(float x, unsigned char px[]);


/** Perceptually uniform colormaps, from Octave */
void proton_cmap_viridis(float x, unsigned char px[]);
void proton_cmap_turbo(float x, unsigned char px[]);


/** For the gradient visualizer */
void proton_cmap_gradient(float x, unsigned char px[]);


#if __cplusplus
}
#endif

#endif /* PROTON_COLORMAP_H */
