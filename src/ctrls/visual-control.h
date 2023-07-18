#pragma once
/** This control is utterly useless
 * 
 *  Maybe it's interpolating the wrong way? So far, it has *not once* predicted
 *  a low pass rate
 */
#ifndef VISUALISER_CONTROL_H
#define VISUALISER_CONTROL_H

#include <wx/wx.h>
#include "../proton/proton-dose.h"

wxDECLARE_EVENT(EVT_VISUAL_CONTROL, wxCommandEvent);


class VisualControl: public wxPanel {
    wxCheckBox *cbox;
    wxButton *reset;
    wxTextCtrl *diff, *derr;
    wxCheckBox *autocalc;

    ProtonPlaneParams params;

    static const wxArrayString &choices() noexcept;

    bool automatic() noexcept { return autocalc->GetValue(); }

    /** Fetches the current depth, computes the error, and writes it out to the
     *  text box */
    void set_auto_error();

    void post_changed_event();

    /** Writes the % dose difference value directly from the parameters struct.
     *  Use when the value needs to be reset */
    void write_diff();

    /** Writes the depth error directly from the parameters struct. Use this
     *  it automatically updates or needs to be reset */
    void write_err();

    void on_evt_checkbox(wxCommandEvent &e);
    void on_evt_reset(wxCommandEvent &e);
    void on_evt_automatic(wxCommandEvent &e);
    void on_evt_difftext(wxCommandEvent &e);
    void on_evt_errtext(wxCommandEvent &e);

public:
    VisualControl(wxWindow *parent);

    void on_depth_changed(wxCommandEvent &e);

    const ProtonPlaneParams &visuals() const noexcept { return params; }
};


#endif /* VISUALISER_CONTROL_H */
