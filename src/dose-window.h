#pragma once

#ifndef DOSE_WINDOW_H
#define DOSE_WINDOW_H

#include <wx/wx.h>
#include <wx/dnd.h>
#include "proton/proton-dose.h"


class DoseWindow : public wxWindow {
    ProtonDose *dose;
    ProtonImage *img;

    wxPoint origin;

    /* class DoseDragNDrop : public wxFileDropTarget {

    public:
        DoseDragNDrop() = default;

        virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString &filenames) override;
    } droptarget; */

    double affine[6];
    double conv[2];

    void paint_detector(wxPaintDC &dc);
    void paint_bitmap(wxPaintDC &dc);

    void on_paint(wxPaintEvent &e);
    void on_size(wxSizeEvent &e);
    void on_lmb(wxMouseEvent &e);
    void on_rmb(wxMouseEvent &e);
    void on_motion(wxMouseEvent &e);

    void conv_write();
    void affine_write();

    void image_write();
    void image_realloc_and_write(const wxSize &csz);

    bool point_in_dose(const wxPoint &p);
    void write_line_dose() noexcept;

public:
    DoseWindow(wxWindow *parent);
    ~DoseWindow();

    void load_file(const char *filename);
    constexpr bool dose_loaded() const noexcept { return dose != nullptr; }

    void on_depth_changed(wxCommandEvent &e);
    void on_plot_changed(wxCommandEvent &e);
    void on_shift_changed(wxCommandEvent &e);

    inline void get_depth_range(float range[]) const noexcept { proton_dose_depth_range(dose, range); }

    constexpr const ProtonDose *get_dose() const noexcept { return dose; }
    void unload_dose() noexcept;
};


#endif /* DOSE_WINDOW_H */
