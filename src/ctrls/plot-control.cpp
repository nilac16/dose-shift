#include "ctrl-symbols.h"
#include "plot-control.h"
#include <wx/valnum.h>

wxDEFINE_EVENT(EVT_PLOT_CONTROL, wxCommandEvent);


void PlotControl::post_change_event()
{
    wxCommandEvent e(EVT_PLOT_CONTROL);
    wxPostEvent(this, e);
}

void PlotControl::on_evt_text(wxCommandEvent &e)
{
    wxString str = e.GetString();
    if (str.IsEmpty()) {
        e.Skip();
    } else {
        xtxt->GetValue().ToDouble(&x);
        ytxt->GetValue().ToDouble(&y);
        post_change_event();
    }
}

PlotControl::PlotControl(wxWindow *parent):
    wxPanel(parent),
    xtxt(new wxTextCtrl(this, wxID_ANY, PLOT_ZERO, wxDefaultPosition, ENTRYSZ)),
    ytxt(new wxTextCtrl(this, wxID_ANY, PLOT_ZERO, wxDefaultPosition, ENTRYSZ)),
    x(0.0), y(0.0)
{
    wxFloatingPointValidator<double> v;
    wxBoxSizer *hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, PLOT_LABEL);
    hbox->Add(new wxStaticText(this, wxID_ANY, PLOT_XLABEL, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(xtxt, 1);
    hbox->Add(new wxStaticText(this, wxID_ANY, PLOT_YLABEL, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(ytxt, 1);
    this->SetSizerAndFit(hbox);

    v.SetPrecision(2);
    xtxt->SetValidator(v);
    ytxt->SetValidator(v);

    xtxt->Bind(wxEVT_TEXT, &PlotControl::on_evt_text, this);
    ytxt->Bind(wxEVT_TEXT, &PlotControl::on_evt_text, this);
}

void PlotControl::get_point(double *x, double *y) const noexcept
{
    *x = this->x;
    *y = this->y;
}

void PlotControl::set_point(double x, double y)
{
    wxString str;
    this->x = x;
    this->y = y;
    str.Printf(wxT("%.2f"), x);
    xtxt->ChangeValue(str);
    str.Printf(wxT("%.2f"), y);
    ytxt->ChangeValue(str);
    post_change_event();
}
