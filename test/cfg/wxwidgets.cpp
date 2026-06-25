
// Test library configuration for wxwidgets.cfg
//
// Usage:
// $ ./cppcheck --check-library --library=wxwidgets --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/wxwidgets.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout
// cppcheck-suppress-file purgedConfiguration

#include <wx/wx.h>
#include <wx/accel.h>
#include <wx/any.h>
#include <wx/app.h>
#include <wx/archive.h>
#include <wx/artprov.h>
#include <wx/bitmap.h>
#if wxCHECK_VERSION(3, 1, 6)  // wxWidets-3.1.6 or higher
#include <wx/bmpbndl.h>
#endif
#include <wx/brush.h>
#include <wx/calctrl.h>
#include <wx/colour.h>
#include <wx/combo.h>
#include <wx/cursor.h>
#include <wx/dc.h>
#include <wx/dataview.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/dynarray.h>
#include <wx/filefn.h>
#include <wx/font.h>
#include <wx/fontenum.h>
#include <wx/fontutil.h>
#include <wx/frame.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/graphics.h>
#include <wx/grid.h>
#include <wx/icon.h>
#include <wx/iconbndl.h>
#include <wx/iconloc.h>
#include <wx/image.h>
#include <wx/imaggif.h>
#include <wx/imagiff.h>
#include <wx/imagjpeg.h>
#include <wx/imagpcx.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/memory.h>
#include <wx/mimetype.h>
#if defined(__WXMSW__)
#include <wx/msw/ole/automtn.h>
#include <wx/metafile.h>
#include <wx/msw/ole/oleutils.h>
#endif
#include <wx/palette.h>
#include <wx/pen.h>
#include <wx/position.h>
#include <wx/propgrid/property.h>
#include <wx/regex.h>
#include <wx/region.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/sysopt.h>
#include <wx/tarstrm.h>
#include <wx/textctrl.h>
#include <wx/unichar.h>
#include <wx/ustring.h>
#include <wx/variant.h>
#include <wx/vector.h>
#include <wx/versioninfo.h>
#include <wx/wrapsizer.h>
#include <wx/zipstrm.h>

#if wxCHECK_VERSION(3, 1, 6)  // wxWidets-3.1.6 or higher
void unreadVariable_wxBitmapBundle(const wxBitmap &bmp, const wxIcon &icon, const wxImage &image, const char *const * xpm, const wxBitmapBundle &bundle)
{
    // cppcheck-suppress unusedVariable
    wxBitmapBundle a;
    // cppcheck-suppress unreadVariable
    wxBitmapBundle b(bmp);
    // cppcheck-suppress unreadVariable
    wxBitmapBundle c(icon);
    // cppcheck-suppress unreadVariable
    wxBitmapBundle d(image);
    // cppcheck-suppress unreadVariable
    wxBitmapBundle e(xpm);
    // cppcheck-suppress unreadVariable
    wxBitmapBundle f(bundle);
}
#endif

#if wxCHECK_VERSION(3, 1, 3)  // wxWidets-3.1.3 or higher
void unreadVariable_wxDCTextBgModeChanger(wxDC &dc)
{
    // cppcheck-suppress unreadVariable
    wxDCTextBgModeChanger a(dc);
}

void unreadVariable_wxDCTextBgColourChanger(wxDC &dc, const wxColour &colour)
{
    // cppcheck-suppress unreadVariable
    wxDCTextBgColourChanger a(dc);
    // cppcheck-suppress unreadVariable
    wxDCTextBgColourChanger b(dc, colour);
}
#endif

void unreadVariable_wxZipEntry(const wxZipEntry &entry)
{
    // cppcheck-suppress unreadVariable
    wxZipEntry a(entry);
}

void unreadVariable_wxTarEntry(const wxTarEntry &entry)
{
    // cppcheck-suppress unreadVariable
    wxTarEntry a(entry);
}

void unreadVariable_wxDCTextColourChanger(wxDC &dc, const wxColour &colour)
{
    // cppcheck-suppress unreadVariable
    wxDCTextColourChanger a(dc);
    // cppcheck-suppress unreadVariable
    wxDCTextColourChanger b(dc, colour);
}

void unreadVariable_wxDCPenChanger(wxDC &dc, const wxPen &pen)
{
    // cppcheck-suppress unreadVariable
    wxDCPenChanger a(dc, pen);
}

void unreadVariable_wxDCFontChanger(wxDC &dc, const wxFont &font)
{
    // cppcheck-suppress unreadVariable
    wxDCFontChanger a(dc);
    // cppcheck-suppress unreadVariable
    wxDCFontChanger b(dc, font);
}

void unreadVariable_wxDCBrushChanger(wxDC &dc, const wxBrush &brush)
{
    // cppcheck-suppress unreadVariable
    wxDCBrushChanger a(dc, brush);
}

