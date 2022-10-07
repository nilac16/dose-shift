#pragma once

#ifndef PROTON_PLOT_H
#define PROTON_PLOT_H

#include <wx/wx.h>
#include <vector>


class ProtonPlot : public wxWindow {
    /** Pure virtual function call danger */
    void on_evt_paint(wxPaintEvent &e);

protected:
    std::vector<std::pair<double, long>> xticks;
    std::vector<double> yticks;
    std::vector<wxString> xticklabels, yticklabels;

    inline void redraw() { this->Refresh(); }

public:
    ProtonPlot(wxWindow *parent);

    virtual void draw_plot(wxGraphicsContext *gc) = 0;
};


#endif /* PROTON_PLOT_H */
