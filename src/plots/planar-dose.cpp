#include "../main-window.h"


PlanarDosePlot::PlanarDosePlot(wxWindow *parent):
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
#if _WIN32
    this->SetDoubleBuffered(true);
#endif
}

PlanarDosePlot::~PlanarDosePlot()
{

}

void PlanarDosePlot::draw_plot(wxGraphicsContext *gc)
{
    wxDouble w, h;

    gc->GetSize(&w, &h);
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->DrawRectangle(0.0, 0.0, w, h);
}
