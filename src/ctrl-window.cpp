#include "ctrl-window.h"


CtrlWindow::CtrlWindow(wxWindow *parent):
    wxPanel(parent),
    dcon(new DepthControl(this)),
    vcon(new VisualControl(this)),
    pcon(new PlotControl(this)),
    scon(new ShiftControl(this))
{
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(dcon, 0, wxEXPAND);
    vbox->AddStretchSpacer();
    vbox->Add(vcon, 0, wxEXPAND);
    vbox->AddStretchSpacer();
    vbox->Add(pcon, 0, wxEXPAND);
    vbox->AddStretchSpacer();
    vbox->Add(scon, 0, wxEXPAND);
    this->SetSizer(vbox);
}

void CtrlWindow::set_depth_range(float min, float max)
{
    int l = static_cast<int>(std::ceil(min));
    int h = static_cast<int>(std::floor(max));
    dcon->set_depth_range(l, h);
}
