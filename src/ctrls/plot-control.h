#pragma once

#ifndef PLOT_CONTROL_H
#define PLOT_CONTROL_H

#include <wx/wx.h>

wxDECLARE_EVENT(EVT_PLOT_CONTROL, wxCommandEvent);


class PlotControl : public wxPanel {
    wxTextCtrl *xtxt, *ytxt;
    double x, y;

    void post_change_event();

    void on_evt_text(wxCommandEvent &e);

public:
    PlotControl(wxWindow *parent);

    void get_point(double *x, double *y) const noexcept;
    void set_point(double x, double y);
};


#endif /* PLOT_CONTROL_H */
