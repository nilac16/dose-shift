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