void unreadVariable_wxGBSpan(const int x)
{
    // cppcheck-suppress unusedVariable
    wxGBSpan a;
    // cppcheck-suppress unreadVariable
    wxGBSpan b(x, x);
}

void unreadVariable_wxGBPosition(const int x)
{
    // cppcheck-suppress unusedVariable
    wxGBPosition a;
    // cppcheck-suppress unreadVariable
    wxGBPosition b(x, x);
}

void unreadVariable_wxWrapSizer(const int x)
{
    // cppcheck-suppress unreadVariable
    wxWrapSizer a(x, x);
}

void unreadVariable_wxGridBagSizer(const int x)
{
    // cppcheck-suppress unreadVariable
    wxGridBagSizer a(x, x);
}

void unreadVariable_wxGBSizerItem(const int x, const wxGBPosition &pos)
{
    // cppcheck-suppress unreadVariable
    wxGBSizerItem a(x, x, pos);
}

void unreadVariable_wxSizerItem(const int x)
{
    // cppcheck-suppress unreadVariable
    wxSizerItem a(x, x);
}

void unreadVariable_wxFlexGridSizer(const int x)
{
    // cppcheck-suppress unreadVariable
    wxFlexGridSizer a(x, x, x);
}

void unreadVariable_wxBoxSizer(const int orient)
{
    // cppcheck-suppress unreadVariable
    wxBoxSizer a(orient);
}

void unreadVariable_wxGridSizer(int x)
{
    // cppcheck-suppress unreadVariable
    wxGridSizer a(x, x, x);
}

void unreadVariable_wxStaticBoxSizer(wxStaticBox *box, const int orient, wxWindow *parent, const wxString &label)
{
    // cppcheck-suppress unreadVariable
    wxStaticBoxSizer a(box, orient);
    // cppcheck-suppress unreadVariable
    wxStaticBoxSizer b(orient, parent);
    // cppcheck-suppress unreadVariable
    wxStaticBoxSizer c(orient, parent, label);
}

void unusedVariable_wxDelegateRendererNative()
{
    // cppcheck-suppress unusedVariable
    wxDelegateRendererNative a;
}

void unusedVariable_wxHeaderButtonParams()
{
    // cppcheck-suppress unusedVariable
    wxHeaderButtonParams a;
}

void unusedVariable_wxRegionIterator()
{
    // cppcheck-suppress unusedVariable
    wxRegionIterator a;
}

void unusedVariable_wxRegionContain()
{
    // cppcheck-suppress unusedVariable
    wxRegionContain a;
}

void unusedVariable_wxPalette()
{
    // cppcheck-suppress unusedVariable
    wxPalette a;
}

void unusedVariable_wxJPEGHandler()
{
    // cppcheck-suppress unusedVariable
    wxJPEGHandler a;
}

void unusedVariable_wxGIFHandler()
{
    // cppcheck-suppress unusedVariable
    wxGIFHandler a;
}

void unusedVariable_wxPCXHandler()
{
    // cppcheck-suppress unusedVariable
    wxPCXHandler a;
}

void unusedVariable_wxIFFHandler()
{
    // cppcheck-suppress unusedVariable
    wxIFFHandler a;
}

void unusedVariable_wxGraphicsBrush()
{
    // cppcheck-suppress unusedVariable
    wxGraphicsBrush a;
}

void unusedVariable_wxGraphicsMatrix()
{
    // cppcheck-suppress unusedVariable
    wxGraphicsMatrix a;
}

void unusedVariable_wxGraphicsFont()
{
    // cppcheck-suppress unusedVariable
    wxGraphicsFont a;
}

void unusedVariable_wxIconBundle()
{
    // cppcheck-suppress unusedVariable
    wxIconBundle a;
}

void unusedVariable_wxStdDialogButtonSizer()
{
    // cppcheck-suppress unusedVariable
    wxStdDialogButtonSizer a;
}

void unusedVariable_wxColourDatabase()
{
    // cppcheck-suppress unusedVariable
    wxColourDatabase a;
}

void unusedVariable_wxFontEnumerator()
{
    // cppcheck-suppress unusedVariable
    wxFontEnumerator a;
}

void unusedVariable_wxCursor()
{
    // cppcheck-suppress unusedVariable
    wxCursor a;
}

void unusedVariable_wxBitmapHandler()
{
    // cppcheck-suppress unusedVariable
    wxBitmapHandler a;
}

void unusedVariable_wxNativeFontInfo()
{
    // cppcheck-suppress unusedVariable
    wxNativeFontInfo a;
}

void unreadVariable_wxDCClipper(wxDC &dc, const wxRegion &region)
{
    // cppcheck-suppress unreadVariable
    wxDCClipper a(dc, region);
}

