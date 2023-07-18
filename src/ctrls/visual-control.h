#pragma once

#ifndef VISUALISER_CONTROL_H
#define VISUALISER_CONTROL_H

#include <wx/wx.h>
#include "../proton/proton-dose.h"

wxDECLARE_EVENT(EVT_VISUAL_CONTROL, wxCommandEvent);


class VisualControl: public wxPanel {
    wxCheckBox *cbox;
    wxButton *reset;
    wxTextCtrl *diff, *derr;

    ProtonPlaneParams params;

    static const wxArrayString &choices() noexcept;

    void post_changed_event();

    void on_evt_checkbox(wxCommandEvent &e);
    void on_evt_difftext(wxCommandEvent &e);
    void on_evt_errtext(wxCommandEvent &e);

public:
    VisualControl(wxWindow *parent);

    const ProtonPlaneParams &visuals() const noexcept { return params; }
};


#endif /* VISUALISER_CONTROL_H */
