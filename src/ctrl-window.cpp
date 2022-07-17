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

void CtrlWindow::set_depth_range(float min, float max)
{
    int l = static_cast<int>(std::ceil(min));
    int h = static_cast<int>(std::floor(max));
    dcon->set_depth_range(l, h);
}

double CtrlWindow::get_max_slider_depth() const
{
    return static_cast<double>(dcon->get_max());
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

void CtrlWindow::get_measurements(std::vector<std::pair<double, double>> &meas) const
{
    pcon->get_measurements(meas);
}

void CtrlWindow::convert_coordinates(double *x, double *y) const noexcept
{
    scon->convert_coordinates(x, y);
}

bool CtrlWindow::detector_enabled() const
{
    return scon->detector_enabled();
}
