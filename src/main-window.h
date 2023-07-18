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

    void load_file(const wxString &path);

    void on_dicom_load(wxFileDirPickerEvent &e);
    void on_depth_change(wxCommandEvent &e);
    void on_plot_change(wxCommandEvent &e);
    void on_shift_change(wxCommandEvent &e);
    void on_visual_change(wxCommandEvent &e);
    void on_plot_open(wxCommandEvent &e);

public:
    inline float get_depth() const { return cwnd->get_depth(); }
    void set_depth_range();

    inline bool detector_enabled() const { return cwnd->detector_enabled(); }
    inline void get_detector_affine(double affine[]) const noexcept { cwnd->get_detector_affine(affine); }

    inline void get_line_dose(double *x, double *y) const noexcept { cwnd->get_line_dose(x, y); }
    inline void set_line_dose(double x, double y) { cwnd->set_line_dose(x, y); }

    inline void set_translation(double x, double y) { cwnd->set_translation(x, y); }

    constexpr bool dose_loaded() const noexcept { return canv->dose_loaded(); }
    constexpr const ProtonDose *get_dose() const noexcept { return canv->get_dose(); }

    inline double get_max_slider_depth() const { return cwnd->get_max_slider_depth(); }
    inline double get_max_dose() const noexcept { return static_cast<double>(proton_dose_max(get_dose())); }

    inline void unload_dose() noexcept { canv->unload_dose(); }

    inline void get_ld_measurements(std::vector<std::tuple<double, double>> &meas) const { cwnd->get_ld_measurements(meas); }
    inline void get_pd_measurements(std::vector<std::tuple<double, double>> &meas) const { cwnd->get_pd_measurements(meas); }
    inline void get_sp_measurements(std::vector<std::tuple<double, double>> &meas) const { cwnd->get_sp_measurements(meas); }

    inline wxString get_RS_directory() const { return lwnd->get_directory(); }

    inline void convert_coordinates(double *x, double *y) const noexcept { cwnd->convert_coordinates(x, y); }

    const ProtonPlaneParams &visuals() const noexcept { return cwnd->visuals(); }

    void dropped_file(const wxString &path);

    virtual bool OnInit() override;
};

wxDECLARE_APP(MainApplication);


#endif /* MAIN_WINDOW_H */
