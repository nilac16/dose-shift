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

void ProtonPlot::write_depth_axis()
{
    constexpr std::array<long, 7> tikdivs = { 100, 50, 20, 10, 5, 2, 1 };
    const long maxdepth = static_cast<long>(wxGetApp().get_max_slider_depth());
    long div = 0, i;
    double tikinc;
    std::ldiv_t res;
    for (const long tdiv : tikdivs) {
        res = std::ldiv(maxdepth, tdiv);
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
        xticks[i] = { static_cast<double>(i) * tikinc, i * div };
        xticklabels[i].Printf(wxT("%li"), xticks[i].second);
    }
    if (res.rem) {
        xticks.push_back({1.0, maxdepth});
    }
}

void ProtonPlot::write_dose_axis(const double limit, const wxString &fmt)
{
    constexpr long nticks = MIN_YTICKS;
    const double doseinc = limit / static_cast<double>(nticks - 1);
    long i;
    yticks.resize(nticks);
    yticklabels.resize(nticks);
    for (i = 0; i < nticks; i++) {
        yticks[i] = static_cast<double>(i) * doseinc;
        yticklabels[i].Printf(fmt, yticks[i]);
    }
}

ProtonPlot::ProtonPlot(wxWindow *parent, const wxString &xlabel, const wxString &ylabel, const wxString &plabel):
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    dosecolor(60, 160, 100),
    meascolor(60, 60, 190),
    diffcolor(200, 50, 20),
    dashpen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT),
    dosepen(dosecolor, 2, wxPENSTYLE_SOLID),
    measpen(meascolor, 1, wxPENSTYLE_SOLID),
    diffpen(diffcolor, 1, wxPENSTYLE_SOLID),
    diffpendashed(diffcolor, 1, wxPENSTYLE_SHORT_DASH),
    xlabel(xlabel), ylabel(ylabel), plabel(plabel)
{
    this->Bind(wxEVT_PAINT, &ProtonPlot::on_evt_paint, this);

#if _WIN32
    this->SetDoubleBuffered(true);
#endif
}

void ProtonPlot::write_axes()
{
    write_xaxis();
    write_yaxis();
}
