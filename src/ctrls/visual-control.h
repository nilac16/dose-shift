#pragma once

#ifndef VISUALISER_CONTROL_H
#define VISUALISER_CONTROL_H

#include <wx/wx.h>

wxDECLARE_EVENT(EVT_VISUAL_CONTROL, wxCommandEvent);


class VisualControl: public wxPanel {
    wxCheckBox *cbox;
    wxChoice *lbox;

    /** I *could* have typedef'd this, but I'm feeling really abstruse right now */
    void (*cmap_func)(float, unsigned char *);
    int mode;

    static const wxArrayString &choices() noexcept;

    void post_changed_event();

    void write_cmap();

    void on_evt_checkbox(wxCommandEvent &e);
    void on_evt_choice(wxCommandEvent &e);

public:
    VisualControl(wxWindow *parent);

    void (*colormap() const noexcept)(float, unsigned char *){ return cmap_func; }
    int visuals() const noexcept { return mode; }
};


#endif /* VISUALISER_CONTROL_H */
