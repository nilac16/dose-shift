#pragma once

#ifndef PLOT_CONTROL_H
#define PLOT_CONTROL_H

#include <array>
#include <vector>
#include <wx/wx.h>
#include "../proton/mcc-data.h"

wxDECLARE_EVENT(EVT_PLOT_CONTROL, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLOT_OPEN, wxCommandEvent);


class PlotMeasurement : public wxPanel {
    wxButton *btn;
    wxTextCtrl *dctrl;
    wxStaticText *flbl;
    
    double depth;
    MCCData *data;

    void post_change_event();

    void on_evt_button(wxCommandEvent &e);
    void on_evt_text(wxCommandEvent &e);

public:
    PlotMeasurement(wxWindow *parent);
    ~PlotMeasurement();

    constexpr bool is_loaded() const noexcept { return data != nullptr; }
    double get_dose(double x, double y) const noexcept;
    constexpr double get_depth() const noexcept { return depth; }
};


class PlotControl : public wxPanel {
    wxTextCtrl *xtxt, *ytxt;
    wxButton *obtn;
    double x, y;

    std::array<PlotMeasurement *, 3> measurements;

    void post_change_event();

    void on_evt_text(wxCommandEvent &e);

public:
    PlotControl(wxWindow *parent);

    void get_point(double *x, double *y) const noexcept;
    void set_point(double x, double y);

    void get_measurements(std::vector<std::pair<double, double>> &meas) const;
};


#endif /* PLOT_CONTROL_H */
