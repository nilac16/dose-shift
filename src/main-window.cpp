#include "main-window.h"

#define MAIN_TITLE wxT("QA shift visualizer")

wxIMPLEMENT_APP(MainApplication);


void MainApplication::initialize_main_window()
{
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    frame = new wxFrame(nullptr, wxID_ANY, MAIN_TITLE,
        wxDefaultPosition, wxSize(1280, 720));
    canv = new DoseWindow(frame);
    cwnd = new CtrlWindow(frame);
    lwnd = new LoadWindow(frame);
    hbox->Add(canv, 1, wxEXPAND);
    hbox->Add(cwnd, 0, wxEXPAND);
    vbox->Add(hbox, 1, wxEXPAND);
    vbox->Add(lwnd, 0, wxEXPAND);
    frame->SetSizer(vbox);

    pwnd = new PlotWindow(frame);

    lwnd->Bind(wxEVT_FILEPICKER_CHANGED, &MainApplication::on_dicom_load, this);
    cwnd->Bind(EVT_DEPTH_CONTROL, &DoseWindow::on_depth_changed, canv);
    cwnd->Bind(EVT_PLOT_CONTROL, &MainApplication::on_plot_change, this);
    cwnd->Bind(EVT_SHIFT_CONTROL, &DoseWindow::on_shift_changed, canv);
    cwnd->Bind(wxEVT_BUTTON, &MainApplication::on_plot_open, this);
}

void MainApplication::on_dicom_load(wxFileDirPickerEvent &e)
{
    wxString str = e.GetPath();
    canv->load_file(str.c_str());
    if (canv->dose_loaded()) {
        cwnd->set_max_depth(canv->get_max_depth());
        pwnd->new_dose_loaded();
    }
}

void MainApplication::on_plot_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_plot_changed(e);
        pwnd->write_line_dose();
        pwnd->redraw();
    }
}

void MainApplication::on_plot_open(wxCommandEvent &WXUNUSED(e))
{
    if (!pwnd->IsVisible()) {
        pwnd->Show();
    } else {
        pwnd->Raise();
    }
}

float MainApplication::get_depth() const
{
    return cwnd->get_depth();
}

bool MainApplication::detector_enabled() const
{
    return cwnd->detector_enabled();
}

void MainApplication::get_detector_affine(double affine[]) const noexcept
{
    cwnd->get_detector_affine(affine);
}

void MainApplication::get_line_dose(double *x, double *y) const noexcept
{
    cwnd->get_line_dose(x, y);
}

void MainApplication::set_line_dose(double x, double y)
{
    cwnd->set_line_dose(x, y);
}

void MainApplication::set_translation(double x, double y)
{
    cwnd->set_translation(x, y);
}

bool MainApplication::dose_loaded() const noexcept
{
    return canv->dose_loaded();
}

const ProtonDose *MainApplication::get_dose() const noexcept
{
    return canv->get_dose();
}

double MainApplication::get_max_slider_depth() const
{
    return cwnd->get_max_slider_depth();
}

double MainApplication::get_max_dose() const noexcept
{
    return static_cast<double>(proton_dose_max(canv->get_dose()));
}

void MainApplication::unload_dose() noexcept
{
    canv->unload_dose();
}

bool MainApplication::OnInit()
{
    try {
        initialize_main_window();
        frame->Show();
        return true;
    } catch (std::bad_alloc &e) {
        wxString str;
        str.Printf(wxT("Bad alloc thrown while initializing window: %s"), e.what());
        wxMessageBox(str, wxT("Init failed"), wxICON_ERROR);
        return false;
    } catch (std::exception &e) {
        wxString str;
        str.Printf(wxT("Failed to initialize the main window: %s"), e.what());
        wxMessageBox(str, wxT("Init failed"), wxICON_ERROR);
        return false;
    }    
}
