#pragma once

#ifndef CTRL_SYMBOLS_H
#define CTRL_SYMBOLS_H

#define ENTRYSZ wxSize(50, -1)

#define SLIDERSCALE 4
#define ENTRYSCALE  1

#define DEPTH_LABEL     wxT("Slice depth (mm)")
#define DEPTH_INIT_MAX  300

#define VISUAL_LABEL    wxT("Parameters")

#define GRADIENT_LABEL  wxT("Gradient")
#define GRADIENT_SHOW   wxT("Show gradient")
#define GRADIENT_AUTO   wxT("Automatic depth error")
#define GRAD_RESET      wxT("Reset")
#define GRAD_DIFFLBL    wxT("% dose difference")
#define GRAD_DIFFINIT   wxT("3.0")
#define GRAD_ERRLBL     wxT("Depth error (mm)")
#define GRAD_ERRINIT    wxT("0.50")

#define PLOT_LABEL      wxT("Line dose")
#define PLOT_XLABEL     wxT("x (mm)")
#define PLOT_YLABEL     wxT("y (mm)")
#define PLOT_ZERO       wxT("0.00")
#define PLOT_OPENLBL    wxT("Open plot window")

#define DETECTOR_SHOW   wxT("Show detector window")
#define DETECTOR_RESET  wxT("Reset")

#define SHIFT_LABEL     wxT("Window translation")
#define SHIFT_XLABEL    wxT("x (cm)")
#define SHIFT_YLABEL    wxT("y (cm)")
#define SHIFT_ZERO      wxT("0.00")

#define ANGLE_LABEL wxT("Window angle (degrees)")
#define ANGLE_MIN  -450
#define ANGLE_MAX   450
#define ANGLE_ZERO  wxT("0.0")


#endif /* CTRL_SYMBOLS_H */
