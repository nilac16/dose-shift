#pragma once

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <wx/wx.h>
#include "dose-window.h"
#include "ctrl-window.h"
#include "load-window.h"
#include "plot-window.h"


class MainApplication : public wxApp {
    wxFrame *m_frame;
    DoseWindow *m_canv;
    CtrlWindow *m_cwnd;
    LoadWindow *m_lwnd;
    PlotWindow *m_pwnd;

    void initialize_main_window();

    void load_file(const wxString &path);

    void on_dicom_load(wxFileDirPickerEvent &e);
    void on_depth_change(wxCommandEvent &e);
    void on_plot_change(wxCommandEvent &e);
    void on_shift_change(wxCommandEvent &e);
    void on_visual_change(wxCommandEvent &e);
    void on_plot_open(wxCommandEvent &e);


    wxFrame *&main_frame() noexcept { return m_frame; }
    DoseWindow *&canvas() noexcept { return m_canv; }
    CtrlWindow *&ctrl_wnd() noexcept { return m_cwnd; }
    LoadWindow *&load_wnd() noexcept { return m_lwnd; }
    PlotWindow *&plot_wnd() noexcept { return m_pwnd; }

    const wxFrame *main_frame() const noexcept { return m_frame; }
    const DoseWindow *canvas() const noexcept { return m_canv; }
    const CtrlWindow *ctrl_wnd() const noexcept { return m_cwnd; }
    const LoadWindow *load_wnd() const noexcept { return m_lwnd; }
    const PlotWindow *plot_wnd() const noexcept { return m_pwnd; }


public:
    float get_depth() const { return ctrl_wnd()->get_depth(); }
    void set_depth_range();

    bool detector_enabled() const { return ctrl_wnd()->detector_enabled(); }

    void get_detector_affine(double affine[])
        const noexcept { ctrl_wnd()->get_detector_affine(affine); }

    void get_line_dose(double *x, double *y)
        const noexcept { ctrl_wnd()->get_line_dose(x, y); }

    void set_line_dose(double x, double y) { ctrl_wnd()->set_line_dose(x, y); }

    void set_translation(double x, double y)
        { ctrl_wnd()->set_translation(x, y); }

    bool dose_loaded() const noexcept { return canvas()->dose_loaded(); }

    const ProtonDose *get_dose() const noexcept { return canvas()->get_dose(); }

    double get_max_slider_depth()
        const { return ctrl_wnd()->get_max_slider_depth(); }

    double get_max_dose()
        const noexcept { return (double)proton_dose_max(get_dose()); }

    void unload_dose() noexcept { canvas()->unload_dose(); }

    void get_ld_measurements(std::vector<std::tuple<double, double>> &meas)
        const { ctrl_wnd()->get_ld_measurements(meas); }

    void get_pd_measurements(std::vector<std::tuple<double, double>> &meas)
        const { ctrl_wnd()->get_pd_measurements(meas); }

    void get_sp_measurements(std::vector<std::tuple<double, double>> &meas)
        const { ctrl_wnd()->get_sp_measurements(meas); }

    wxString get_RS_directory() const { return load_wnd()->get_directory(); }

    void convert_coordinates(double *x, double *y)
        const noexcept { ctrl_wnd()->convert_coordinates(x, y); }

    const ProtonPlaneParams &visuals()
        const noexcept { return ctrl_wnd()->visuals(); }

    void dropped_file(const wxString &path);

    virtual bool OnInit() override;
};

wxDECLARE_APP(MainApplication);


#endif /* MAIN_WINDOW_H */
