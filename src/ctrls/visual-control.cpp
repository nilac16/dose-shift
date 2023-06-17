#include <array>
#include "visual-control.h"
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


void VisualControl::write_cmap()
{
    /* static void (*cmaptbl[])(float, unsigned char *) = {
        proton_colormap,
        proton_cmap_turbo,
        proton_cmap_viridis
    };
    int sel;

    sel = lbox->GetSelection();
    cmap_func = cmaptbl[sel]; */
    cmap_func = proton_colormap;
}


void VisualControl::on_evt_checkbox(wxCommandEvent &)
{
    if (cbox->GetValue()) {
        cmap_func = proton_cmap_gradient;
        mode = PROTON_IMG_GRAD;
    } else {
        write_cmap();
        mode = PROTON_IMG_DOSE;
    }
    post_changed_event();
}


void VisualControl::on_evt_choice(wxCommandEvent &)
{
    if (mode == PROTON_IMG_DOSE) {
        write_cmap();
        post_changed_event();
    }
}


VisualControl::VisualControl(wxWindow *parent):
    wxPanel(parent),
    cbox(new wxCheckBox(this, wxID_ANY, wxT("Show gradient"))),
    //lbox(new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices())),
    cmap_func(proton_colormap),
    mode(PROTON_IMG_DOSE)
{
    wxSizer *box = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Visualizer"));
    //wxSizer *listbox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Dose colormap"));
    //listbox->Add(lbox, wxEXPAND);
    box->Add(cbox, wxEXPAND);
    //box->Add(listbox, wxEXPAND);
    this->SetSizerAndFit(box);

    cbox->SetValue(false);
    cbox->Bind(wxEVT_CHECKBOX, &VisualControl::on_evt_checkbox, this);

    //lbox->SetSelection(0);
    //lbox->Bind(wxEVT_CHOICE, &VisualControl::on_evt_choice, this);
}