void unreadVariable_wxMask(const wxBitmap &bmp, int x, const wxColour & colour)
{
    // cppcheck-suppress unusedVariable
    wxMask a;
    // cppcheck-suppress unreadVariable
    wxMask b(bmp);
    // cppcheck-suppress unreadVariable
    wxMask c(bmp, x);
    // cppcheck-suppress unreadVariable
    wxMask d(bmp, colour);
}

void unreadVariable_wxGraphicsGradientStops()
{
    // cppcheck-suppress unusedVariable
    wxGraphicsGradientStops a;
    // cppcheck-suppress unreadVariable
    wxGraphicsGradientStops b(wxTransparentColour);
    // cppcheck-suppress unreadVariable
    wxGraphicsGradientStops c(wxTransparentColour, wxTransparentColour);
}

void unreadVariable_wxGraphicsGradientStop()
{
    // cppcheck-suppress unusedVariable
    wxGraphicsGradientStop a;
    // cppcheck-suppress unreadVariable
    wxGraphicsGradientStop b(wxTransparentColour);
    // cppcheck-suppress unreadVariable
    wxGraphicsGradientStop c(wxTransparentColour, 0.42);
}

void unusedVariable_wxFontMetrics()
{
    // cppcheck-suppress unusedVariable
    wxFontMetrics a;
}

void unusedVariable_wxIconLocation()
{
    // cppcheck-suppress unusedVariable
    wxIconLocation a;
}

void unreadVariable_wxIcon(const wxIcon &icon, const wxIconLocation &loc, const char *const *ptr)
{
    // cppcheck-suppress unusedVariable
    wxIcon a;
    // cppcheck-suppress unreadVariable
    wxIcon b(icon);
    // cppcheck-suppress unreadVariable
    wxIcon c(loc);
    // cppcheck-suppress unreadVariable
    wxIcon d(ptr);
}

void unreadVariable_wxImage(const wxImage &image, const int x)
{
    // cppcheck-suppress unusedVariable
    wxImage a;
    // cppcheck-suppress unreadVariable
    wxImage b(image);
    // cppcheck-suppress unreadVariable
    wxImage c(x, x);
    // cppcheck-suppress unreadVariable
    wxImage d(x, x, true);
}

void unreadVariable_wxUString(const wxUString &str, const wxChar32 *strPtr)
{
    // cppcheck-suppress unusedVariable
    wxUString a;
    // cppcheck-suppress unreadVariable
    wxUString b(str);
    // cppcheck-suppress unreadVariable
    wxUString c(strPtr);
}

void unreadVariable_wxAny(const wxVariant &variant, const wxAny &any)
{
    // cppcheck-suppress unusedVariable
    wxAny a;
    // cppcheck-suppress unreadVariable
    wxAny b(42);
    // cppcheck-suppress unreadVariable
    wxAny c(variant);
    // cppcheck-suppress unreadVariable
    wxAny d(any);
}

void unreadVariable_wxVariant(wxVariantData *data,
                              const wxString &name,
                              const wxVariant &variant,
                              const wxAny &any,
                              const wxChar *valuePtr,
                              const wxString &valueStr,
                              const wxChar charValue,
                              long lValue,
                              bool bvalue)
{
    // cppcheck-suppress unusedVariable
    wxVariant a;
    // cppcheck-suppress unreadVariable
    wxVariant b(data);
    // cppcheck-suppress unreadVariable
    wxVariant c(data, name);
    // cppcheck-suppress unreadVariable
    wxVariant d(variant);
    // cppcheck-suppress unreadVariable
    wxVariant e(any);
    // cppcheck-suppress unreadVariable
    wxVariant f(valuePtr);
    // cppcheck-suppress unreadVariable
    wxVariant g(valuePtr, name);
    // cppcheck-suppress unreadVariable
    wxVariant h(valueStr);
    // cppcheck-suppress unreadVariable
    wxVariant i(valueStr, name);
    // cppcheck-suppress unreadVariable
    wxVariant j(charValue);
    // cppcheck-suppress unreadVariable
    wxVariant k(charValue, name);
    // cppcheck-suppress unreadVariable
    wxVariant l(lValue);
    // cppcheck-suppress unreadVariable
    wxVariant m(lValue, name);
    // cppcheck-suppress unreadVariable
    wxVariant n(bvalue);
    // cppcheck-suppress unreadVariable
    wxVariant o(bvalue, name);
}

#if defined(__WXMSW__)

void unusedVariable_wxMetafile()
{
    // cppcheck-suppress unusedVariable
    wxMetafile a;
}

void unusedVariable_wxVariantDataErrorCode()
{
    // cppcheck-suppress unusedVariable
    wxVariantDataErrorCode a;
}
void unusedVariable_wxVariantDataCurrency()
{
    // cppcheck-suppress unusedVariable
    wxVariantDataCurrency a;
}
void unusedVariable_wxVariantDataSafeArray()
{
    // cppcheck-suppress unusedVariable
    wxVariantDataSafeArray a;
}
#endif

