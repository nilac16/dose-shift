#include "ctrl-window.h"


CtrlWindow::CtrlWindow(wxWindow *parent):
    wxPanel(parent),
    dcon(new DepthControl(this)),
    pcon(new PlotControl(this)),
    scon(new ShiftControl(this))
{
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(dcon, 0, wxEXPAND);
    vbox->AddStretchSpacer();
    vbox->Add(pcon, 0, wxEXPAND);
    vbox->AddStretchSpacer();
    vbox->Add(scon, 0, wxEXPAND);
    this->SetSizer(vbox);
}

float CtrlWindow::get_depth() const
{
    return static_cast<float>(dcon->get_value());
}

void CtrlWindow::set_max_depth(int depth)
{
    int cur = dcon->get_value();
    if (cur > depth) {
        dcon->set_value(0);
    }
    dcon->set_max_depth(depth);
}

void CtrlWindow::get_detector_affine(double affine[]) const noexcept
{
    scon->get_affine(affine);
}

void CtrlWindow::set_translation(double x, double y)
{
    scon->set_translation(x, y);
}

void CtrlWindow::get_line_dose(double *x, double *y) const noexcept
{
    return pcon->get_point(x, y);
}

void CtrlWindow::set_line_dose(double x, double y)
{
    return pcon->set_point(x, y);
}

bool CtrlWindow::detector_enabled() const
{
    return scon->detector_enabled();
}
