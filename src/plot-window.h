#pragma once

#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <wx/wx.h>


class PlotWindow : public wxFrame {
    long ndose;
    float *line_dose;
    float maxdepth;

    double pmargins[4];

    void on_evt_paint(wxPaintEvent &e);
    void on_evt_close(wxCloseEvent &e);

public:
    PlotWindow(wxWindow *parent);


};


#endif /* PLOT_WINDOW_H */