void unreadVariable_wxBitmap(const wxBitmap &bmp, const char bits[], const int x, const wxSize &sz)
{
    // cppcheck-suppress unusedVariable
    wxBitmap a;
    // cppcheck-suppress unreadVariable
    wxBitmap b(bmp);
    // cppcheck-suppress unreadVariable
    wxBitmap c(bits, x, x);
    // cppcheck-suppress unreadVariable
    wxBitmap d(bits, x, x, x);
    // cppcheck-suppress unreadVariable
    wxBitmap e(x, x);
    // cppcheck-suppress unreadVariable
    wxBitmap f(x, x, x);
    // cppcheck-suppress unreadVariable
    wxBitmap g(sz);
    // cppcheck-suppress unreadVariable
    wxBitmap h(sz, x);
}

void unusedVariable_wxChar()
{
    // cppcheck-suppress unusedVariable
    wxChar a;
}

void unusedVariable_wxUniChar(const int c)
{
    // cppcheck-suppress unusedVariable
    wxUniChar a;
    // cppcheck-suppress unreadVariable
    wxUniChar b(c);
}

void unusedVariable_wxSystemOptions()
{
    // cppcheck-suppress unusedVariable
    wxSystemOptions a;
}

void unusedVariable_wxSystemSettings()
{
    // cppcheck-suppress unusedVariable
    wxSystemSettings a;
}

void unusedVariable_wxPenList()
{
    // cppcheck-suppress unusedVariable
    wxPenList a;
}

void unusedVariable_wxPen(const wxColour &colour, int width, const wxPenStyle style, const wxPen &pen)
{
    // cppcheck-suppress unusedVariable
    wxPen a;
    // cppcheck-suppress unreadVariable
    wxPen b(colour, width);
    // cppcheck-suppress unreadVariable
    wxPen c(colour, width, style);
    // cppcheck-suppress unreadVariable
    wxPen d(pen);
}

void unusedVariable_wxBrush(const wxColour &color, const wxBrushStyle style, const wxBitmap &bmp, const wxBrush &brush)
{
    // cppcheck-suppress unusedVariable
    wxBrush a;
    // cppcheck-suppress unreadVariable
    wxBrush b(color, style);
    // cppcheck-suppress unreadVariable
    wxBrush c(bmp);
    // cppcheck-suppress unreadVariable
    wxBrush d(brush);
}

void unusedVariable_wxFontList()
{
    // cppcheck-suppress unusedVariable
    wxFontList a;
}

void unusedVariable_wxFontInfo(const double pointSize, const wxSize &sz)
{
    // cppcheck-suppress unusedVariable
    wxFontInfo a;
    // cppcheck-suppress unreadVariable
    wxFontInfo b(pointSize);
    // cppcheck-suppress unreadVariable
    wxFontInfo c(sz);
}

void unusedVariable_wxFont(const wxFont &font,
                           const wxFontInfo &fontInfo,
                           const int pointSize,
                           const wxFontFamily family,
                           const wxFontStyle style,
                           const wxFontWeight weight,
                           const bool underline,
                           const wxString &faceName,
                           const wxFontEncoding encoding)
{
    // cppcheck-suppress unusedVariable
    wxFont a;
    // cppcheck-suppress unreadVariable
    wxFont b(font);
    // cppcheck-suppress unreadVariable
    wxFont c(fontInfo);
    // cppcheck-suppress unreadVariable
    wxFont d(pointSize, family, style, weight);
    // cppcheck-suppress unreadVariable
    wxFont e(pointSize, family, style, weight, underline);
    // cppcheck-suppress unreadVariable
    wxFont f(pointSize, family, style, weight, underline, faceName);
    // cppcheck-suppress unreadVariable
    wxFont g(pointSize, family, style, weight, underline, faceName, encoding);
}

void unusedVariable_wxVector()
{
    // cppcheck-suppress unusedVariable
    wxVector<int> a;
}

void unusedVariable_wxArrayInt()
{
    // cppcheck-suppress unusedVariable
    wxArrayInt a;
}

void unusedVariable_wxArrayDouble()
{
    // cppcheck-suppress unusedVariable
    wxArrayDouble a;
}

void unusedVariable_wxArrayShort()
{
    // cppcheck-suppress unusedVariable
    wxArrayShort a;
}

void unusedVariable_wxArrayString()
{
    // cppcheck-suppress unusedVariable
    wxArrayString a;
}

void unusedVariable_wxArrayPtrVoid()
{
    // cppcheck-suppress unusedVariable
    wxArrayPtrVoid a;
}

void unreadVariable_wxColour(const unsigned char uc, const wxString &name, const unsigned long colRGB, const wxColour &colour)
{
    // cppcheck-suppress unusedVariable
    wxColour a;
    // cppcheck-suppress unreadVariable
    wxColour b(uc, uc, uc);
    // cppcheck-suppress unreadVariable
    wxColour c(uc, uc, uc, uc);
    // cppcheck-suppress unreadVariable
    wxColour d(name);
    // cppcheck-suppress unreadVariable
    wxColour e(colRGB);
    // cppcheck-suppress unreadVariable
    wxColour f(colour);
}

