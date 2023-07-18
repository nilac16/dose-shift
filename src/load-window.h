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

    /** Let's open a dialog
     *  The file dialog
     *  
     *  Wow really no way to do this without refactoring this entire control
     *  I knew I would pay the price later for such a facile implementation
     */
    //void open();
};


#endif /* LOAD_WINDOW_H */
