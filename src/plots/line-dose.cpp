#include <algorithm>
#include <array>
#include "../main-window.h"
#include <cmath>

#ifndef M_PI_2 /* Since nobody but glibc wants to #def this */
#   define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif

#define DOSEMAX_MULT    1.0
#define DEPTHMAX_MULT   1.1

#define GRID_OPACITY        0.2
#define LINEDOSE_OPACITY    0.8

#define MIN_XTICKS      9
#define MIN_YTICKS      9

#define IMAGE_AXFONT_SIZE  20
#define IMAGE_TIKFONT_SIZE 14


static const wxString xlabel = wxT("Depth (mm)");
static const wxString ylabel = wxT("Dose (Gy)");
static const wxString plabel = wxT("% Dose difference");

static const std::array<wxString, 4> plbls = {
    wxT("-3"), wxT("-2"), wxT(" 2"), wxT(" 3") };


void LineDosePlot::draw_legend(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    constexpr double toffset = 3.0;
    static const wxString tlbl = wxT("TPS dose");
    static const wxString mlbl = wxT("Measured dose");
    static const wxString dlbl = wxT("% Difference");
    const double textorg = std::fma(toffset, ctx->boxwidth, ctx->origin.m_x);
    const double symorg = std::fma(toffset / 2.0, ctx->boxwidth, ctx->origin.m_x);
    double lh, mh, dh, legendy, width;

    gc->SetFont(ctx->axfont, *wxBLACK);

    gc->GetTextExtent(tlbl, &width, &lh);
    gc->GetTextExtent(mlbl, &width, &mh);
    gc->GetTextExtent(dlbl, &width, &dh);

    legendy = ctx->origin.m_y - dh;
    width = 0.3 * dh;
    gc->SetPen(diffpen);
    gc->DrawEllipse(
        std::fma(-0.5, width, symorg),
        std::fma( 0.5, width, legendy),
        width, width);
    gc->DrawText(dlbl, textorg, legendy);

    legendy -= mh;
    width = 0.3 * mh;
    gc->SetPen(measpen);
    gc->DrawRectangle(
        std::fma(-0.5, width, symorg),
        std::fma( 0.5, width, legendy),
        width, width);
    gc->DrawText(mlbl, textorg, legendy);

    legendy -= dh;
    width = dh;
    gc->SetPen(dosepen);
    gc->StrokeLine(
        std::fma(-0.5, width, symorg), std::fma(0.5, width, legendy),
        std::fma( 0.5, width, symorg), std::fma(0.5, width, legendy));
    gc->DrawText(tlbl, textorg, legendy);
}

void LineDosePlot::draw_measurements(wxGraphicsContext *gc, struct plot_context *ctx)
{
    const double dosescale = ctx->width.m_y / yticks.back();
    const double depthscale = ctx->width.m_x / static_cast<double>(xticks.back().second);
    gc->SetPen(measpen);
    if (ctx->measurements.empty()) {
        /* Draw a line at the currently drawn depth */
        const double depth = wxGetApp().get_depth();
        const double x = std::fma(depth, depthscale, ctx->origin.m_x);
        const double y = std::fma(proton_line_get_dose(wxGetApp().get_dose(), depth), dosescale, ctx->origin.m_y);
        gc->StrokeLine(x, ctx->origin.m_y, x, y);
    } else {
        const double dcenter = std::fma(0.5, ctx->width.m_y, ctx->origin.m_y);
        const double dscale = 5.0 * ctx->width.m_y;
        for (std::pair<double, double> &meas : ctx->measurements) {
            const double x = std::fma(meas.first, depthscale, ctx->origin.m_x);
            const double y = std::fma(meas.second, dosescale, ctx->origin.m_y);
            const double mdose = proton_line_get_dose(wxGetApp().get_dose(), meas.first);
            gc->DrawRectangle(
                std::fma(-0.5, ctx->boxwidth, x),
                std::fma(-0.5, ctx->boxwidth, y),
                ctx->boxwidth, ctx->boxwidth);
            meas.second = (meas.second - mdose) / mdose;
            meas.first = x;
        }
        gc->SetPen(diffpen);
        for (const std::pair<double, double> &meas : ctx->measurements) {
            const double x = meas.first;
            const double y = std::fma(meas.second, dscale, dcenter);/* ctx->origin.m_y + 0.5 * ctx->width.m_y + 5 * ctx->width.m_y * meas.second; */
            gc->DrawEllipse(
                std::fma(-0.5, ctx->boxwidth, x),
                std::fma(-0.5, ctx->boxwidth, y),
                ctx->boxwidth, ctx->boxwidth);
        }
    }
}