void unreadVariable_wxPoint2DInt(const wxInt32 x, const wxPoint2DInt& pti, const wxPoint &pt)
{
    // cppcheck-suppress unusedVariable
    wxPoint2DInt a;
    // cppcheck-suppress unreadVariable
    wxPoint2DInt b(x, x);
    // cppcheck-suppress unreadVariable
    wxPoint2DInt c(pti);
    // cppcheck-suppress unreadVariable
    wxPoint2DInt d(pt);
}

void unreadVariable_wxPoint2DDouble(const wxDouble x, const wxPoint2DDouble& ptd, const wxPoint2DInt& pti, const wxPoint &pt)
{
    // cppcheck-suppress unusedVariable
    wxPoint2DDouble a;
    // cppcheck-suppress unreadVariable
    wxPoint2DDouble b(x, x);
    // cppcheck-suppress unreadVariable
    wxPoint2DDouble c(ptd);
    // cppcheck-suppress unreadVariable
    wxPoint2DDouble d(pti);
    // cppcheck-suppress unreadVariable
    wxPoint2DDouble e(pt);
}

void unusedVariable_wxAcceleratorEntry()
{
    // cppcheck-suppress unusedVariable
    wxAcceleratorEntry a;
}

void unreadVariable_wxDateSpan(const int x)
{
    // cppcheck-suppress unusedVariable
    wxDateSpan a;
    // cppcheck-suppress unreadVariable
    wxDateSpan b{x};
    // cppcheck-suppress unreadVariable
    wxDateSpan c{x, x};
    // cppcheck-suppress unreadVariable
    wxDateSpan d{x, x, x};
    // cppcheck-suppress unreadVariable
    wxDateSpan e{x, x, x, x};
}

void unreadVariable_wxTimeSpan(const long x, const wxLongLong y)
{
    // cppcheck-suppress unusedVariable
    wxTimeSpan a;
    // cppcheck-suppress unreadVariable
    wxTimeSpan b{};
    // cppcheck-suppress unreadVariable
    wxTimeSpan c{x};
    // cppcheck-suppress unreadVariable
    wxTimeSpan d{x, x};
    // cppcheck-suppress unreadVariable
    wxTimeSpan e{x, x, y};
    // cppcheck-suppress unreadVariable
    wxTimeSpan f{x, x, y, y};
}

void unreadVariable_wxFileType(const wxFileTypeInfo &info)
{
    // cppcheck-suppress unreadVariable
    wxFileType a(info);
}

void unreadVariable_wxPosition(const int x)
{
    // cppcheck-suppress unusedVariable
    wxPosition a;
    // cppcheck-suppress unreadVariable
    wxPosition b{};
    // cppcheck-suppress unreadVariable
    wxPosition c{x,x};
}

void unreadVariable_wxRegEx(const wxString &expr, const int flags)
{
    // cppcheck-suppress unusedVariable
    wxRegEx a;
    // cppcheck-suppress unreadVariable
    wxRegEx b{expr};
    // cppcheck-suppress unreadVariable
    wxRegEx c{expr, flags};
}

void unreadVariable_wxRegion(const wxCoord x, const wxPoint &pt, const wxRect &rect, const wxRegion &region, const wxBitmap &bmp)
{
    // cppcheck-suppress unusedVariable
    wxRegion a;
    // cppcheck-suppress unreadVariable
    wxRegion b{};
    // cppcheck-suppress unreadVariable
    wxRegion c{x,x,x,x};
    // cppcheck-suppress unreadVariable
    wxRegion d{pt,pt};
    // cppcheck-suppress unreadVariable
    wxRegion e{rect};
    // cppcheck-suppress unreadVariable
    wxRegion f{region};
    // cppcheck-suppress unreadVariable
    wxRegion g{bmp};
}

void unreadVariable_wxVersionInfo(const wxString &name, const int major, const int minor, const int micro, const wxString &description, const wxString &copyright)
{
    // cppcheck-suppress unusedVariable
    wxVersionInfo a;
    // cppcheck-suppress unreadVariable
    wxVersionInfo b(name);
    // cppcheck-suppress unreadVariable
    wxVersionInfo c(name, major);
    // cppcheck-suppress unreadVariable
    wxVersionInfo d(name, major, minor);
    // cppcheck-suppress unreadVariable
    wxVersionInfo e(name, major, minor, micro);
    // cppcheck-suppress unreadVariable
    wxVersionInfo f(name, major, minor, micro, description);
    // cppcheck-suppress unreadVariable
    wxVersionInfo g(name, major, minor, micro, description, copyright);
}

