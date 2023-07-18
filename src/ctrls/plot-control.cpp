#include "ctrl-symbols.h"
/* #include "plot-control.h" */
#include "../main-window.h"
#include <wx/filename.h>
#include <wx/valnum.h>
#include <map>

wxDEFINE_EVENT(EVT_PLOT_CONTROL, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLOT_OPEN, wxCommandEvent);

wxDEFINE_EVENT(EVT_PLOTMEAS_DCHANGE, wxCommandEvent);


void PlotMeasurement::post_change_event()
{
    wxCommandEvent e(EVT_PLOTMEAS_DCHANGE);

    wxPostEvent(this, e);
}


static void entry_write_double(wxTextCtrl *ctrl, double x)
{
    wxString str;

    str.Printf(wxT("%.1f"), x);
    ctrl->ChangeValue(str);
}


void PlotMeasurement::on_evt_button(wxCommandEvent &WXUNUSED(e))
{
    if (data) {
        mcc_data_destroy(data);
        data = nullptr;
        btn->SetLabelText(wxT("Load"));
        dctrl->Enable(false);
        dctrl->ChangeValue(wxEmptyString);
        flbl->SetLabelText(wxEmptyString);
    } else {
        wxFileDialog dlg(this, wxT("Load an MCC file"), wxGetApp().get_RS_directory(),
            wxEmptyString, wxT("MCC files (*.mcc)|*.mcc"));
        if (dlg.ShowModal() == wxID_CANCEL) {
            return;
        } else {
            wxString result = dlg.GetPath();
            wxFileName fname(result);
            int envno;
            data = mcc_data_create(result.c_str(), &envno);
            if (data) {
                btn->SetLabelText(wxT("Remove"));
                dctrl->Enable(true);
                flbl->SetLabelText(fname.GetName());
                depth = wxGetApp().get_depth();
                entry_write_double(dctrl, depth / 10.0);
            } else {
                wxString msg;
                msg.Printf(wxT("Failed to load MCC file: %s"), wxString::FromUTF8(mcc_get_error(envno)));
                wxMessageBox(msg, wxT("Load failed"), wxICON_ERROR);
                return;
            }
        }
    }
    post_change_event();
}


void PlotMeasurement::on_evt_text(wxCommandEvent &e)
{
    wxString str;
    double x;

    str = e.GetString();
    if (str.IsEmpty()) {
        e.Skip();
    } else {
        str.ToDouble(&x);
        if ((x >= 0.0) && (x <= wxGetApp().get_max_slider_depth())) {
            depth = x * 10.0;
            post_change_event();
        } else {
            entry_write_double(dctrl, depth / 10.0);
        }
    }
}


PlotMeasurement::PlotMeasurement(wxWindow *parent):
    wxPanel(parent),
    btn(new wxButton(this, wxID_ANY, wxT("Load"))),
    dctrl(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, ENTRYSZ)),
    flbl(new wxStaticText(this, wxID_ANY, wxEmptyString)),
    data(nullptr)
{
    wxFloatingPointValidator<double> v;
    wxBoxSizer *hbox;

    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btn, 0);
    hbox->Add(dctrl, 0, wxEXPAND);
    hbox->Add(flbl, 1, wxEXPAND);
    this->SetSizer(hbox);

    v.SetPrecision(1);
    dctrl->SetValidator(v);
    dctrl->Enable(false);

    btn->Bind(wxEVT_BUTTON, &PlotMeasurement::on_evt_button, this);
    dctrl->Bind(wxEVT_TEXT, &PlotMeasurement::on_evt_text, this);
}


PlotMeasurement::~PlotMeasurement()
{
    mcc_data_destroy(data);
}


void PlotControl::post_change_event()
{
    wxCommandEvent e(EVT_PLOT_CONTROL);

    wxPostEvent(this, e);
}


void PlotControl::on_evt_text(wxCommandEvent &e)
{
    wxString str;

    str = e.GetString();
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
    obtn(new wxButton(this, wxID_ANY, PLOT_OPENLBL)),
    x(0.0), y(0.0),
    measurements({new PlotMeasurement(this),
        new PlotMeasurement(this),
        new PlotMeasurement(this)})
{
    wxFloatingPointValidator<double> v;
    wxBoxSizer *hbox = new wxStaticBoxSizer(wxHORIZONTAL, this, PLOT_LABEL);
    hbox->Add(new wxStaticText(this, wxID_ANY, PLOT_XLABEL, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(xtxt, 1);
    hbox->Add(new wxStaticText(this, wxID_ANY, PLOT_YLABEL, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1);
    hbox->Add(ytxt, 1);
    wxBoxSizer *measbox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Measurements"));
    for (PlotMeasurement *p: measurements) {
        measbox->Add(p, 1, wxEXPAND);
        p->Bind(EVT_PLOTMEAS_DCHANGE, [this](wxCommandEvent &){
                this->post_change_event();
            });
    }
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND);
    vbox->Add(obtn, 1, wxEXPAND);
    vbox->Add(measbox, 0, wxEXPAND);
    this->SetSizer(vbox);

    v.SetPrecision(2);
    xtxt->SetValidator(v);
    ytxt->SetValidator(v);

    xtxt->Bind(wxEVT_TEXT, &PlotControl::on_evt_text, this);
    ytxt->Bind(wxEVT_TEXT, &PlotControl::on_evt_text, this);

    obtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent &e){
            e.SetEventType(EVT_PLOT_OPEN);
            wxPostEvent(this, e);
        });
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


void PlotControl::get_ld_measurements(std::vector<std::tuple<double, double>> &meas) const
{
    double mccx = this->x, mccy = this->y;

    wxGetApp().convert_coordinates(&mccx, &mccy);
    meas.clear();
    for (const PlotMeasurement *p: measurements) {
        if (p->is_loaded()) {
            meas.push_back({p->get_depth(), p->get_dose(mccx, mccy)});
        }
    }
}


void PlotControl::get_pd_measurements(std::vector<std::tuple<double, double>> &meas) const
#warning "Averaging algorithm needs to be rewritten"
{
    std::map<double, std::vector<std::pair<double, long>>> map;

    meas.clear();
    for (const PlotMeasurement *p: measurements) {
        if (p->is_loaded()) {
            map[p->get_depth()].push_back({p->get_sum(), p->get_supp()});
        }
    }
    for (const auto &[depth, ints] : map) {
        double x = 0.0;
        long y = 0;
        for (auto [sum, supp] : ints) {
            x += sum;
            y += supp;
        }
        meas.push_back({depth, x / static_cast<double>(y)});
    }
}


void PlotControl::get_sp_measurements(std::vector<std::tuple<double, double>> &meas) const
{
    meas.clear();
    for (const PlotMeasurement *p: measurements) {
        if (p->is_loaded()) {
            meas.push_back({p->get_depth(), p->get_sum() * ((0.27 * 0.27) * 1e3 / 1405.)});
        }
    }
}
