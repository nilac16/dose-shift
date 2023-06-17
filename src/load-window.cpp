#include "load-window.h"

#define LOAD_FRAME  wxT("Load a DICOM")
#define LOAD_TITLE  wxT("Load a Raystation DICOM")
#define LOAD_FILTER wxT("DICOM files (*.dcm)|*.dcm")


LoadWindow::LoadWindow(wxWindow *parent):
    wxPanel(parent),
    fctrl(new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, LOAD_TITLE, LOAD_FILTER))
{
    wxBoxSizer *hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, LOAD_FRAME);
    hbox->AddStretchSpacer();
    hbox->Add(fctrl, 3, wxEXPAND | wxHORIZONTAL);
    hbox->AddStretchSpacer();
    this->SetSizer(hbox);
}

wxString LoadWindow::get_directory() const
{
    wxString str = fctrl->GetPath();
    if (!str.empty()) {
        wxFileName path(str);
        str = path.GetPath();
    }
    return str;
}

void LoadWindow::set_directory(const wxString &dir)
{
    fctrl->SetPath(dir);
}
