#pragma once

#ifndef PROTON_PLOT_H
#define PROTON_PLOT_H

#include <wx/wx.h>
#include <vector>

/** Controls the vertical axis limit of the line dose plot. Determines what 
 *  proportion of the global maximum dose density will be used as the limit */
#define DOSEMAX_MULT    1.0

/** Controls the vertical axis limit of the planar dose plot, using the same 
 *  logic as above, but with planar integrated dose */
#define PLANEMAX_MULT   1.1

#define GRID_OPACITY        0.2
#define LINEDOSE_OPACITY    0.8

#define MIN_XTICKS      9
#define MIN_YTICKS      9

#define IMAGE_AXFONT_SIZE  20   /* Axis titles */
#define IMAGE_TIKFONT_SIZE 14   /* Axis tick labels */

#define DEPTH_AXLABEL   wxT("Depth (mm)")
#define DOSE_AXLABEL    wxT("Dose (Gy)")
#define PLANE_AXLABEL   wxT("Average planar dose (Gy)")
#define PDIFF_AXLABEL   wxT("% Dose difference")

#ifndef M_PI_2
constexpr double M_PI_2 = 1.57079632679489661923;
#endif


class ProtonPlot : public wxWindow {
    void on_evt_paint(wxPaintEvent &e);

    virtual void write_xaxis() = 0;
    virtual void write_yaxis() = 0;

protected:
    struct plot_context {
        wxFont tikfont, axfont;
        wxPoint2DDouble origin, width;
        wxPoint2DDouble bright, tright, tleft;
        double tikwidth, ytikscale;
        double boxwidth;
        double maxxheight, maxywidth, maxpwidth;
    };

    const wxColour dosecolor, meascolor, diffcolor;
    const wxPen dashpen, dosepen, measpen, diffpen, diffpendashed;

    std::vector<std::pair<double, long>> xticks;
    std::vector<double> yticks;
    std::vector<wxString> xticklabels, yticklabels;

    const wxString xlabel, ylabel, plabel;

    std::vector<std::tuple<double, double>> measurements;

    void write_depth_axis();
    void write_dose_axis(const double limit, const wxString &fmt);

public:
    ProtonPlot(wxWindow *parent, const wxString &xlabel, 
               const wxString &ylabel, const wxString &plabel);

    virtual void draw_plot(wxGraphicsContext *gc) = 0;
    void write_axes();
};


#endif /* PROTON_PLOT_H */
