#pragma once

#ifndef PROTON_PLOT_H
#define PROTON_PLOT_H

#include <wx/wx.h>


/** Literally just a vtable for the drawing function */
class ProtonPlot {

public:
    virtual void draw_plot(wxGraphicsContext *gc) = 0;
};


#endif /* PROTON_PLOT_H */
