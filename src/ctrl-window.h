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

    /** Also changes the current depth to zero if it is greater than the 
     *  passed value
     */
    void set_max_depth(int depth);

    void get_detector_affine(double affine[]) const noexcept;

    void set_translation(double x, double y);

    void get_line_dose(double *x, double *y) const noexcept;
    void set_line_dose(double x, double y);

    bool detector_enabled() const;
};


#endif /* CTRL_WINDOW_H */
