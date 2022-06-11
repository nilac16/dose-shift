#include <algorithm>
#include <array>
#include "main-window.h"
#include <wx/graphics.h>
#include <cmath>

#ifndef M_PI_2 /* Since nobody but glibc wants to #def this */
#   define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif

#define DOSEMAX_MULT    1.0
#define DEPTHMAX_MULT   1.1

#define GRID_OPACITY        0.2
#define LINEDOSE_OPACITY    0.8

#define MIN_XTICKS      6
#define MIN_YTICKS      5


void PlotWindow::draw_measurements(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                                   const wxPoint2DDouble &pwidth)
{
    const double dosescale = pwidth.m_y / yticks.back();
    const double depthscale = pwidth.m_x / static_cast<double>(xticks.back().second);
    const double boxwidth = (pwidth.m_x > pwidth.m_y) ? pwidth.m_x / 60.0 : pwidth.m_y / 60.0;
    std::vector<std::pair<double, double>> measurements;
    wxGetApp().get_measurements(measurements);
    gc->SetPen(measpen);
    if (measurements.empty()) {
        const double depth = wxGetApp().get_depth();
        const double x = std::fma(depth, depthscale, porigin.m_x);
        const double y = std::fma(proton_line_get_dose(line, depth), dosescale, porigin.m_y);
        gc->StrokeLine(x, porigin.m_y, x, y);
    } else {
        for (std::pair<double, double> &meas : measurements) {
            const double x = std::fma(meas.first, depthscale, porigin.m_x);
            const double y = std::fma(meas.second, dosescale, porigin.m_y);
            gc->DrawRectangle(x - boxwidth / 2, y - boxwidth / 2,
                boxwidth, boxwidth);
            meas.second = (meas.second - proton_line_get_dose(line, meas.first)) / proton_line_get_dose(line, meas.first);
            meas.first = x;
        }
        gc->SetPen(diffpen);
        for (const std::pair<double, double> &meas : measurements) {
            const double x = meas.first;
            const double y = porigin.m_y + 0.5 * pwidth.m_y + 10 * pwidth.m_y * meas.second;
            gc->DrawEllipse(x - boxwidth / 2, y - boxwidth / 2, boxwidth, boxwidth);
        }
    }
}

void PlotWindow::draw_line_dose(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                                const wxPoint2DDouble &pwidth)
{
    const double dosescale = pwidth.m_y / yticks.back();
    const double depthscale = pwidth.m_x * proton_dose_spacing(wxGetApp().get_dose(), 1) / static_cast<double>(xticks.back().second);
    const float *ld = proton_line_raw(line);
    wxGraphicsPath p;
    long i;
    p = gc->CreatePath();
    gc->SetPen(dosepen);
    p.MoveToPoint(porigin.m_x,
        std::fma(static_cast<double>(*ld), dosescale, porigin.m_y));
    ld++;
    for (i = 1; i < proton_line_length(line); i++, ld++) {
        const double dose = static_cast<double>(*ld);
        p.AddLineToPoint(
            std::fma(static_cast<double>(i), depthscale, porigin.m_x),
            std::fma(dose, dosescale, porigin.m_y));
    }
    gc->StrokePath(p);
}

void PlotWindow::draw_dashes(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                             const wxPoint2DDouble &pwidth)
{
    const double tikscale = pwidth.m_y / static_cast<double>(yticks.size() - 1);
    wxGraphicsPath p = gc->CreatePath();
    unsigned i;
    /* gc->BeginLayer(GRID_OPACITY); */
    gc->Clip(porigin.m_x, porigin.m_y, pwidth.m_x, pwidth.m_y);
    gc->SetPen(dashpen);
    for (i = 1; i < xticks.size(); i++) {
        const double tikx = std::fma(xticks[i].first, pwidth.m_x, porigin.m_x);
        p.MoveToPoint(tikx, porigin.m_y);
        p.AddLineToPoint(tikx, porigin.m_y + pwidth.m_y);
    }
    for (i = 1; i < yticks.size(); i++) {
        const double tiky = std::fma(static_cast<double>(i), tikscale, porigin.m_y);
        p.MoveToPoint(porigin.m_x, tiky);
        p.AddLineToPoint(porigin.m_x + pwidth.m_x, tiky);
    }
    gc->StrokePath(p);
    /* gc->EndLayer(); */
}

