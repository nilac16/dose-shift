#pragma once

#ifndef DEPTH_CONTROL_H
#define DEPTH_CONTROL_H

#include <wx/wx.h>

wxDECLARE_EVENT(EVT_DEPTH_CONTROL, wxCommandEvent);


class DepthControl : public wxPanel {
    wxSlider *slider;
    wxTextCtrl *entry;

    void post_change_event();

    void on_evt_slider(wxScrollEvent &e);
    void on_evt_text(wxCommandEvent &e);

public:
    DepthControl(wxWindow *parent);

    inline int get_value() const { return slider->GetValue(); }
    void set_value(int x);

    void set_depth_range(int low, int high);
    inline int get_max() const { return slider->GetMax(); }
};


#endif /* DEPTH_CONTROL_H */
