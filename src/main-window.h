#pragma once

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <wx/wx.h>
#include "dose-window.h"
#include "ctrl-window.h"
#include "load-window.h"


class MainApplication : public wxApp {
    wxFrame *frame;
    DoseWindow *canv;
    CtrlWindow *cwnd;
    LoadWindow *lwnd;

    void initialize_main_window();

    void on_dicom_load(wxFileDirPickerEvent &e);
    void on_plot_change(wxCommandEvent &e);

public:
    float get_depth() const;

    bool detector_enabled() const;
    void get_detector_affine(double affine[]) const noexcept;

    void get_line_dose(double *x, double *y) const noexcept;
    void set_line_dose(double x, double y);

    void set_translation(double x, double y);

    virtual bool OnInit() override;
};

wxDECLARE_APP(MainApplication);


#endif /* MAIN_WINDOW_H */
