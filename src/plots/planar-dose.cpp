#include "../main-window.h"


void PlanarDosePlot::draw_measurements(wxGraphicsContext *gc, struct plot_context *ctx)
{
    const double dosescale = ctx->width.m_y / yticks.back();
    const double depthscale = ctx->width.m_x / static_cast<double>(xticks.back().second);
    gc->SetPen(measpen);
    if (measurements.empty()) {
        /* Draw a line at the currently drawn depth */
        const double depth = wxGetApp().get_depth();
        const double x = std::fma(depth, depthscale, ctx->origin.m_x);
        const double y = std::fma(proton_planes_get_dose(wxGetApp().get_dose(), depth), dosescale, ctx->origin.m_y);
        gc->StrokeLine(x, ctx->origin.m_y, x, y);
    } else {
        const double dcenter = std::fma(0.5, ctx->width.m_y, ctx->origin.m_y);
        const double dscale = 5.0 * ctx->width.m_y;
        for (auto &[depth, _, dose] : measurements) {
            const double x = std::fma(depth, depthscale, ctx->origin.m_x);
            const double y = std::fma(dose, dosescale, ctx->origin.m_y);
            const double mdose = proton_planes_get_dose(wxGetApp().get_dose(), depth);
            gc->DrawRectangle(
                std::fma(-0.5, ctx->boxwidth, x),
                std::fma(-0.5, ctx->boxwidth, y),
                ctx->boxwidth, ctx->boxwidth);
            dose = (dose - mdose) / mdose;
            depth = x;
        }
        gc->SetPen(diffpen);
        for (const auto &[x, _, dose] : measurements) {
            const double y = std::fma(dose, dscale, dcenter);/* ctx->origin.m_y + 0.5 * ctx->width.m_y + 5 * ctx->width.m_y * meas.second; */
            gc->DrawEllipse(
                std::fma(-0.5, ctx->boxwidth, x),
                std::fma(-0.5, ctx->boxwidth, y),
                ctx->boxwidth, ctx->boxwidth);
        }
    }
}

void PlanarDosePlot::draw_line_dose(wxGraphicsContext *gc, const struct plot_context *ctx)
{
    const double dosescale = ctx->width.m_y / yticks.back();
    const double depthscale = ctx->width.m_x * proton_dose_spacing(wxGetApp().get_dose(), 1) / static_cast<double>(xticks.back().second);
    const double depthorig = std::fma(depthscale, 0.5, ctx->origin.m_x);
    const float *ld;
    wxGraphicsPath p = gc->CreatePath();
    long i, n;
    ld = proton_planes_raw(wxGetApp().get_dose(), &n);
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

void PlanarDosePlot::draw_dashes(wxGraphicsContext *gc, const struct plot_context *ctx)
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

void PlanarDosePlot::draw_xaxis(wxGraphicsContext *gc, const struct plot_context *ctx)
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

void PlanarDosePlot::draw_yaxis(wxGraphicsContext *gc, const struct plot_context *ctx)
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

void PlanarDosePlot::initialize_plot_context(wxGraphicsContext *gc, struct plot_context *ctx)
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

    ctx->tikwidth = std::min(ctx->width.m_x, ctx->width.m_y);
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

    gc->SetFont(ctx->axfont, *wxBLACK);
    gc->GetTextExtent(ylabel, &txw, &txh);
    ctx->origin.m_x = txh + ctx->maxywidth + ctx->tikwidth;
    gc->GetTextExtent(xlabel, &txw, &txh);
    ctx->origin.m_y = ctx->width.m_y - (txh + ctx->maxxheight + ctx->tikwidth);
    ctx->width.m_x -= ctx->maxpwidth + ctx->tikwidth + ctx->origin.m_x;
    ctx->width.m_y = std::fma(tmargin, ctx->width.m_y, -ctx->origin.m_y);

    ctx->bright = ctx->tleft = ctx->origin;
    ctx->bright.m_x += ctx->width.m_x;
    ctx->tleft.m_y += ctx->width.m_y;
    ctx->tright = ctx->tleft;
    ctx->tright.m_x += ctx->width.m_x;

    ctx->ytikscale = ctx->width.m_y / static_cast<double>(yticks.size() - 1);
    ctx->boxwidth = pmean * 0.01;
}

void PlanarDosePlot::draw_plot(wxGraphicsContext *gc)
{
    struct plot_context ctx;

    gc->GetSize(&ctx.width.m_x, &ctx.width.m_y);
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->DrawRectangle(0.0, 0.0, ctx.width.m_x, ctx.width.m_y);
    wxGetApp().get_measurements(measurements);
    initialize_plot_context(gc, &ctx);

    gc->SetBrush(wxNullBrush);
    gc->SetPen(*wxBLACK_PEN);
    draw_xaxis(gc, &ctx);
    draw_yaxis(gc, &ctx);

    gc->Clip(ctx.origin.m_x, ctx.origin.m_y, ctx.width.m_x, ctx.width.m_y);
    draw_dashes(gc, &ctx);
    draw_line_dose(gc, &ctx);
    draw_measurements(gc, &ctx);
}

void PlanarDosePlot::write_xaxis()
{
    write_depth_axis();
}

void PlanarDosePlot::write_yaxis()
{
    write_dose_axis(PLANEMAX_MULT * proton_planes_max(wxGetApp().get_dose()));
}

PlanarDosePlot::PlanarDosePlot(wxWindow *parent):
    ProtonPlot(parent, DEPTH_AXLABEL, wxString::FromUTF8(PLANE_AXLABEL))
{

}
