#pragma once

#ifndef PLANAR_DOSE_PLOT_H
#define PLANAR_DOSE_PLOT_H

#include "proton-plot.h"


class PlanarDosePlot : public ProtonPlot {

public:
    PlanarDosePlot(wxWindow *parent);
    ~PlanarDosePlot() = default;

    virtual void draw_plot(wxGraphicsContext *gc) override;
};


#endif /* PLANAR_DOSE_PLOT_H */
