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
    pwnd = new PlotWindow(frame);

    hbox->Add(canv, 1, wxEXPAND);
    hbox->Add(cwnd, 0, wxEXPAND);
    vbox->Add(hbox, 1, wxEXPAND);
    vbox->Add(lwnd, 0, wxEXPAND);
    frame->SetSizer(vbox);

    lwnd->Bind(wxEVT_FILEPICKER_CHANGED, &MainApplication::on_dicom_load, this);

    /** Depth was changed */
    cwnd->Bind(EVT_DEPTH_CONTROL, &MainApplication::on_depth_change, this);

    /** Line dose marker, measurement file or measurement depth were changed */
    cwnd->Bind(EVT_PLOT_CONTROL, &MainApplication::on_plot_change, this);

    /** Detector window was manipulated at all */
    cwnd->Bind(EVT_SHIFT_CONTROL, &MainApplication::on_shift_change, this);

    /** Visualization mode was changed */
    cwnd->Bind(EVT_VISUAL_CONTROL, &MainApplication::on_visual_change, this);

    /** "Open plot window" button pressed */
    cwnd->Bind(EVT_PLOT_OPEN, &MainApplication::on_plot_open, this);
}

void MainApplication::on_dicom_load(wxFileDirPickerEvent &e)
{
    wxString str = e.GetPath();
    canv->load_file(str.c_str());
    pwnd->on_dicom_changed(e);
}

void MainApplication::on_depth_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_depth_changed(e);
        pwnd->on_depth_changed(e);
    }
}

void MainApplication::on_plot_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_plot_changed(e);
        pwnd->on_plot_changed(e);
    }
}

void MainApplication::on_shift_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_shift_changed(e);
        pwnd->on_shift_changed(e);
    }
}

void MainApplication::on_visual_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_depth_changed(e);  /* yikes asymmetric */
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

void MainApplication::set_depth_range()
{
    float range[2];
    canv->get_depth_range(range);
    cwnd->set_depth_range(range[0], range[1]);
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
