#include "main-window.h"
#include <wx/graphics.h>
#include "proton-aux.h"


void DoseWindow::paint_detector(wxPaintDC &dc)
{
    constexpr double oct = 260.0;
    const double conv[2] = {
        static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
        static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
    };
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    gc->Scale(conv[0], conv[1]);
    gc->Translate(-proton_dose_origin(dose, 0), -proton_dose_origin(dose, 2));
    gc->SetPen(*wxBLACK_PEN);
    gc->PushState();
    gc->ConcatTransform(gc->CreateMatrix(
        affine[0], affine[1], affine[2], affine[3], affine[4], affine[5]));
    gc->DrawRectangle(-oct / 2.0, -oct / 2.0, oct, oct);
    /* {
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
    delete gc;
}

void DoseWindow::paint_bitmap(wxPaintDC &dc)
{
    const wxSize psz(proton_image_dimension(img, 0),
                     proton_image_dimension(img, 1));
    wxImage pimg(psz, proton_image_raw(img), true);
    if (!pimg.IsOk()) {
        return;
    }
    dc.SetDeviceOrigin(origin.x, origin.y);
    dc.SetClippingRegion(wxPoint(0, 0), psz);
    dc.DrawBitmap(pimg, 0, 0);
}

void DoseWindow::on_paint(wxPaintEvent &WXUNUSED(e))
{
    wxPaintDC dc(this);
    if (dose_loaded()) {
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
    }
}

void DoseWindow::on_lmb(wxMouseEvent &e)
{
    if (dose_loaded()) {
        wxPoint p = e.GetPosition();
        if (point_in_dose(p)) {
            double x, y;
            const double conv[2] = {
                static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
                static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
            };
            p.x -= origin.x;
            p.y -= origin.y;
            x = static_cast<double>(p.x) / conv[0] + proton_dose_origin(dose, 0);
            y = static_cast<double>(p.y) / conv[1] + proton_dose_origin(dose, 2);
            wxGetApp().set_translation(x, y);
        }
    }
    e.Skip();
}

void DoseWindow::on_motion(wxMouseEvent &e)
{
    if (e.LeftIsDown()) {
        on_lmb(e);
    } else {
        e.Skip();
    }
}

void DoseWindow::affine_write()
{
    /* As it turns out, composing affine transformations by matrix 
    multiplication does not have the expected effect on the state of a 
    wxGraphicsContext. Unless I can determine a cause/fix (including 
    potential programmer error), this function and the class member it
    writes to are largely  obsolete.

    const double conv[2] = {
        static_cast<double>(proton_image_dimension(img, 0)) / proton_dose_width(dose, 0),
        static_cast<double>(proton_image_dimension(img, 1)) / proton_dose_width(dose, 2)
    };
    double tmp[6];
    affine[0] = conv[0];
    affine[3] = conv[1];
    affine[1] = affine[2] = 0.0;
    affine[4] = static_cast<double>(origin.x) - proton_dose_origin(dose, 0) * conv[0];
    affine[5] = static_cast<double>(origin.y) - proton_dose_origin(dose, 2) * conv[1]; */
    wxGetApp().get_detector_affine(affine);
    /* proton_affine_matmul(tmp, affine); */
}

void DoseWindow::image_write()
{
    float depth = wxGetApp().get_depth();
    proton_dose_get_plane(dose, img, depth, proton_colormap);
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
        proton_dose_destroy(dose);
        dose = nullptr;
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

DoseWindow::DoseWindow(wxWindow *parent):
    wxWindow(parent, wxID_ANY),
    dose(nullptr),
    img(nullptr)
{
    this->SetCursor(*wxCROSS_CURSOR);

    this->Bind(wxEVT_PAINT, &DoseWindow::on_paint, this);
    this->Bind(wxEVT_SIZE, &DoseWindow::on_size, this);
    this->Bind(wxEVT_LEFT_DOWN, &DoseWindow::on_lmb, this);
    this->Bind(wxEVT_MOTION, &DoseWindow::on_motion, this);

#if _WIN32
    this->SetDoubleBuffered(true);
#endif
}

DoseWindow::~DoseWindow()
{
    proton_image_destroy(img);
    proton_dose_destroy(dose);
}

void DoseWindow::load_file(const char *filename)
{
    proton_dose_destroy(dose);
    dose = proton_dose_create(filename);
    if (dose) {
        image_realloc_and_write(this->GetSize());
        affine_write();
    } else {
        wxMessageBox(wxT("Failed to load dose"),
            wxT("Load failed"), wxICON_ERROR, this);
    }
    this->Refresh();
}

void DoseWindow::on_depth_changed(wxCommandEvent &WXUNUSED(e))
{
    if (dose_loaded()) {
        image_write();
        this->Refresh();
    }
}

void DoseWindow::on_shift_changed(wxCommandEvent &WXUNUSED(e))
{
    if (dose_loaded()) {
        affine_write();
        this->Refresh();
    }
}

float DoseWindow::get_max_depth() const noexcept
{
    return proton_dose_max_depth(dose);
}
