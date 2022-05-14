#pragma once

#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <wx/wx.h>
#include <vector>


class PlotWindow : public wxFrame {
    wxWindow *canv;

    ProtonLine *line;
    std::vector<std::pair<double, long>> xticks;
    std::vector<double> yticks;

    wxPen dashpen, dosepen;

    void draw_line_dose(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                        const wxPoint2DDouble &pwidth);

    void draw_dashes(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                     const wxPoint2DDouble &pwidth);

    void draw_xaxis(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                    const wxPoint2DDouble &pwidth, double tikwidth);
    void draw_yaxis(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                    const wxPoint2DDouble &pwidth, double tikwidth);

    void draw_plot(wxGraphicsContext *gc);

    void on_evt_paint(wxPaintEvent &e);
    void on_evt_close(wxCloseEvent &e);

    void write_xaxis();
    void write_yaxis();

    void allocate_line_dose();

public:
    PlotWindow(wxWindow *parent);
    ~PlotWindow();

    void write_line_dose();
    void redraw();
    void new_dose_loaded();
};


#endif /* PLOT_WINDOW_H */