void PlotWindow::draw_xaxis(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                            const wxPoint2DDouble &pwidth, double tikwidth)
{
    wxGraphicsPath p = gc->CreatePath();
    double txw, txh;
    wxString lbl;
    gc->SetFont(this->GetFont().MakeSmaller(), *wxBLACK);
    p.MoveToPoint(porigin);
    p.AddLineToPoint(porigin.m_x + pwidth.m_x, porigin.m_y);
    for (const auto &pr : xticks) {
        const double tikx = std::fma(pr.first, pwidth.m_x, porigin.m_x);
        p.MoveToPoint(tikx, porigin.m_y);
        p.AddLineToPoint(tikx, porigin.m_y + tikwidth);
        lbl.Printf(wxT("%li"), pr.second);
        gc->GetTextExtent(lbl, &txw, &txh);
        gc->DrawText(lbl, std::fma(txw, -0.5, tikx), porigin.m_y + tikwidth);
    }
    gc->SetFont(*wxNORMAL_FONT, *wxBLACK);
    lbl = wxT("Depth (mm)");
    gc->GetTextExtent(lbl, &txw, &txh);
    gc->DrawText(lbl, std::fma(txw, -0.5, std::fma(pwidth.m_x, 0.5, porigin.m_x)),
        std::fma(tikwidth, 5.0, porigin.m_y));
    gc->StrokePath(p);
}

void PlotWindow::draw_yaxis(wxGraphicsContext *gc, const wxPoint2DDouble &porigin,
                            const wxPoint2DDouble &pwidth, double tikwidth)
{
    const double tikscale = pwidth.m_y / static_cast<double>(yticks.size() - 1);
    wxGraphicsPath p = gc->CreatePath();
    double txw, txh;
    wxString lbl;
    unsigned i;
    gc->SetFont(this->GetFont().MakeSmaller(), *wxBLACK);
    p.MoveToPoint(porigin);
    p.AddLineToPoint(porigin.m_x, porigin.m_y + pwidth.m_y);
    for (i = 0; i < yticks.size(); i++) {
        const double tiky = std::fma(static_cast<double>(i), tikscale, porigin.m_y);
        p.MoveToPoint(porigin.m_x, tiky);
        p.AddLineToPoint(porigin.m_x - tikwidth, tiky);
        lbl.Printf(wxT("%.2f"), yticks[i]);
        gc->GetTextExtent(lbl, &txw, &txh);
        gc->DrawText(lbl, porigin.m_x - tikwidth - txw, std::fma(txh, -0.5, tiky));
    }
    gc->SetFont(*wxNORMAL_FONT, *wxBLACK);
    lbl = wxT("Dose (Gy)");
    gc->GetTextExtent(lbl, &txw, &txh);
    gc->DrawText(lbl, std::fma(tikwidth, -9.0, porigin.m_x - txh),
        std::fma(txw, 0.5, std::fma(pwidth.m_y, 0.5, porigin.m_y)), M_PI_2);
    gc->StrokePath(p);
}

static void plot_window_compute_plot_area(wxPoint2DDouble &porigin,
                                          wxPoint2DDouble &pwidth,
                                          double &tikwidth)
{
    constexpr double lmarg = 0.1, rmarg = 0.1;
    constexpr double tmarg = 0.1, bmarg = 0.1;
    porigin.m_x = lmarg * pwidth.m_x;
    porigin.m_y = (1.0 - bmarg) * pwidth.m_y;
    pwidth.m_x *= (1.0 - lmarg - rmarg);
    pwidth.m_y *= (1.0 - tmarg - bmarg);
    tikwidth = (pwidth.m_x < pwidth.m_y) ? pwidth.m_x : pwidth.m_y;
    tikwidth *= 0.01;
    pwidth.m_y = -pwidth.m_y;
}

void PlotWindow::draw_plot(wxGraphicsContext *gc)
{
    wxPoint2DDouble porigin, pwidth;
    double tikwidth;

    /* Draw the background and compute the plot area subset */
    gc->GetSize(&pwidth.m_x, &pwidth.m_y);
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->DrawRectangle(0.0, 0.0, pwidth.m_x, pwidth.m_y);
    plot_window_compute_plot_area(porigin, pwidth, tikwidth);

    /* Prepare for drawing axes */
    gc->SetBrush(wxNullBrush);
    gc->SetPen(*wxBLACK_PEN);
    draw_xaxis(gc, porigin, pwidth, tikwidth);
    draw_yaxis(gc, porigin, pwidth, tikwidth);
    draw_dashes(gc, porigin, pwidth);
    draw_line_dose(gc, porigin, pwidth);
    draw_measurements(gc, porigin, pwidth);
}

