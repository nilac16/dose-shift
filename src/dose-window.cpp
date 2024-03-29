#include "main-window.h"
#include <wx/graphics.h>
#include "proton-aux.h"


bool DoseWindow::DoseDragNDrop::OnDropFiles(wxCoord              WXUNUSED(x),
                                            wxCoord              WXUNUSED(y),
                                            const wxArrayString &filenames)
{
    if (filenames.size() != 1) {
        return false;
    } else {
        wxGetApp().dropped_file(filenames[0]);
        return true;
    }
}


void DoseWindow::paint_detector(wxPaintDC &dc)
{
    constexpr double xhair = 5.0;
    constexpr double oct = 260.0;
    /* const double conv[2] = {
        static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
        static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
    }; */
    wxGraphicsContext *gc;
    
    gc = wxGraphicsContext::Create(dc);
    gc->Scale(conv[0], conv[1]);
    gc->Translate(-proton_dose_origin(dose, 0), -proton_dose_origin(dose, 2));
    gc->SetPen(*wxBLACK_PEN);
    gc->PushState();
    gc->ConcatTransform(gc->CreateMatrix(
        affine[0], affine[1], affine[2], affine[3], affine[4], affine[5]));
    gc->DrawRectangle(-oct / 2.0, -oct / 2.0, oct, oct);
    /* Don't draw the center of the detector, use a crosshair cursor instead
    {
        wxGraphicsPath p = gc->CreatePath();
        p.MoveToPoint(5.0, 1.0);
        p.AddLineToPoint(1.0, 5.0);
        p.MoveToPoint(-1.0, 5.0);
        p.AddLineToPoint(-5.0, 1.0);
        p.MoveToPoint(-5.0, -1.0);
        p.AddLineToPoint(-1.0, -5.0);
        p.MoveToPoint(1.0, -5.0);
        p.AddLineToPoint(5.0, -1.0);
        gc->StrokePath(p);
    } */
    gc->PopState();
    {
        wxGraphicsPath p;
        double ldx, ldy;

        p = gc->CreatePath();
        wxGetApp().get_line_dose(&ldx, &ldy);
        gc->Translate(ldx, ldy);
        p.MoveToPoint(0.0, -xhair);
        p.AddLineToPoint(0.0, xhair);
        p.MoveToPoint(-xhair, 0.0);
        p.AddLineToPoint(xhair, 0.0);
        p.AddCircle(0.0, 0.0, xhair / 2.00);
        gc->StrokePath(p);
    }
    delete gc;
}


void DoseWindow::paint_bitmap(wxPaintDC &dc)
{
    wxSize psz;

    psz = wxSize(proton_image_dimension(img, 0),
                 proton_image_dimension(img, 1));
    dc.SetClippingRegion(origin, psz);
    dc.SetDeviceOrigin(origin.x, origin.y);
    dc.DrawBitmap(wxImage(psz, proton_image_raw(img), true), 0, 0);
}


void DoseWindow::on_paint(wxPaintEvent &WXUNUSED(e))
{
    wxPaintDC dc(this);

    if (dose_loaded() && !proton_image_empty(img)) {
        paint_bitmap(dc);
        if (wxGetApp().detector_enabled()) {
            paint_detector(dc);
        }
    }
}


void DoseWindow::on_size(wxSizeEvent &e)
{
    if (dose_loaded()) {
        image_realloc_and_write(e.GetSize());
        affine_write();
        conv_write();
    }
}


void DoseWindow::on_lmb(wxMouseEvent &e)
{
    double x, y;
    wxPoint p;

    if (dose_loaded()) {
        p = e.GetPosition();
        if (point_in_dose(p)) {
            /* const double conv[2] = {
                static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
                static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
            }; */
            p.x -= origin.x;
            p.y -= origin.y;
            x = static_cast<double>(p.x) / conv[0] + proton_dose_origin(dose, 0);
            y = static_cast<double>(p.y) / conv[1] + proton_dose_origin(dose, 2);
            wxGetApp().set_translation(x, y);
        }
    }
    e.Skip();
}


void DoseWindow::on_rmb(wxMouseEvent &e)
{
    double x, y;
    wxPoint p;

    if (dose_loaded()) {
        p = e.GetPosition();
        if (point_in_dose(p)) {
            /* const double conv[2] = {
                static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
                static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
            }; */
            p.x -= origin.x;
            p.y -= origin.y;
            x = static_cast<double>(p.x) / conv[0] + proton_dose_origin(dose, 0);
            y = static_cast<double>(p.y) / conv[1] + proton_dose_origin(dose, 2);
            wxGetApp().set_line_dose(x, y);
        }
    }
    e.Skip();
}