void unreadVariable_wxSize(const wxSize &s)
{
    // cppcheck-suppress unusedVariable
    wxSize a;
    // cppcheck-suppress unreadVariable
    wxSize b{};
    // cppcheck-suppress unreadVariable
    wxSize c{4, 2};
    // cppcheck-suppress unreadVariable
    wxSize d(4, 2);
    // cppcheck-suppress unreadVariable
    wxSize e(s);
}

void unreadVariable_wxPoint(const wxRealPoint &rp, const int x, const int y)
{
    // cppcheck-suppress unusedVariable
    wxPoint a;
    // cppcheck-suppress unreadVariable
    wxPoint b{};
    // cppcheck-suppress unreadVariable
    wxPoint c{4, 2};
    // cppcheck-suppress unreadVariable
    wxPoint d(4, 2);
    // cppcheck-suppress unreadVariable
    wxPoint e{x, 2};
    // cppcheck-suppress unreadVariable
    wxPoint f(4, y);
    // cppcheck-suppress unreadVariable
    wxPoint g(rp);
}

void unreadVariable_wxRealPoint(const wxPoint &pt, const double x, const double y)
{
    // cppcheck-suppress unusedVariable
    wxRealPoint a;
    // cppcheck-suppress unreadVariable
    wxRealPoint b{};
    // cppcheck-suppress unreadVariable
    wxRealPoint c{4.0, 2.0};
    // cppcheck-suppress unreadVariable
    wxRealPoint d(4.0, 2.0);
    // cppcheck-suppress unreadVariable
    wxRealPoint e{x, 2.0};
    // cppcheck-suppress unreadVariable
    wxRealPoint f(4.0, y);
    // cppcheck-suppress unreadVariable
    wxRealPoint g(pt);
}

void unreadVariable_wxRect(const int x, const wxPoint &pt, const wxSize &sz)
{
    // cppcheck-suppress unusedVariable
    wxRect a;
    // cppcheck-suppress unreadVariable
    wxRect b{};
    // cppcheck-suppress unreadVariable
    wxRect c{x,x,x,x};
    // cppcheck-suppress unreadVariable
    wxRect d{pt,sz};
    // cppcheck-suppress unreadVariable
    wxRect e{sz};
    // cppcheck-suppress unreadVariable
    wxRect f(x,x,x,x);
    // cppcheck-suppress unreadVariable
    wxRect g(pt,sz);
    // cppcheck-suppress unreadVariable
    wxRect h(sz);
}

void uninitvar_wxRegEx_GetMatch(const wxRegEx &obj, size_t *start, size_t *len, size_t index)
{
    size_t s,l;
    size_t *sPtr,*lPtr;
    // cppcheck-suppress uninitvar
    (void)obj.GetMatch(&s,lPtr);
    // TODO cppcheck-suppress uninitvar
    (void)obj.GetMatch(sPtr,&l);
    (void)obj.GetMatch(&s,&l);
    (void)obj.GetMatch(start,len);
    (void)obj.GetMatch(start,len,0);
    (void)obj.GetMatch(start,len,index);
}

#ifdef __VISUALC__
// Ensure no duplicateBreak warning is issued after wxLogApiError() calls.
// This function does not terminate execution.
bool duplicateBreak_wxLogApiError(const wxString &msg, const HRESULT &hr, wxString &str)
{
    if (hr) {
        wxLogApiError(msg,hr);
        str = "fail";
        return false;
    }
    return true;
}
#endif

void argDirection_wxString_ToDouble(const wxString &str)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    double value;
    const bool convOk = str.ToDouble(&value);
    if (convOk && value <= 42.0) {}
}

void argDirection_wxString_ToCDouble(const wxString &str)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    double value;
    const bool convOk = str.ToCDouble(&value);
    if (convOk && value <= 42.0) {}
}

void argDirection_wxTextCtrl_GetSelection(const wxTextCtrl *const textCtrl)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    long start;
    long end;
    textCtrl->GetSelection(&start, &end);
    if (start > 0 && end > 0) {}
}

void useRetval_wxString_MakeCapitalized(wxString &str)
{
    // No warning is expected for
    str.MakeCapitalized();
}

void useRetval_wxString_MakeLower(wxString &str)
{
    // No warning is expected for
    str.MakeLower();
}

void useRetval_wxString_MakeUpper(wxString &str)
{
    // No warning is expected for
    str.MakeUpper();
}

wxString containerOutOfBounds_wxArrayString(void)
{
    wxArrayString a;
    a.Add("42");
    a.Clear();
    // TODO: wxArrayString is defined to be a vector
    // TODO: cppcheck-suppress containerOutOfBounds
    return a[0];
}

int containerOutOfBounds_wxArrayInt(void)
{
    wxArrayInt a;
    a.Add(42);
    a.Clear();
    // TODO: wxArrayString is defined to be a vector
    // TODO: cppcheck-suppress containerOutOfBounds
    return a[0];
}

