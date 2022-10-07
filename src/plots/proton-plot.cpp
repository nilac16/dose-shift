#include "../main-window.h"


void ProtonPlot::on_evt_paint(wxPaintEvent &WXUNUSED(e))
{
    wxPaintDC dc(this);
    if (wxGetApp().dose_loaded()) {
        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        this->draw_plot(gc);
        delete gc;
    }
}

ProtonPlot::ProtonPlot(wxWindow *parent):
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    this->Bind(wxEVT_PAINT, &ProtonPlot::on_evt_paint, this);

#if _WIN32
    this->SetDoubleBuffered(true);
#endif
}
