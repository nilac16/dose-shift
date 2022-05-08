#include "main-window.h"
#include <wx/graphics.h>


void PlotWindow::on_evt_paint(wxPaintEvent &WXUNUSED(e))
{
    constexpr double hmargins[2] = { 0.1, 0.1 };
    constexpr double vmargins[2] = { 0.1, 0.1 };
    wxPaintDC dc(this);
    if (wxGetApp().dose_loaded()) {
        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        double w, h, tikwidth;
        gc->SetBrush(*wxWHITE_BRUSH);
        gc->GetSize(&w, &h);
        gc->DrawRectangle(0.0, 0.0, w, h);
        gc->SetBrush(wxNullBrush);
        gc->SetPen(*wxBLACK_PEN);
        gc->Translate(w * hmargins[0], h * (1.0 - vmargins[0]));
        w *= (1.0 - hmargins[0] - hmargins[1]);
        h *= (1.0 - vmargins[0] - vmargins[1]);
        tikwidth = (w < h) ? 0.01 * w : 0.01 * h;
        {
            wxGraphicsPath p = gc->CreatePath();
            p.MoveToPoint(0.0, 0.0);
            p.AddLineToPoint(0.0, -h);
            p.MoveToPoint(0.0, 0.0);
            p.AddLineToPoint(w, 0.0);
            gc->StrokePath(p);
        }
        gc->Scale(wxGetApp().get_max_slider_depth() / w, -wxGetApp().get_max_dose() / h);
        w = wxGetApp().get_max_slider_depth();
        h = wxGetApp().get_max_dose();
        {
            const unsigned nv = 5, nh = 6;
            const double vinc = h / static_cast<double>(nv), hinc = w / static_cast<double>(nh);
            wxGraphicsPath p = gc->CreatePath();
            unsigned i;
            gc->SetFont(*wxNORMAL_FONT, *wxBLACK);
            for (i = 0; i <= nv; i++) {
                const double vtik = static_cast<double>(i) * vinc;
                p.MoveToPoint(-100.0 * tikwidth, vtik);
                p.AddLineToPoint(0.0, vtik);
            }
            for (i = 0; i <= nh; i++) {
                const double htik = static_cast<double>(i) * hinc;
                p.MoveToPoint(htik, -100.0 * tikwidth);
                p.AddLineToPoint(htik, 0.0);
            }
            gc->StrokePath(p);
            gc->BeginLayer(0.5);
            gc->SetPen(*wxBLACK_DASHED_PEN);
            for (i = 1; i <= nv; i++) {
                const double vtik = static_cast<double>(i) * vinc;
                p.MoveToPoint(0.0, vtik);
                p.AddLineToPoint(w, vtik);
            }
            for (i = 1; i <= nh; i++) {
                const double htik = static_cast<double>(i) * hinc;
                p.MoveToPoint(htik, 0.0);
                p.AddLineToPoint(htik, h);    
            }
            gc->StrokePath(p);
            gc->EndLayer();
        }
        delete gc;
    }
}

void PlotWindow::on_evt_close(wxCloseEvent &WXUNUSED(e))
{
    this->Show(false);
}

PlotWindow::PlotWindow(wxWindow *parent):
    wxFrame(parent, wxID_ANY, wxT("Plot window"), wxDefaultPosition, wxSize(640, 480)),
    ndose(0), line_dose(nullptr)
{
    this->Bind(wxEVT_CLOSE_WINDOW, &PlotWindow::on_evt_close, this);
    this->Bind(wxEVT_PAINT, &PlotWindow::on_evt_paint, this);
}