void ignoredReturnValue_wxDC_GetSize(const wxDC &dc, wxCoord *width, wxCoord *height)
{
    // No warning is expected for
    dc.GetSize(width, height);
    // No warning is expected for
    (void)dc.GetSize();
}

void ignoredReturnValue_wxDC_GetSizeMM(const wxDC &dc, wxCoord *width, wxCoord *height)
{
    // No warning is expected for
    dc.GetSizeMM(width, height);
    // Now warning is expected for
    (void)dc.GetSizeMM();
}

void ignoredReturnValue_wxPanel_Layout(wxPanel* panel) { // #14403
    panel->Layout();
}

wxSizerItem* invalidFunctionArgBool_wxSizer_Add(wxSizer *sizer, wxWindow * window, const wxSizerFlags &flags)
{
    // No warning is expected for
    return sizer->Add(window,flags);
}

bool invalidFunctionArgBool_wxPGProperty_Hide(wxPGProperty *pg, bool hide, int flags)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)pg->Hide(hide, true);
    // No warning is expected for
    return pg->Hide(hide, flags);
}

wxTextCtrlHitTestResult nullPointer_wxTextCtrl_HitTest(const wxTextCtrl& txtCtrl, const wxPoint& pos)
{
    // no nullPointer-warning is expected
    return txtCtrl.HitTest(pos, NULL);
}

void validCode()
{
    wxString str = wxGetCwd();
    (void)str;

    wxLogGeneric(wxLOG_Message, "test %d", 0);
    wxLogMessage("test %s", "str");

    wxString translation1 = _("text");
    wxString translation2 = wxGetTranslation("text");
    wxString translation3 = wxGetTranslation("string", "domain");
    (void)translation1;
    (void)translation2;
    (void)translation3;
}

#if wxUSE_GUI==1
void validGuiCode()
{
#if wxUSE_SPINCTRL==1
    extern wxSpinCtrl spinCtrlInstance;
    spinCtrlInstance.SetBase(10);
    spinCtrlInstance.SetBase(16);
#endif
}
#endif

void nullPointer(const wxString &str)
{
    // cppcheck-suppress nullPointer
    wxLogGeneric(wxLOG_Message, (char*)NULL);
    // cppcheck-suppress nullPointer
    wxLogMessage((char*)NULL);

    double *doublePtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToDouble(doublePtr);
    double *doublePtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCDouble(doublePtr1);

    long * longPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToLong(longPtr);
    long * longPtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCLong(longPtr1);

    unsigned long * ulongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToULong(ulongPtr);
    unsigned long * ulongPtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCULong(ulongPtr1);

    long long * longLongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToLongLong(longLongPtr);

    unsigned long long * ulongLongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToULongLong(ulongLongPtr);
}

void nullPointer_wxSizer_Add(wxSizer &sizer, wxWindow *w)
{
    wxWindow * const ptr = 0;
    // @todo cppcheck-suppress nullPointer
    sizer.Add(ptr);
    // No warning shall be issued for
    sizer.Add(w);
}

void uninitvar_wxSizer_Add(wxSizer &sizer, wxWindow *w,wxObject* userData)
{
    int uninit1, uninit2, uninit3;
    // cppcheck-suppress uninitvar
    sizer.Add(w,uninit1);
    // cppcheck-suppress uninitvar
    sizer.Add(w,4,uninit2);
    // cppcheck-suppress uninitvar
    sizer.Add(w,4,2,uninit3,userData);
}

void ignoredReturnValue(const wxString &s)
{
    // cppcheck-suppress ignoredReturnValue
    wxGetCwd();
    // cppcheck-suppress ignoredReturnValue
    wxAtoi(s);
    // cppcheck-suppress ignoredReturnValue
    wxAtol(s);
    // cppcheck-suppress ignoredReturnValue
    wxAtof(s);
}

void invalidFunctionArg(const wxString &str)
{
#if wxUSE_SPINCTRL==1
    extern wxSpinCtrl spinCtrlInstance;
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(0);
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(5);
#endif

    long l;
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, 1);
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, 37);
}

void uninitvar(wxWindow &w)
{
    wxLogLevel logLevelUninit;
    // cppcheck-suppress constVariable
    char cBufUninit[10];
    const char *pcUninit;
    bool uninitBool;
    // cppcheck-suppress uninitvar
    wxLogGeneric(logLevelUninit, "test");
    // cppcheck-suppress uninitvar
    wxLogMessage(cBufUninit);
    // cppcheck-suppress uninitvar
    wxLogMessage(pcUninit);
    // cppcheck-suppress uninitvar
    w.Close(uninitBool);
}

void uninitvar_wxStaticText(wxStaticText &s)
{
    // no warning
    s.Wrap(-1);
    int uninitInt;
    // cppcheck-suppress uninitvar
    s.Wrap(uninitInt);
}