template <typename HDC>
static wxGraphicsContext *get_graphics_context(const HDC &dc)
{
/* Is it already doing this?
#if _WIN32
    return wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(dc);
#else */
    return wxGraphicsContext::Create(dc);
/* #endif */
}

void PlotWindow::on_evt_paint(wxPaintEvent &WXUNUSED(e))
{
    wxPaintDC dc(canv);
    if (wxGetApp().dose_loaded()) {
        wxGraphicsContext *gc = get_graphics_context(dc);
        draw_plot(gc);
        delete gc;
    }
}

void PlotWindow::on_evt_close(wxCloseEvent &WXUNUSED(e))
{
    this->Show(false);
}

static double plot_window_compute_max_depth()
{
    const double max_actual = std::floor(proton_dose_width(wxGetApp().get_dose(), 1));
    double maxdepth = wxGetApp().get_max_slider_depth();
    maxdepth *= DEPTHMAX_MULT;
    maxdepth = std::clamp<double>(std::floor(maxdepth), 0.0, max_actual);
    return maxdepth;
}

void PlotWindow::write_xaxis()
{
    constexpr std::array<long, 7> tikdivs = { 100, 50, 20, 10, 5, 2, 1 };
    const long maxdepth = static_cast<long>(plot_window_compute_max_depth());
    double tikinc;
    long div = 0, i;
    ldiv_t res;
    for (const long &tdiv : tikdivs) {
        res = ldiv(maxdepth, tdiv);
        if (res.quot > MIN_XTICKS) {
            div = tdiv;
            break;
        }
    }
    if (!div) [[unlikely]] {
        /* Is this even possible? */
        xticks.clear();
        return;
    } else {
        double xtra = static_cast<double>(res.rem) / static_cast<double>(maxdepth);
        tikinc = (1.0 - xtra) / static_cast<double>(res.quot);
    }
    xticks.resize(res.quot + 1);
    for (i = 0; i <= res.quot; i++) {
        xticks[i].first = static_cast<double>(i) * tikinc;
        xticks[i].second = i * div;
    }
    if (res.rem) {
        xticks.push_back(std::pair(1.0, maxdepth));
    }
}

void PlotWindow::write_yaxis()
{
    constexpr long nticks = MIN_YTICKS;
    const double maxdose = DOSEMAX_MULT * wxGetApp().get_max_dose();
    const double doseinc = maxdose / static_cast<double>(nticks - 1);
    long i;
    yticks.resize(nticks);
    for (i = 0; i < nticks; i++) {
        yticks[i] = static_cast<double>(i) * doseinc;
    }
}

void PlotWindow::allocate_line_dose()
{
    const double maxdepth = plot_window_compute_max_depth();
    proton_line_destroy(line);
    line = proton_line_create(wxGetApp().get_dose(), maxdepth);
}

PlotWindow::PlotWindow(wxWindow *parent):
    wxFrame(parent, wxID_ANY, wxT("Plot window"), wxDefaultPosition, wxSize(640, 480)),
    canv(new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)),
    line(nullptr),
    dashpen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT),
    dosepen(wxColour(60, 160, 100), 2, wxPENSTYLE_SOLID),
    measpen(wxColour(60, 60, 190), 1, wxPENSTYLE_SOLID),
    diffpen(wxColour(180, 100, 100), 1, wxPENSTYLE_SOLID)
{
    this->Bind(wxEVT_CLOSE_WINDOW, &PlotWindow::on_evt_close, this);
    canv->Bind(wxEVT_PAINT, &PlotWindow::on_evt_paint, this);

#if _WIN32
    canv->SetDoubleBuffered(true);
#endif
}

PlotWindow::~PlotWindow()
{
    proton_line_destroy(line);
}

void PlotWindow::write_line_dose()
{
    double x, y;
    wxGetApp().get_line_dose(&x, &y);
    proton_dose_get_line(wxGetApp().get_dose(), line, x, y);
    redraw();
}

void PlotWindow::redraw()
{
    canv->Refresh();
}

void PlotWindow::new_dose_loaded()
{
    allocate_line_dose();
    if (line) {
        write_xaxis();
        write_yaxis();
        write_line_dose();
        redraw();
    } else {
        wxGetApp().unload_dose();
    }
}
