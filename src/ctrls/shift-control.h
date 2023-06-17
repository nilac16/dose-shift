#pragma once

#ifndef SHIFT_CONTROL_H
#define SHIFT_CONTROL_H

#include <wx/wx.h>

wxDECLARE_EVENT(EVT_SHIFT_CONTROL, wxCommandEvent);


class ShiftControl : public wxPanel {
    wxCheckBox* enabld;
    wxButton *reset;
    wxTextCtrl *shfttext1, *shfttext2;
    wxSlider *anglsldr;
    wxTextCtrl *angltext;
    double x, y;
    double cos, sin;

    void post_change_event();

    void write_trig_functions(double degrees);

    void on_evt_checkbox(wxCommandEvent &e);
    void on_evt_button(wxCommandEvent &e);
    void on_evt_translation(wxCommandEvent &e);
    void on_evt_rot_slide(wxScrollEvent &e);
    void on_evt_rot_text(wxCommandEvent &e);

public:
    ShiftControl(wxWindow *parent);

    void get_affine(double affine[]) const noexcept;
    void set_translation(double x, double y);

    void convert_coordinates(double *x, double *y) const noexcept;

    inline bool detector_enabled() const { return enabld->GetValue(); }
};


#endif /* SHIFT_CONTROL_H */