void uninitvar_wxString_NumberConversion(const wxString &str, const int numberBase)
{
    int uninitInteger1;
    int uninitInteger2;
    int uninitInteger3;
    int uninitInteger4;
    int uninitInteger5;
    int uninitInteger6;
    long l;
    long long ll;
    unsigned long ul;
    unsigned long long ull;

    // cppcheck-suppress uninitvar
    (void)str.ToLong(&l, uninitInteger1);
    // cppcheck-suppress uninitvar
    (void)str.ToLongLong(&ll, uninitInteger2);
    // cppcheck-suppress uninitvar
    (void)str.ToULong(&ul, uninitInteger3);
    // cppcheck-suppress uninitvar
    (void)str.ToULongLong(&ull, uninitInteger4);

    // cppcheck-suppress uninitvar
    (void)str.ToCLong(&l, uninitInteger5);
    // cppcheck-suppress uninitvar
    (void)str.ToCULong(&ul, uninitInteger6);
}

void uninitvar_SetMenuBar(wxFrame * const framePtr, wxMenuBar * const menuBarPtr)
{
    wxMenuBar *menuBar;
    // cppcheck-suppress uninitvar
    framePtr->SetMenuBar(menuBar);
    framePtr->SetMenuBar(menuBarPtr);
}

void uninitvar_wxMenuBarAppend(wxMenuBar * const menuBarPtr, wxMenu * const menuPtr, const wxString &title)
{
    wxMenu *menu;
    // cppcheck-suppress uninitvar
    menuBarPtr->Append(menu, title);
    menuBarPtr->Append(menuPtr, title);
}

void deprecatedFunctions_wxDataViewCustomRenderer(wxDataViewCustomRenderer &dataViewCustomRenderer, wxPoint cursor, wxRect cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col)
{
    // cppcheck-suppress ActivateCalled
    dataViewCustomRenderer.Activate(cell, model, item, col);
    // cppcheck-suppress LeftClickCalled
    dataViewCustomRenderer.LeftClick(cursor, cell, model, item, col);
}

void deprecatedFunctions([[maybe_unused]] wxApp &a,
                         const wxString &s,
                         [[maybe_unused]] wxArtProvider *artProvider,
                         [[maybe_unused]] wxCalendarCtrl &calenderCtrl,
                         wxComboCtrl &comboCtrl,
                         wxChar * path)
{
#ifdef __WXOSX__
    // cppcheck-suppress MacOpenFileCalled
    a.MacOpenFile(s);
#endif

#if wxCHECK_VERSION(3, 1, 0)  // wxWidets-3.1.0 or higher:
    // Some functions are not available anymore in newer versions

    // cppcheck-suppress ShowPopupCalled
    comboCtrl.ShowPopup();
#else
    // cppcheck-suppress InsertCalled
    wxArtProvider::Insert(artProvider);

    // cppcheck-suppress GetTextIndentCalled
    // cppcheck-suppress ignoredReturnValue
    comboCtrl.GetTextIndent();

    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(true);
    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(false);
    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(/*default=false*/);

    // cppcheck-suppress SetTextIndentCalled
    comboCtrl.SetTextIndent(0);

#if wxUSE_DEBUG_CONTEXT==1
    // cppcheck-suppress GetLevelCalled
    // cppcheck-suppress ignoredReturnValue
    wxDebugContext::GetLevel();
    // cppcheck-suppress SetLevelCalled
    wxDebugContext::SetLevel(42);
#endif

    // cppcheck-suppress wxDos2UnixFilenameCalled
    wxDos2UnixFilename(path);

    // cppcheck-suppress wxFileNameFromPathCalled
    // cppcheck-suppress ignoredReturnValue
    wxFileNameFromPath(wxT_2("../test.c"));
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    // EnableYearChange() is not available on these GUI systems
#else
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(false);
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(true);
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(/*default=yes*/);
#endif
}

void wxString_test1(wxString s)
{
    for (int i = 0; i <= s.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        s[i] = 'x';
    }
}

void wxString_test2()
{
    wxString s;
    // cppcheck-suppress containerOutOfBounds
    s[1] = 'a';
    s.append("abc");
    s[1] = 'B';
    printf("%s", static_cast<const char*>(s.c_str()));
    wxPrintf("%s", s);
    wxPrintf("%s", s.c_str());
    s.Clear();
}

wxString::iterator wxString_test3()
{
    wxString wxString1;
    wxString wxString2;
    // cppcheck-suppress mismatchingContainers
    for (wxString::iterator it = wxString1.begin(); it != wxString2.end(); ++it)
    {}

    wxString::iterator it = wxString1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

wxGrid::wxGridSelectionModes get_wxGridSelectionModes()
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar // TODO: configure enum #8183
    return wxGrid::wxGridSelectCells;
}
