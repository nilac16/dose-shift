#pragma once

#ifndef LOAD_WINDOW_H
#define LOAD_WINDOW_H

#include <wx/wx.h>
#include <wx/filepicker.h>


class LoadWindow : public wxPanel {
    wxFilePickerCtrl *fctrl;

public:
    LoadWindow(wxWindow *parent);
};


#endif /* LOAD_WINDOW_H */
