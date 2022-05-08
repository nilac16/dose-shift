#include "ctrl-symbols.h"
#include "depth-control.h"
#include <wx/valnum.h>

wxDEFINE_EVENT(EVT_DEPTH_CONTROL, wxCommandEvent);


void DepthControl::post_change_event()
{
    wxCommandEvent e(EVT_DEPTH_CONTROL);
    e.SetInt(slider->GetValue()); 
    wxPostEvent(this, e);
}

void DepthControl::on_evt_slider(wxScrollEvent &e)
{
    wxString str;
    str.Printf(wxT("%d"), e.GetInt());
    entry->ChangeValue(str);
    post_change_event();
}

void DepthControl::on_evt_text(wxCommandEvent &e)
{
    wxString str = e.GetString();
    if (str.IsEmpty()) {
        e.Skip();
    } else {
        const unsigned int max = static_cast<int>(slider->GetMax());
        unsigned long x;
        str.ToULong(&x);
        if (/* (x >= 0) &&  */(x <= max)) {
            slider->SetValue(static_cast<int>(x));
            post_change_event();
        } else {
            str.Printf(wxT("%d"), slider->GetValue());
            entry->ChangeValue(str);
        }
    }
}

DepthControl::DepthControl(wxWindow *parent):
    wxPanel(parent),
    slider(new wxSlider(this, wxID_ANY, 0, 0, DEPTH_INIT_MAX)),
    entry(new wxTextCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, ENTRYSZ))
{
    wxBoxSizer *hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, DEPTH_LABEL);
    hbox->Add(slider, SLIDERSCALE);
    hbox->Add(entry, ENTRYSCALE);
    this->SetSizerAndFit(hbox);

    entry->SetValidator(wxIntegerValidator<unsigned int>());
    /* entry->SetMaxLength(3); Not needed */

    slider->Bind(wxEVT_SCROLL_TOP, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_BOTTOM, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_LINEUP, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_LINEDOWN, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_PAGEUP, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_PAGEDOWN, &DepthControl::on_evt_slider, this);
    slider->Bind(wxEVT_SCROLL_THUMBTRACK, &DepthControl::on_evt_slider, this);
    entry->Bind(wxEVT_TEXT, &DepthControl::on_evt_text, this);
}

int DepthControl::get_value() const
{
    return slider->GetValue();
}

void DepthControl::set_value(int x)
{
    slider->SetValue(x);
    entry->ChangeValue(wxT("0"));
}

void DepthControl::set_max_depth(int depth)
{
    slider->SetMax(depth);
}

int DepthControl::get_max() const
{
    return slider->GetMax();
}