void LineDosePlot::draw_line_dose(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    const double dosescale = ctx->width.m_y / yticks.back();
    const double depthscale = ctx->width.m_x * proton_dose_spacing(wxGetApp().get_dose(), 1) / static_cast<double>(xticks.back().second);
    const double depthorig = std::fma(depthscale, 0.5, ctx->origin.m_x);
    const float *ld = proton_line_raw(wxGetApp().get_dose());
    wxGraphicsPath p = gc->CreatePath();
    long i;
    gc->SetPen(dosepen);
    p.MoveToPoint(depthorig, std::fma(static_cast<double>(*ld), dosescale, ctx->origin.m_y));
    ld++;
    for (i = 1; i < proton_line_length(wxGetApp().get_dose()); i++, ld++) {
        const double dose = static_cast<double>(*ld);
        p.AddLineToPoint(
            std::fma(static_cast<double>(i), depthscale, depthorig),
            std::fma(dose, dosescale, ctx->origin.m_y));
    }
    gc->StrokePath(p);
}

void LineDosePlot::draw_dashes(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    wxGraphicsPath p = gc->CreatePath();
    unsigned i;
    /* gc->BeginLayer(GRID_OPACITY); */
    gc->SetPen(dashpen);
    for (i = 1; i < xticks.size(); i++) {
        const double tikx = std::fma(xticks[i].first, ctx->width.m_x, ctx->origin.m_x);
        p.MoveToPoint(tikx, ctx->origin.m_y);
        p.AddLineToPoint(tikx, ctx->tleft.m_y);
    }
    for (i = 1; i < yticks.size(); i++) {
        const double tiky = std::fma(static_cast<double>(i), ctx->ytikscale, ctx->origin.m_y);
        p.MoveToPoint(ctx->origin.m_x, tiky);
        p.AddLineToPoint(ctx->bright.m_x, tiky);
    }
    gc->StrokePath(p);
    /* gc->EndLayer(); */
}

void LineDosePlot::draw_xaxis(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    const wxString &axlbl = xlabel;
    const double tikend = ctx->origin.m_y + ctx->tikwidth;
    wxGraphicsPath p = gc->CreatePath();
    double txw, txh;
    unsigned i;
    gc->SetFont(ctx->tikfont, *wxBLACK);
    p.MoveToPoint(ctx->origin);
    p.AddLineToPoint(ctx->bright);
    for (i = 0; i < (xticks.size() - 1); i++) {
        auto &pr = xticks[i];
        const double tikx = std::fma(pr.first, ctx->width.m_x, ctx->origin.m_x);
        p.MoveToPoint(tikx, ctx->origin.m_y);
        p.AddLineToPoint(tikx, tikend);
        gc->GetTextExtent(xticklabels[i], &txw, &txh);
        gc->DrawText(xticklabels[i], std::fma(txw, -0.5, tikx), tikend);
    }
    gc->SetFont(ctx->axfont, *wxBLACK);
    gc->GetTextExtent(axlbl, &txw, &txh);
    gc->DrawText(axlbl,
        std::fma(txw, -0.5, std::fma(ctx->width.m_x, 0.5, ctx->origin.m_x)),
        ctx->origin.m_y + ctx->tikwidth + ctx->maxxheight);
    gc->StrokePath(p);
}

void LineDosePlot::draw_yaxis(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    const wxString &axlbl = ylabel;
    const double tikend = ctx->origin.m_x - ctx->tikwidth;
    wxGraphicsPath p = gc->CreatePath();
    double txw, txh;
    unsigned i;
    gc->SetFont(ctx->tikfont, *wxBLACK);
    p.MoveToPoint(ctx->origin);
    p.AddLineToPoint(ctx->tleft);
    for (i = 0; i < yticks.size(); i++) {
        const double tiky = std::fma(static_cast<double>(i), ctx->ytikscale, ctx->origin.m_y);
        p.MoveToPoint(ctx->origin.m_x, tiky);
        p.AddLineToPoint(tikend, tiky);
        gc->GetTextExtent(yticklabels[i], &txw, &txh);
        gc->DrawText(yticklabels[i], tikend - txw, std::fma(txh, -0.5, tiky));
    }
    gc->SetFont(ctx->axfont, *wxBLACK);
    gc->GetTextExtent(axlbl, &txw, &txh);
    gc->DrawText(axlbl,
        ctx->origin.m_x - ctx->tikwidth - ctx->maxywidth - txh,/* std::fma(ctx->tikwidth, -7.0, ctx->origin.m_x - txh), */
        std::fma(txw, 0.5, std::fma(ctx->width.m_y, 0.5, ctx->origin.m_y)), M_PI_2);
    gc->StrokePath(p);
}

