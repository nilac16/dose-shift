#pragma once

#ifndef CTRL_WINDOW_H
#define CTRL_WINDOW_H

#include <wx/wx.h>
#include "ctrls/depth-control.h"
#include "ctrls/plot-control.h"
#include "ctrls/shift-control.h"


class CtrlWindow : public wxPanel {
    DepthControl *dcon;
    PlotControl  *pcon;
    ShiftControl *scon;

public:
    CtrlWindow(wxWindow *parent);

    float get_depth() const;

    /** Sets the valid integer slider depths to [ceil(min), floor(max)] */
    void set_depth_range(float min, float max);
    double get_max_slider_depth() const;

    void get_detector_affine(double affine[]) const noexcept;

    void set_translation(double x, double y);

    void get_line_dose(double *x, double *y) const noexcept;
    void set_line_dose(double x, double y);

    void get_measurements(std::vector<std::tuple<double, double, double>> &meas) const;

    /** Converts the RS does coordinates to MCC dose coordinates */
    void convert_coordinates(double *x, double *y) const noexcept;

    bool detector_enabled() const;
};


#endif /* CTRL_WINDOW_H */
