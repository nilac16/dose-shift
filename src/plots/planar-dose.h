#pragma once

#ifndef PLANAR_DOSE_PLOT_H
#define PLANAR_DOSE_PLOT_H

#include "proton-plot.h"


class PlanarDosePlot : public ProtonPlot {

    void draw_measurements(wxGraphicsContext *gc, struct plot_context *ctx);
    void draw_line_dose(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_dashes(wxGraphicsContext *gc, const struct plot_context *ctx);

    void draw_xaxis(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_yaxis(wxGraphicsContext *gc, const struct plot_context *ctx);

    void initialize_plot_context(wxGraphicsContext *gc, struct plot_context *ctx);

    virtual void write_xaxis() override;
    virtual void write_yaxis() override;

public:
    PlanarDosePlot(wxWindow *parent);
    ~PlanarDosePlot() = default;

    virtual void draw_plot(wxGraphicsContext *gc) override;
};


#endif /* PLANAR_DOSE_PLOT_H */