void LineDosePlot::draw_paxis(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    const wxString &axlbl = plabel;
    const double c = std::fma(0.5, ctx->width.m_y, ctx->origin.m_y);
    const double t = std::fma(1.0, ctx->tikwidth, ctx->bright.m_x);
    const double heights[] = {
        std::fma(-0.15, ctx->width.m_y, c),
        std::fma(-0.10, ctx->width.m_y, c),
        std::fma( 0.10, ctx->width.m_y, c),
        std::fma( 0.15, ctx->width.m_y, c)
    };
    wxGraphicsPath p = gc->CreatePath();
    double txw, txh;
    gc->PushState();

    gc->SetPen(diffpen);
    p.MoveToPoint(ctx->bright);
    p.AddLineToPoint(ctx->tright);
    p.MoveToPoint(ctx->origin.m_x, heights[1]);
    p.AddLineToPoint(ctx->bright.m_x, heights[1]);
    p.MoveToPoint(ctx->origin.m_x, heights[2]);
    p.AddLineToPoint(ctx->bright.m_x, heights[2]);
    gc->StrokePath(p);

    gc->SetPen(diffpendashed);
    p = gc->CreatePath();
    p.MoveToPoint(ctx->origin.m_x, heights[0]);
    p.AddLineToPoint(ctx->bright.m_x, heights[0]);
    p.MoveToPoint(ctx->origin.m_x, heights[3]);
    p.AddLineToPoint(ctx->bright.m_x, heights[3]);
    gc->StrokePath(p);

    gc->SetFont(ctx->tikfont, diffcolor);
    gc->GetTextExtent(plbls[0], &txw, &txh);
    gc->DrawText(plbls[0], t, std::fma(-0.5, txh, heights[0]));
    gc->GetTextExtent(plbls[1], &txw, &txh);
    gc->DrawText(plbls[1], t, std::fma(-0.5, txh, heights[1]));
    gc->GetTextExtent(plbls[2], &txw, &txh);
    gc->DrawText(plbls[2], t, std::fma(-0.5, txh, heights[2]));
    gc->GetTextExtent(plbls[3], &txw, &txh);
    gc->DrawText(plbls[3], t, std::fma(-0.5, txh, heights[3]));

    gc->SetFont(ctx->axfont, diffcolor);
    gc->GetTextExtent(axlbl, &txw, &txh);
    gc->DrawText(axlbl,
        ctx->bright.m_x + ctx->tikwidth + ctx->maxpwidth,/* std::fma(6.0, ctx->tikwidth, ctx->bright.m_x), */
        std::fma(0.5, txw, c), M_PI_2);
    gc->PopState();
}

