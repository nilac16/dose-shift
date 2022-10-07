#pragma once

#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/graphics.h>
#include "plots/line-dose.h"
#include "plots/planar-dose.h"

/** TODO: Computations with LibreOffice yielded an aspect ratio of 1.45,
 *        test this with MS Excel to be certain 
 *                      ** originally 1044x720 **
 * 
 *  RESULT: In Excel, the plot boxes appear to have an aspect ratio of 1.13
 *  ACTUALLY: On the PDF, they have an aspect of ~1.3
 */
#define IMAGE_RES wxSize(1080, 800)

/** PLEASE ALWAYS MAKE THIS THE SQUARE ROOT OF THE OUTPUT IMAGE AREA **/
constexpr double image_gmean = 929.51600308978;


class PlotWindow : public wxFrame {
    wxNotebook *nb;
    LineDosePlot *ldplot;
    PlanarDosePlot *pdplot;

    void on_evt_close(wxCloseEvent &e);

    void on_context_menu_selection(wxCommandEvent &e);
    void on_context_menu(wxContextMenuEvent &e);

    inline ProtonPlot *get_current_plot()
        { return dynamic_cast<ProtonPlot *>(nb->GetCurrentPage()); }

public:
    PlotWindow(wxWindow *parent);
    ~PlotWindow();

/** The only event that requires more than a redraw (currently) is the plot 
 *  changing, since the line dose will need to be reinterpolated. Depth/
 *  measurement markers are computed in the paint handler
 */

    void on_dicom_changed(wxCommandEvent &e);
    void on_depth_changed(wxCommandEvent &e);
    void on_plot_changed(wxCommandEvent &e);
    void on_shift_changed(wxCommandEvent &e);
};


#endif /* PLOT_WINDOW_H */
