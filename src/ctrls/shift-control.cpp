#include "ctrl-symbols.h"
#include "shift-control.h"
#include <wx/valnum.h>

wxDEFINE_EVENT(EVT_SHIFT_CONTROL, wxCommandEvent);


void ShiftControl::post_change_event()
{
    wxCommandEvent e(EVT_SHIFT_CONTROL);

    wxPostEvent(this, e);
}


void ShiftControl::write_trig_functions(double degrees) noexcept
{
    degrees *= M_PI / 180.0;
    cos = std::cos(degrees);
    sin = std::sin(degrees);
}


void ShiftControl::on_evt_checkbox(wxCommandEvent &WXUNUSED(e))
{
    post_change_event();
}


void ShiftControl::on_evt_button(wxCommandEvent &WXUNUSED(e))
{
    shfttext1->ChangeValue(SHIFT_ZERO);
    shfttext2->ChangeValue(SHIFT_ZERO);
    anglsldr->SetValue(0);
    angltext->ChangeValue(ANGLE_ZERO);
    x = y = sin = 0.0;
    cos = 1.0;
    post_change_event();
}


void ShiftControl::on_evt_translation(wxCommandEvent &e)
{
    wxString str;

    str = e.GetString();
    if (str.IsEmpty()) {
        e.Skip();
    } else {
        shfttext1->GetValue().ToDouble(&x);
        shfttext2->GetValue().ToDouble(&y);
        post_change_event();
    }
}


void ShiftControl::on_evt_rot_slide(wxScrollEvent &e)
{
    wxString str;
    double x;

    x = static_cast<double>(e.GetInt()) / 10.0;
    str.Printf(wxT("%.1f"), x);
    angltext->ChangeValue(str);
    write_trig_functions(x);
    post_change_event();
}


void ShiftControl::on_evt_rot_text(wxCommandEvent &e)
{
    int min, max, x;
    wxString str;
    double y;

    str = e.GetString();
    if (str.IsEmpty()) {
        e.Skip();
    } else {
        min = anglsldr->GetMin();
        max = anglsldr->GetMax();
        str.ToDouble(&y);
        x = static_cast<int>(y * 10.0);
        if ((x >= min) && (x <= max)) {
            anglsldr->SetValue(x);
            write_trig_functions(y);
            post_change_event();
        } else {
            y = static_cast<double>(anglsldr->GetValue()) / 10;
            str.Printf(wxT("%.1f"), y);
            angltext->ChangeValue(str);
        }
    }
}


ShiftControl::ShiftControl(wxWindow *parent):
    wxPanel(parent),
    enabld(new wxCheckBox(this, wxID_ANY, DETECTOR_SHOW)),
    reset(new wxButton(this, wxID_ANY, DETECTOR_RESET)),
    shfttext1(new wxTextCtrl(this, wxID_ANY, SHIFT_ZERO, wxDefaultPosition, ENTRYSZ)),
    shfttext2(new wxTextCtrl(this, wxID_ANY, SHIFT_ZERO, wxDefaultPosition, ENTRYSZ)),
    anglsldr(new wxSlider(this, wxID_ANY, 0, ANGLE_MIN, ANGLE_MAX)),
    angltext(new wxTextCtrl(this, wxID_ANY, ANGLE_ZERO, wxDefaultPosition, ENTRYSZ)),
    x(0.0), y(0.0), cos(1.0), sin(0.0)
{
    wxFloatingPointValidator<double> v;
    wxBoxSizer *vbox, *hbox;
    vbox = new wxBoxSizer(wxVERTICAL/* , this, DETECTOR_LABEL */);
    hbox = new wxBoxSizer(wxHORIZONTAL);

    /* Add the checkbox and reset button to the vbox */
    hbox->Add(enabld, 1);
    hbox->Add(reset, 0);
    vbox->Add(hbox, 0, wxEXPAND);

    /* Add the x and y shift boxes the the vbox */
    hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, SHIFT_LABEL);
    hbox->Add(new wxStaticText(this, wxID_ANY, SHIFT_XLABEL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(shfttext1, 1);
    hbox->Add(new wxStaticText(this, wxID_ANY, SHIFT_YLABEL,
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(shfttext2, 1);
    vbox->Add(hbox, 0, wxEXPAND);

    /* Add the angle slider to the vbox */
    hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, ANGLE_LABEL);
    hbox->Add(anglsldr, SLIDERSCALE);
    hbox->Add(angltext, ENTRYSCALE);
    vbox->Add(hbox, 0, wxEXPAND);
    this->SetSizerAndFit(vbox);

    /* Activ8te the check8ox and set floating-point valid8tors */
    enabld->SetValue(true);
    v.SetPrecision(2);
    shfttext1->SetValidator(v);
    shfttext2->SetValidator(v);
    v.SetPrecision(1);
    angltext->SetValidator(v);

    enabld->Bind(wxEVT_CHECKBOX, &ShiftControl::on_evt_checkbox, this);
    reset->Bind(wxEVT_BUTTON, &ShiftControl::on_evt_button, this);
    shfttext1->Bind(wxEVT_TEXT, &ShiftControl::on_evt_translation, this);
    shfttext2->Bind(wxEVT_TEXT, &ShiftControl::on_evt_translation, this);
    anglsldr->Bind(wxEVT_SCROLL_TOP, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_BOTTOM, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_LINEUP, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_LINEDOWN, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_PAGEUP, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_PAGEDOWN, &ShiftControl::on_evt_rot_slide, this);
    anglsldr->Bind(wxEVT_SCROLL_THUMBTRACK, &ShiftControl::on_evt_rot_slide, this);
    angltext->Bind(wxEVT_TEXT, &ShiftControl::on_evt_rot_text, this);
}


void ShiftControl::get_affine(double affine[]) const noexcept
{
    affine[0] = affine[3] = cos;
    affine[1] = -sin;
    affine[2] = sin;
    affine[4] = (x * affine[0] + y * affine[2]) * 10.0;
    affine[5] = (x * affine[1] + y * affine[3]) * 10.0;
}


void ShiftControl::set_translation(double x, double y)
{
    wxString str;

    this->x = (cos * x - sin * y) / 10.0;
    this->y = (sin * x + cos * y) / 10.0;
    str.Printf(wxT("%.2f"), this->x);
    shfttext1->ChangeValue(str);
    str.Printf(wxT("%.2f"), this->y);
    shfttext2->ChangeValue(str);
    post_change_event();
}


void ShiftControl::convert_coordinates(double *x, double *y) const noexcept
{
    double tmp = *x;

    *x = cos * tmp - sin * *y - 10.0 * this->x;
    *y = sin * tmp + cos * *y - 10.0 * this->y;
}
