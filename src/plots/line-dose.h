#pragma once

#ifndef LINE_DOSE_PLOT_H
#define LINE_DOSE_PLOT_H

#include "proton-plot.h"
#include <vector>


class LineDosePlot : public wxWindow, public ProtonPlot {
    struct plot_context {
        wxFont tikfont, axfont;
        wxPoint2DDouble origin, width;
        wxPoint2DDouble bright, tright, tleft;
        double tikwidth, ytikscale;
        double boxwidth;
        double maxxheight, maxywidth, maxpwidth;
        std::vector<std::pair<double, double>> measurements;
    };

    std::vector<std::pair<double, long>> xticks;
    std::vector<double> yticks;
    std::vector<wxString> xticklabels, yticklabels;

    const wxColour dosecolor, meascolor, diffcolor;
    const wxPen dashpen, dosepen, measpen, diffpen, diffpendashed;

    void draw_legend(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_measurements(wxGraphicsContext *gc, struct plot_context *ctx);                       
    void draw_line_dose(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_dashes(wxGraphicsContext *gc, const struct plot_context *ctx);

    void draw_xaxis(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_yaxis(wxGraphicsContext *gc, const struct plot_context *ctx);
    void draw_paxis(wxGraphicsContext *gc, const struct plot_context *ctx);

    void initialize_plot_context(wxGraphicsContext *gc, struct plot_context *ctx);

    void on_evt_paint(wxPaintEvent &e);

    void write_xaxis();
    void write_yaxis();

    inline void redraw() { this->Refresh(); }

public:
    LineDosePlot(wxWindow *parent);
    ~LineDosePlot() = default;

    virtual void draw_plot(wxGraphicsContext *gc) override;

    void write_line_dose();

    void new_dose_loaded();
    void dose_unloaded();
};


#endif /* PLOT_WINDOW_H */
