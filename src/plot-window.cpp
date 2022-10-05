#include <wx/clipbrd.h>
#include "main-window.h"

#define DEFWINDOWSZ wxSize(640, 480)


void PlotWindow::on_evt_close(wxCloseEvent &WXUNUSED(e))
{
    long winstyle = this->GetWindowStyleFlag();
    this->Show(false);
    /* Correct retention of the stay on top flag varies wildly by user 
    environment. Best to just nuke it */
    if (winstyle & wxSTAY_ON_TOP) {
        this->SetWindowStyleFlag(winstyle ^ wxSTAY_ON_TOP);
    }
}

void PlotWindow::on_context_menu_selection(wxCommandEvent &e)
{
    switch (e.GetId()) {
    case 0:
        this->SetWindowStyleFlag(this->GetWindowStyleFlag() ^ wxSTAY_ON_TOP);
        break;
    case 1:
        if (wxGetApp().dose_loaded()) {
            wxBitmap bmp(IMAGE_RES, 24);
            wxMemoryDC dc(bmp);
            wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
            if (wxTheClipboard->Open()) {
                get_current_plot()->draw_plot(gc);
                wxTheClipboard->SetData(new wxBitmapDataObject(bmp));
                wxTheClipboard->Close();
            }
            delete gc; // omfg stop forgetting to do this
        }
        break;
    case 2:
        this->SetSize(DEFWINDOWSZ);
        break;
    default:
        break;
    }
}

void PlotWindow::on_context_menu(wxContextMenuEvent &e)
{
    wxMenu menu;
    menu.AppendCheckItem(0, wxT("Always on top"));
    menu.Append(1, wxT("Copy image"));
    menu.Append(2, wxT("Reset window size"));
    menu.Check(0, this->GetWindowStyleFlag() & wxSTAY_ON_TOP);
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &PlotWindow::on_context_menu_selection, this);
    PopupMenu(&menu);
    e.Skip();
}

PlotWindow::PlotWindow(wxWindow *parent):
    wxFrame(parent, wxID_ANY, wxT("Plot window"), wxDefaultPosition, DEFWINDOWSZ),
    nb(new wxNotebook(this, wxID_ANY)),
    ldplot(new LineDosePlot(nb)),
    pdplot(new PlanarDosePlot(nb))
{
    nb->AddPage(ldplot, wxT("Line dose"));
    nb->AddPage(pdplot, wxT("Planar dose"));

    this->Bind(wxEVT_CLOSE_WINDOW, &PlotWindow::on_evt_close, this);
    this->Bind(wxEVT_CONTEXT_MENU, &PlotWindow::on_context_menu, this);

    /* Cannot copy the image to clipboard without this */
    wxImage::AddHandler(new wxPNGHandler);
}

PlotWindow::~PlotWindow()
{
    
}

void PlotWindow::on_depth_changed(wxCommandEvent &WXUNUSED(e))
{
    /* Both plots' markers must be redrawn */
    nb->GetCurrentPage()->Refresh();
}

void PlotWindow::on_plot_changed(wxCommandEvent &WXUNUSED(e))
{
    /* Line dose needs to be updated, and measurements reinterpolated */
    ldplot->write_line_dose();
    nb->GetCurrentPage()->Refresh();
}

void PlotWindow::on_shift_changed(wxCommandEvent &WXUNUSED(e))
{
    /* Measurements need to be reinterpolated */
    nb->GetCurrentPage()->Refresh();
}

void PlotWindow::new_dose_loaded()
{
    ldplot->new_dose_loaded();
}

void PlotWindow::dose_unloaded()
{
    ldplot->dose_unloaded();
}