void LineDosePlot::initialize_plot_context(wxGraphicsContext *gc, struct plot_context *ctx)
{
    constexpr double tmargin = 0.05;
    const double pmean = std::sqrt(ctx->width.m_x * ctx->width.m_y);
    double txw, txh;

    {
        const double pscale = pmean / image_gmean;
        ctx->tikfont = wxFont(std::floor(IMAGE_TIKFONT_SIZE * pscale),
            wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        ctx->axfont = wxFont(std::floor(IMAGE_AXFONT_SIZE * pscale),
            wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    }

    ctx->tikwidth = (ctx->width.m_x < ctx->width.m_y) ? ctx->width.m_x : ctx->width.m_y;
    ctx->tikwidth *= 0.01;

    ctx->maxxheight = ctx->maxywidth = ctx->maxpwidth = 0.0;
    gc->SetFont(ctx->tikfont, *wxBLACK);
    for (const wxString &xtl : xticklabels) {
        gc->GetTextExtent(xtl, &txw, &txh);
        ctx->maxxheight = std::max(ctx->maxxheight, txh);
    }
    for (const wxString &ytl : yticklabels) {
        gc->GetTextExtent(ytl, &txw, &txh);
        ctx->maxywidth = std::max(ctx->maxywidth, txw);
    }
    for (const wxString &ptl : plbls) {
        gc->GetTextExtent(ptl, &txw, &txh);
        ctx->maxpwidth = std::max(ctx->maxpwidth, txw);
    }

    gc->SetFont(ctx->axfont, *wxBLACK);
    gc->GetTextExtent(ylabel, &txw, &txh);
    ctx->origin.m_x = txh + ctx->maxywidth + ctx->tikwidth;
    gc->GetTextExtent(xlabel, &txw, &txh);
    ctx->origin.m_y = ctx->width.m_y - (txh + ctx->maxxheight + ctx->tikwidth);
    gc->GetTextExtent(plabel, &txw, &txh);
    ctx->width.m_x -= txh + ctx->maxpwidth + ctx->tikwidth + ctx->origin.m_x;
    ctx->width.m_y = std::fma(tmargin, ctx->width.m_y, -ctx->origin.m_y);

    ctx->bright = ctx->tleft = ctx->origin;
    ctx->bright.m_x += ctx->width.m_x;
    ctx->tleft.m_y += ctx->width.m_y;
    ctx->tright = ctx->tleft;
    ctx->tright.m_x += ctx->width.m_x;

    ctx->ytikscale = ctx->width.m_y / static_cast<double>(yticks.size() - 1);
    ctx->boxwidth = pmean * 0.01;
    wxGetApp().get_measurements(ctx->measurements);
}

void LineDosePlot::draw_plot(wxGraphicsContext *gc)
{
    struct plot_context ctx;

    /* Draw the background and compute the plot area subset */
    gc->GetSize(&ctx.width.m_x, &ctx.width.m_y);
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->DrawRectangle(0.0, 0.0, ctx.width.m_x, ctx.width.m_y);
    initialize_plot_context(gc, &ctx);

    /* Prepare for drawing axes */
    gc->SetBrush(wxNullBrush);
    gc->SetPen(*wxBLACK_PEN);
    draw_xaxis(gc, &ctx);
    draw_yaxis(gc, &ctx);
    if (!ctx.measurements.empty()) {
        draw_paxis(gc, &ctx);
    }
    gc->Clip(ctx.origin.m_x, ctx.origin.m_y, ctx.width.m_x, ctx.width.m_y);
    draw_dashes(gc, &ctx);
    draw_line_dose(gc, &ctx);
    draw_measurements(gc, &ctx);
    if (!ctx.measurements.empty()) {
        draw_legend(gc, &ctx);
    }
}

void LineDosePlot::write_xaxis()
{
    constexpr std::array<long, 7> tikdivs = { 100, 50, 20, 10, 5, 2, 1 };
    const long maxdepth = static_cast<long>(wxGetApp().get_max_slider_depth());
    double tikinc;
    long div = 0, i;
    ldiv_t res;
    for (const long tdiv : tikdivs) {
        res = ldiv(maxdepth, tdiv);
        if (res.quot > MIN_XTICKS) {
            div = tdiv;
            break;
        }
    }
    if (div) {
        const double xtra = static_cast<double>(res.rem) / static_cast<double>(maxdepth);
        tikinc = (1.0 - xtra) / static_cast<double>(res.quot);
    } else [[unlikely]] {
        /* Is this even possible? */
        xticks.clear();
        return;
    }
    xticks.resize(res.quot + 1);
    xticklabels.resize(res.quot + 1);
    for (i = 0; i <= res.quot; i++) {
        xticks[i].first = static_cast<double>(i) * tikinc;
        xticks[i].second = i * div;
        xticklabels[i].Printf(wxT("%li"), xticks[i].second);
    }
    if (res.rem) {
        xticks.push_back(std::pair(1.0, maxdepth));
    }
}

void LineDosePlot::write_yaxis()
{
    constexpr long nticks = MIN_YTICKS;
    const double maxdose = DOSEMAX_MULT * wxGetApp().get_max_dose();
    const double doseinc = maxdose / static_cast<double>(nticks - 1);
    long i;
    yticks.resize(nticks);
    yticklabels.resize(nticks);
    for (i = 0; i < nticks; i++) {
        yticks[i] = static_cast<double>(i) * doseinc;
        yticklabels[i].Printf(wxT("%.2f"), yticks[i]);
    }
}

LineDosePlot::LineDosePlot(wxWindow *parent):
    ProtonPlot(parent),
    dosecolor(60, 160, 100),
    meascolor(60, 60, 190),
    diffcolor(200, 50, 20),
    dashpen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT),
    dosepen(dosecolor, 2, wxPENSTYLE_SOLID),
    measpen(meascolor, 1, wxPENSTYLE_SOLID),
    diffpen(diffcolor, 1, wxPENSTYLE_SOLID),
    diffpendashed(diffcolor, 1, wxPENSTYLE_SHORT_DASH)
{
    
}

void LineDosePlot::write_axes()
{
    write_xaxis();
    write_yaxis();
}