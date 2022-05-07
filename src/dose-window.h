#pragma once

#ifndef DOSE_WINDOW_H
#define DOSE_WINDOW_H

#include <wx/wx.h>
#include "proton/proton-dose.h"


class DoseWindow : public wxWindow {
    ProtonDose *dose;
    ProtonImage *img;

    wxPoint origin;

    double affine[6];

    void paint_detector(wxPaintDC &dc);
    void paint_bitmap(wxPaintDC &dc);

    void on_paint(wxPaintEvent &e);
    void on_size(wxSizeEvent &e);
    void on_lmb(wxMouseEvent &e);
    void on_motion(wxMouseEvent &e);

    void affine_write();

    void image_write();
    void image_realloc_and_write(const wxSize &csz);

    bool point_in_dose(const wxPoint &p);

public:
    DoseWindow(wxWindow *parent);
    ~DoseWindow();

    void load_file(const char *filename);
    constexpr bool dose_loaded() const noexcept { return dose != nullptr; }

    void on_depth_changed(wxCommandEvent &e);
    void on_shift_changed(wxCommandEvent &e);

    float get_max_depth() const noexcept;
};


#endif /* DOSE_WINDOW_H */
