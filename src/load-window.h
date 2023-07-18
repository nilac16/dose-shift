#pragma once

#ifndef LOAD_WINDOW_H
#define LOAD_WINDOW_H

#include <wx/wx.h>
#include <wx/filepicker.h>


class LoadWindow : public wxPanel {
    wxFilePickerCtrl *fctrl;

public:
    LoadWindow(wxWindow *parent);

    wxString get_directory() const;

    void set_file(const wxString &path);
};


#endif /* LOAD_WINDOW_H */
