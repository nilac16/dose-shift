#include <array>
#include "visual-control.h"
#include "ctrl-symbols.h"
#include <wx/valnum.h>
#include "../proton-aux.h"
#include "../proton/proton-dose.h"

wxDEFINE_EVENT(EVT_VISUAL_CONTROL, wxCommandEvent);


const wxArrayString &VisualControl::choices() noexcept
{
    static const std::array<wxString, 3> choices = {
        wxString(wxT("Jet")),
        wxString(wxT("Turbo")),
        wxString(wxT("Viridis"))
    };
    static const wxArrayString res(choices.size(), choices.data());

    return res;
}


void VisualControl::post_changed_event()
{
    wxPostEvent(this, wxCommandEvent(EVT_VISUAL_CONTROL));
}


void VisualControl::on_evt_checkbox(wxCommandEvent &)
{
    if (cbox->GetValue()) {
        params.colormap = proton_cmap_gradient;
        params.type = ProtonPlaneParams::PROTON_IMG_GRAD;
    } else {
        params.colormap = proton_colormap;
        params.type = ProtonPlaneParams::PROTON_IMG_DOSE;
    }
    post_changed_event();
}


void VisualControl::on_evt_difftext(wxCommandEvent &e)
{
    wxString str;
    double x;

    str = e.GetString();
    if (str.empty()) {
        e.Skip();
    } else {
        str.ToDouble(&x);
        if (x <= 100.0 && x >= 0) {
            params.pct_diff = x / 100.0;
            post_changed_event();
        } else {
            str.Printf(wxT("%.2f"), params.pct_diff);
            diff->ChangeValue(str);
        }
    }
}


void VisualControl::on_evt_errtext(wxCommandEvent &e)
{
    wxString str;
    double x;

    str = e.GetString();
    if (str.empty()) {
        e.Skip();
    } else {
        str.ToDouble(&x);
        if (x <= 10.0 && x >= 0) {
            params.depth_err = x;
            post_changed_event();
        } else {
            str.Printf(wxT("%.1f"), params.depth_err);
            diff->ChangeValue(str);
        }
    }
}


VisualControl::VisualControl(wxWindow *parent):
    wxPanel(parent),
    cbox(new wxCheckBox(this, wxID_ANY, GRADIENT_SHOW)),
    reset(new wxButton(this, wxID_ANY, GRAD_RESET)),
    diff(new wxTextCtrl(this, wxID_ANY, GRAD_DIFFINIT,
                        wxDefaultPosition, ENTRYSZ)),
    derr(new wxTextCtrl(this, wxID_ANY, GRAD_ERRINIT,
                        wxDefaultPosition, ENTRYSZ)),
    params({ ProtonPlaneParams::PROTON_IMG_DOSE, proton_colormap, 0.03f, 0.5f })
{
    wxFloatingPointValidator<double> valid8tor; /* 8ad 8r8k */
    wxSizer *box, *hboxtop, *hbox, *diffbox, *derrbox;
    wxStaticText *difftxt, *derrtxt;

    box = new wxStaticBoxSizer(wxVERTICAL, this, VISUAL_LABEL);
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hboxtop = new wxBoxSizer(wxHORIZONTAL);
    diffbox = new wxBoxSizer(wxHORIZONTAL);
    derrbox = new wxBoxSizer(wxHORIZONTAL);
    difftxt = new wxStaticText(this, wxID_ANY, GRAD_DIFFLBL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL);
    derrtxt = new wxStaticText(this, wxID_ANY, GRAD_ERRLBL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL);

    hboxtop->Add(cbox, 1);
    hboxtop->Add(reset, 0);
    diffbox->Add(difftxt, 1);
    diffbox->Add(diff, 0);
    derrbox->Add(derrtxt, 1);
    derrbox->Add(derr, 0);
    hbox->Add(diffbox, 1);
    hbox->Add(derrbox, 1);
    box->Add(hboxtop);
    box->Add(hbox, 0, wxEXPAND);
    this->SetSizerAndFit(box);

    valid8tor.SetPrecision(1);
    diff->SetValidator(valid8tor);
    valid8tor.SetPrecision(2);
    derr->SetValidator(valid8tor);

    cbox->SetValue(false);
    cbox->Bind(wxEVT_CHECKBOX, &VisualControl::on_evt_checkbox, this);
    diff->Bind(wxEVT_TEXT, &VisualControl::on_evt_difftext, this);
    derr->Bind(wxEVT_TEXT, &VisualControl::on_evt_errtext, this);
}
