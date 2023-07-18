#include "main-window.h"

#define MAIN_TITLE wxT("QA shift visualizer")

wxIMPLEMENT_APP(MainApplication);


void MainApplication::initialize_main_window()
{
    wxBoxSizer *hbox, *vbox;

    hbox = new wxBoxSizer(wxHORIZONTAL);
    vbox = new wxBoxSizer(wxVERTICAL);
    main_frame() = new wxFrame(nullptr,
                               wxID_ANY,
                               MAIN_TITLE,
                               wxDefaultPosition,
                               wxSize(1280, 720));
    canvas() = new DoseWindow(main_frame());
    ctrl_wnd() = new CtrlWindow(main_frame());
    load_wnd() = new LoadWindow(main_frame());
    plot_wnd() = new PlotWindow(main_frame());

    hbox->Add(canvas(), 1, wxEXPAND);
    hbox->Add(ctrl_wnd(), 0, wxEXPAND);
    vbox->Add(hbox, 1, wxEXPAND);
    vbox->Add(load_wnd(), 0, wxEXPAND);
    main_frame()->SetSizer(vbox);

    load_wnd()->Bind(wxEVT_FILEPICKER_CHANGED,
                     &MainApplication::on_dicom_load,
                     this);
    /** Depth was changed */
    ctrl_wnd()->Bind(EVT_DEPTH_CONTROL,
                     &MainApplication::on_depth_change,
                     this);
    /** Line dose marker, measurement file or measurement depth were changed */
    ctrl_wnd()->Bind(EVT_PLOT_CONTROL,
                     &MainApplication::on_plot_change,
                     this);
    /** Detector window was manipulated at all */
    ctrl_wnd()->Bind(EVT_SHIFT_CONTROL,
                     &MainApplication::on_shift_change,
                     this);
    /** Visualization mode was changed */
    ctrl_wnd()->Bind(EVT_VISUAL_CONTROL,
                     &MainApplication::on_visual_change,
                     this);
    /** "Open plot window" button pressed */
    ctrl_wnd()->Bind(EVT_PLOT_OPEN,
                     &MainApplication::on_plot_open,
                     this);
}


void MainApplication::load_file(const wxString &path)
{
    canvas()->load_file(path.c_str());
    plot_wnd()->on_dicom_changed();
}


void MainApplication::on_dicom_load(wxFileDirPickerEvent &e)
{
    wxString str;
    
    str = e.GetPath();
    load_file(e.GetPath());
}


void MainApplication::on_depth_change(wxCommandEvent &e)
{
    if (canvas()->dose_loaded()) {
        canvas()->on_depth_changed(e);
        ctrl_wnd()->on_depth_changed(e);
        plot_wnd()->on_depth_changed(e);
    }
}


void MainApplication::on_plot_change(wxCommandEvent &e)
{
    if (canvas()->dose_loaded()) {
        canvas()->on_plot_changed(e);
        plot_wnd()->on_plot_changed(e);
    }
}


void MainApplication::on_shift_change(wxCommandEvent &e)
{
    if (canvas()->dose_loaded()) {
        canvas()->on_shift_changed(e);
        plot_wnd()->on_shift_changed(e);
    }
}


void MainApplication::on_visual_change(wxCommandEvent &e)
{
    if (canvas()->dose_loaded()) {
        canvas()->on_depth_changed(e);
    }
}


void MainApplication::on_plot_open(wxCommandEvent &WXUNUSED(e))
{
    if (!plot_wnd()->IsVisible()) {
        plot_wnd()->Show();
    } else {
        plot_wnd()->Raise();
    }
}


void MainApplication::set_depth_range()
{
    float range[2];

    canvas()->get_depth_range(range);
    ctrl_wnd()->set_depth_range(range[0], range[1]);
}


void MainApplication::dropped_file(const wxString &path)
/** This works on Windows, but not GTK... Do I just #if __linux or something...
 *  Ok never mind, it does work on GTK, it just doesn't update the picker
 *  control text
 */
{
    load_wnd()->set_file(path);
    load_file(path);
}


bool MainApplication::OnInit()
{
    bool res = false;
    wxString str;

    try {
        initialize_main_window();
        main_frame()->Show();
        res = true;
    } catch (std::bad_alloc &e) {
        str.Printf(wxT("Bad alloc thrown while initializing window: %s"), e.what());
        wxMessageBox(str, wxT("Init failed"), wxICON_ERROR);
    } catch (std::exception &e) {
        str.Printf(wxT("Failed to initialize the main window: %s"), e.what());
        wxMessageBox(str, wxT("Init failed"), wxICON_ERROR);
    }
    return res;
}
