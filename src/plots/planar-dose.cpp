#include "../main-window.h"


PlanarDosePlot::PlanarDosePlot(wxWindow *parent):
    ProtonPlot(parent)
{

}

void PlanarDosePlot::draw_plot(wxGraphicsContext *gc)
{
    wxDouble w, h;

    gc->GetSize(&w, &h);
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->DrawRectangle(0.0, 0.0, w, h);
}
