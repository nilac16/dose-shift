#pragma once

#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <wx/wx.h>
#include <vector>


class PlotWindow : public wxFrame {
    wxWindow *canv;

    struct plot_context {
        wxFont tikfont, axfont;
        wxPoint2DDouble origin, width;
        wxPoint2DDouble bright, tright, tleft;
        double tikwidth, ytikscale;
        double boxwidth;
        double maxxheight, maxywidth, maxpwidth;
        std::vector<std::pair<double, double>> measurements;
    };

    ProtonLine *line;
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
    void draw_plot(wxGraphicsContext *gc);

    void on_evt_paint(wxPaintEvent &e);
    void on_evt_close(wxCloseEvent &e);
    void on_context_menu(wxContextMenuEvent &e);

    void write_xaxis();
    void write_yaxis();

    void allocate_line_dose();

public:
    PlotWindow(wxWindow *parent);
    ~PlotWindow();

    void write_line_dose();
    void redraw();
    void new_dose_loaded();
    void dose_unloaded();
};


#endif /* PLOT_WINDOW_H */
