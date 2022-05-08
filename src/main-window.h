#pragma once

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <wx/wx.h>
#include "dose-window.h"
#include "ctrl-window.h"
#include "load-window.h"
#include "plot-window.h"


class MainApplication : public wxApp {
    wxFrame *frame;
    DoseWindow *canv;
    CtrlWindow *cwnd;
    LoadWindow *lwnd;
    PlotWindow *pwnd;

    void initialize_main_window();

    void on_dicom_load(wxFileDirPickerEvent &e);
    void on_plot_change(wxCommandEvent &e);
    void on_plot_open(wxCommandEvent &e);

public:
    float get_depth() const;

    bool detector_enabled() const;
    void get_detector_affine(double affine[]) const noexcept;

    void get_line_dose(double *x, double *y) const noexcept;
    void set_line_dose(double x, double y);

    void set_translation(double x, double y);

    bool dose_loaded() const noexcept;
    const ProtonDose *get_dose() const noexcept;

    double get_max_slider_depth() const;
    double get_max_dose() const noexcept;

    virtual bool OnInit() override;
};

wxDECLARE_APP(MainApplication);


#endif /* MAIN_WINDOW_H */
