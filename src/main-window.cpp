#include <csignal>
#include "main-window.h"

#if defined _MSC_VER && _MSC_VER
typedef _crt_signal_t sighandler_t;
#endif

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
    cwnd->Bind(EVT_DEPTH_CONTROL, &MainApplication::on_depth_change, this);
    cwnd->Bind(EVT_PLOT_CONTROL, &MainApplication::on_plot_change, this);
    cwnd->Bind(EVT_SHIFT_CONTROL, &MainApplication::on_shift_change, this);
    cwnd->Bind(EVT_PLOT_OPEN, &MainApplication::on_plot_open, this);
}

void MainApplication::on_dicom_load(wxFileDirPickerEvent &e)
{
    wxString str = e.GetPath();
    canv->load_file(str.c_str());
    if (canv->dose_loaded()) {
        pwnd->new_dose_loaded();
    } else {
        /* What resources must be destroyed if the DICOM is unloaded? 
        The dose window maintains a resizable pixel buffer, and simply 
        doesn't draw it if a dose is not loaded
        The plot window allocates a new line buffer when the dose is 
        loaded, so that must be destroyed */
        pwnd->dose_unloaded();
    }
}

void MainApplication::on_depth_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_depth_changed(e);
        pwnd->redraw();
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

void MainApplication::on_shift_change(wxCommandEvent &e)
{
    if (canv->dose_loaded()) {
        canv->on_shift_changed(e);
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

void MainApplication::set_depth_range()
{
    float range[2];
    canv->get_depth_range(range);
    cwnd->set_depth_range(range[0], range[1]);
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

void MainApplication::get_measurements(std::vector<std::pair<double, double>> &meas) const
{
    cwnd->get_measurements(meas);
}

wxString MainApplication::get_RS_directory() const
{
    return lwnd->get_directory();
}

void MainApplication::convert_coordinates(double *x, double *y) const noexcept
{
    return cwnd->convert_coordinates(x, y);
}

/** Displays a modal message box informing the user that the application 
 *  state is irreparably beans'd, then unhooks itself so the OS can have its 
 *  way with the app
 * 
 *  If this ever crashes on Windows, the signal handler will keep the application 
 *  running until the user switches back to it to see this. I have wondered many 
 *  times whether I closed the application by accident, or if it silently crashed 
 *  on the other workspace
 * 
 *  Currently trapping (on all platforms):
 *   - Illegal instructions (I might want to use vector instructions)
 *   - Segmentation faults
 *
 *  Also trapping on POSIX-compliant platforms:
 *   - Bus error
 *   - Bad syscall
 * 
 *  Not trapping, but including commented-out code:
 *   - Floating-point exceptions (If I become concerned that they may occur, they 
 *     can be trapped. I would prefer to avoid them in the code. At worst, the 
 *     result should simply be unloading the current DICOM and resetting the fpenv)
 */
static void main_signal_handler(int sig/*, int fpcode */)
{
    static const wxString title(wxT("Signal caught"));
    wxString message;
    switch (sig) {
    case SIGILL:
        message = wxT("The application has raised a SIGILL (illegal instruction) and must now terminate.");
        break;
    /* case SIGFPE:
        message = wxT("The application has raised a SIGFPE (floating point exception) and must now terminate.");
        break; */
    case SIGSEGV:
        message = wxT("The application has raised a SIGSEGV (segmentation fault) and must now terminate.");
        break;
#if !_WIN32
    case SIGBUS:
        message = wxT("Bus error");
        break;
    case SIGSYS:
        message = wxT("Bad system call");
        break;
#endif
    }
    wxMessageBox(message, title, wxICON_ERROR);
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

static void main_signal_register(void)
{
    std::signal(SIGILL, (sighandler_t)main_signal_handler);
    /* std::signal(SIGFPE, (sighandler_t)main_signal_handler); */
    std::signal(SIGSEGV, (sighandler_t)main_signal_handler);
#if !_WIN32
    std::signal(SIGBUS, (sighandler_t)main_signal_handler);
    std::signal(SIGSYS, (sighandler_t)main_signal_handler);
#endif
}

bool MainApplication::OnInit()
{
    main_signal_register();
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