void DoseWindow::on_motion(wxMouseEvent &e)
{
    if (e.LeftIsDown()) {
        on_lmb(e);
    } else if (e.RightIsDown()) {
        on_rmb(e);
    } else {
        e.Skip();
    }
}


void DoseWindow::conv_write()
{
    conv[0] = static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0);
    conv[1] = static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2);
}


void DoseWindow::affine_write()
{
    wxGetApp().get_detector_affine(affine);
}


void DoseWindow::image_write()
{
    float depth;
    
    depth = wxGetApp().get_depth();
    proton_dose_get_plane(dose, &wxGetApp().visuals(), img, depth);
}


void DoseWindow::image_realloc_and_write(const wxSize &csz)
{
    const int W = csz.GetWidth(), H = csz.GetHeight();
    const float clientratio = static_cast<float>(W) / static_cast<float>(H);
    const float imgratio = proton_dose_coronal_aspect(dose);
    int w, h;

    if (clientratio < imgratio) {
        w = W;
        h = static_cast<int>(static_cast<float>(W) / imgratio);
        origin.x = 0;
        origin.y = (H - h) / 2;
    } else {
        w = static_cast<int>(static_cast<float>(H) * imgratio);
        h = H;
        origin.x = (W - w) / 2;
        origin.y = 0;
    }
    if (proton_image_realloc(&img, w, h)) {
        unload_dose();
        wxMessageBox(wxT("Failed to reallocate image buffer\n"\
            "The dose has been unloaded"),
            wxT("Realloc failed"), wxICON_ERROR);
    } else {
        image_write();
    }
}


bool DoseWindow::point_in_dose(const wxPoint &p)
{
    const wxRect clip(origin, wxSize(proton_image_dimension(img, 0),
        proton_image_dimension(img, 1)));

    return clip.Contains(p);
}


void DoseWindow::write_line_dose()
    noexcept
{
    double x, y;

    wxGetApp().get_line_dose(&x, &y);
    proton_dose_get_line(dose, x, y);
}


DoseWindow::DoseWindow(wxWindow *parent):
    wxWindow(parent,
             wxID_ANY,
             wxDefaultPosition,
             wxDefaultSize,
             wxFULL_REPAINT_ON_RESIZE),
    dose(nullptr),
    img(nullptr),
    droptarget(new DoseDragNDrop)
{
    this->SetCursor(*wxCROSS_CURSOR);

    this->Bind(wxEVT_PAINT, &DoseWindow::on_paint, this);
    this->Bind(wxEVT_SIZE, &DoseWindow::on_size, this);
    this->Bind(wxEVT_LEFT_DOWN, &DoseWindow::on_lmb, this);
    this->Bind(wxEVT_RIGHT_DOWN, &DoseWindow::on_rmb, this);
    this->Bind(wxEVT_MOTION, &DoseWindow::on_motion, this);

#if _WIN32
    this->SetDoubleBuffered(true);
#endif

    this->SetDropTarget(droptarget);
}


DoseWindow::~DoseWindow()
{
    proton_image_destroy(img);
    proton_dose_destroy(dose);
}


void DoseWindow::load_file(const char *filename)
{
    char err[1024] = { 0 };

    proton_dose_destroy(dose);
    dose = proton_dose_create(filename, sizeof err, err);
    if (dose) {
        wxGetApp().set_depth_range();
        image_realloc_and_write(this->GetSize());
        affine_write();
        conv_write();
        write_line_dose();
    } else {
        wxMessageBox(wxString(err), wxT("Load failed"), wxICON_ERROR, this);
    }
    this->Refresh();
}


void DoseWindow::on_depth_changed(wxCommandEvent &WXUNUSED(e))
{
    image_write();
    this->Refresh();
}


void DoseWindow::on_plot_changed(wxCommandEvent &WXUNUSED(e))
{
    this->Refresh();
    /** HI:
     *  Consider whether or not we actually want to do this while this plot 
     *  is not selected (none of the tested machines had any trouble with 
     *  this) */
    write_line_dose();
}


void DoseWindow::on_shift_changed(wxCommandEvent &WXUNUSED(e))
{
    affine_write();
    this->Refresh();
}


void DoseWindow::unload_dose()
    noexcept
{
    proton_dose_destroy(dose);
    dose = nullptr;
}
