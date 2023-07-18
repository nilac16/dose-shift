#include <array>
#include "../main-window.h"
#include "visual-control.h"
#include "ctrl-symbols.h"
#include <wx/valnum.h>
#include "../proton-aux.h"
#include "../proton/proton-dose.h"

#define DOSE_DIFF_INIT 0.03
#define DEPTH_ERR_INIT 0.5

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


void VisualControl::set_auto_error()
{
    double depth;
    wxString str;

    depth = wxGetApp().get_depth();
    params.depth_err = proton_buildup_err(depth);
    write_err();
    post_changed_event();
}


void VisualControl::post_changed_event()
{
    wxPostEvent(this, wxCommandEvent(EVT_VISUAL_CONTROL));
}


void VisualControl::write_diff()
{
    wxString str;

    str.Printf(wxT("%.1f"), params.pct_diff * 100.0);
    diff->ChangeValue(str);
}


void VisualControl::write_err()
{
    wxString str;

    str.Printf(wxT("%.2f"), params.depth_err);
    derr->ChangeValue(str);
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


void VisualControl::on_evt_reset(wxCommandEvent &)
{
    params.pct_diff = DOSE_DIFF_INIT;
    write_diff();
    params.depth_err = DEPTH_ERR_INIT;
    write_err();
    autocalc->SetValue(true);
    derr->Enable(false);
    set_auto_error();
    post_changed_event();
}


void VisualControl::on_evt_automatic(wxCommandEvent &e)
{
    derr->Enable(!e.IsChecked());
    if (e.IsChecked()) {
        set_auto_error();
    }
    post_changed_event();
}


void VisualControl::on_evt_difftext(wxCommandEvent &e)
{
    constexpr double range[] = { 0.0, 100.0 };
    wxString str;
    double x;

    str = e.GetString();
    if (str.empty()) {
        e.Skip();
    } else {
        str.ToDouble(&x);
        if (x <= range[1] && x >= range[0]) {
            params.pct_diff = x / 100.0;
            post_changed_event();
        } else {
            write_diff();
        }
    }
}


void VisualControl::on_evt_errtext(wxCommandEvent &e)
{
    constexpr double range[] = { 0.0, 10.0 };
    wxString str;
    double x;

    str = e.GetString();
    if (str.empty()) {
        e.Skip();
    } else {
        str.ToDouble(&x);
        if (x <= range[1] && x >= range[0]) {
            params.depth_err = x;
            post_changed_event();
        } else {
            write_err();
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
    autocalc(new wxCheckBox(this, wxID_ANY, GRADIENT_AUTO)),
    params({
        ProtonPlaneParams::PROTON_IMG_DOSE,
        proton_colormap,
        DOSE_DIFF_INIT,
        DEPTH_ERR_INIT
    })
{
    wxFloatingPointValidator<double> valid8tor;
    wxSizer *vbox_master, *vbox_params, *hbox_show, *grid;
    wxStaticText *difftxt, *derrtxt;

    vbox_master = new wxBoxSizer(wxVERTICAL);
    vbox_params = new wxStaticBoxSizer(wxVERTICAL, this, VISUAL_LABEL);
    hbox_show = new wxBoxSizer(wxHORIZONTAL);
    grid = new wxGridSizer(2, 0, 10);
    difftxt = new wxStaticText(this, wxID_ANY, GRAD_DIFFLBL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL);
    derrtxt = new wxStaticText(this, wxID_ANY, GRAD_ERRLBL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL);

    grid->Add(difftxt, 0);
    grid->Add(diff, 0);
    grid->Add(derrtxt, 0);
    grid->Add(derr, 0);
    vbox_params->Add(grid, wxSizerFlags().Expand());
    vbox_params->Add(autocalc);
    hbox_show->Add(cbox, 1);
    hbox_show->Add(reset, 0);
    vbox_master->Add(hbox_show, wxSizerFlags().Expand());
    vbox_master->Add(vbox_params, wxSizerFlags().Expand());
    this->SetSizerAndFit(vbox_master);

    valid8tor.SetPrecision(1);
    diff->SetValidator(valid8tor);
    valid8tor.SetPrecision(2);
    derr->SetValidator(valid8tor);

    cbox->SetValue(false);
    autocalc->SetValue(true);
    derr->Enable(false);
    cbox->Bind(wxEVT_CHECKBOX, &VisualControl::on_evt_checkbox, this);
    reset->Bind(wxEVT_BUTTON, &VisualControl::on_evt_reset, this);
    autocalc->Bind(wxEVT_CHECKBOX, &VisualControl::on_evt_automatic, this);
    diff->Bind(wxEVT_TEXT, &VisualControl::on_evt_difftext, this);
    derr->Bind(wxEVT_TEXT, &VisualControl::on_evt_errtext, this);
}


void VisualControl::on_depth_changed(wxCommandEvent &)
{
    if (automatic()) {
        set_auto_error();
    }
}
