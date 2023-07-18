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


/** @brief Compute the physical solid water buildup required to create an
 *      apparent depth of @p theor to the proton beam
 *  @param theor
 *      The theoretical depth of water to be measured, in CENTIMETERS
 *  @returns The required physical buildup of solid water, in centimeters
 *  @todo Maybe add a parameters struct to make this more generic
 */
double proton_buildup(double theor);


#if __cplusplus
}
#endif

#endif /* PROTON_COLORMAP_H */
