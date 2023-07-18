#pragma once

#ifndef CTRL_WINDOW_H
#define CTRL_WINDOW_H

#include <wx/wx.h>
#include "ctrls/depth-control.h"
#include "ctrls/plot-control.h"
#include "ctrls/shift-control.h"
#include "ctrls/visual-control.h"


class CtrlWindow : public wxPanel {
    DepthControl  *dcon;
    VisualControl *vcon;
    PlotControl   *pcon;
    ShiftControl  *scon;

public:
    CtrlWindow(wxWindow *parent);

    inline float get_depth() const { return static_cast<float>(dcon->get_value()); }

    void on_depth_changed(wxCommandEvent &e) { vcon->on_depth_changed(e); }

    /** Sets the valid integer slider depths to [ceil(min), floor(max)] */
    void set_depth_range(float min, float max);
    inline double get_max_slider_depth() const { return static_cast<double>(dcon->get_max()); }

    inline void get_detector_affine(double affine[]) const noexcept { scon->get_affine(affine); }

    inline void set_translation(double x, double y) { scon->set_translation(x, y); }

    inline void get_line_dose(double *x, double *y) const noexcept { pcon->get_point(x, y); }
    inline void set_line_dose(double x, double y) { pcon->set_point(x, y); }

    inline void get_ld_measurements(std::vector<std::tuple<double, double>> &meas) const { pcon->get_ld_measurements(meas); }
    inline void get_pd_measurements(std::vector<std::tuple<double, double>> &meas) const { pcon->get_pd_measurements(meas); }
    inline void get_sp_measurements(std::vector<std::tuple<double, double>> &meas) const { pcon->get_sp_measurements(meas); }

    /** Converts the RS does coordinates to MCC dose coordinates */
    inline void convert_coordinates(double *x, double *y) const noexcept { scon->convert_coordinates(x, y); }

    inline bool detector_enabled() const { return scon->detector_enabled(); }

    const ProtonPlaneParams &visuals() const noexcept { return vcon->visuals(); }
};


#endif /* CTRL_WINDOW_H */
