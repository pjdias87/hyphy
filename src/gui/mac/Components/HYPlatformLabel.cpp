/*
    Button component for Mac OS.

    Sergei L. Kosakovsky Pond, May 2000 - December 2002
*/

#include "HYLabel.h"
#include "HYUtils.h"

//__________________________________________________________________

_HYPlatformLabel::_HYPlatformLabel(void)
{
    backFill = NewPixPat();
    if (!backFill) {
        warnError (-108);
    }
    RGBColor  wht = {0xffff,0xffff,0xffff};
    MakeRGBPat (backFill,&wht);
    fontID = 0;
    fc.red = fc.blue = fc.green = 0;
}

//__________________________________________________________________

_HYPlatformLabel::~_HYPlatformLabel(void)
{
    if (backFill) {
        DisposePixPat (backFill);
    }

}

//__________________________________________________________________

void        _HYPlatformLabel::_SetBackColor (_HYColor& c)
{
    RGBColor newBG;
    newBG.red = c.R*256;
    newBG.blue = c.B*256;
    newBG.green = c.G*256;
    MakeRGBPat (backFill,&newBG);

}

//__________________________________________________________________

void        _HYPlatformLabel::_SetFont (_HYFont& f)
{
    Str255 fName;
    StringToStr255 (f.face,fName);
    short fNum=0;
    GetFNum (fName,&fNum);
    fontID = fNum;
}
//__________________________________________________________________

void        _HYPlatformLabel::_SetForeColor (_HYColor& c)
{
    fc.red = c.R*256;
    fc.green = c.G*256;
    fc.blue = c.B*256;
}

//__________________________________________________________________
void        _HYPlatformLabel::_Update (Ptr p)
{
    _Paint (p);
}

//__________________________________________________________________
void        _HYPlatformLabel::_SetDimensions (_HYRect r, _HYRect rel)
{
    _HYLabel* theParent = (_HYLabel*) this;
    theParent->_HYPlatformComponent::_SetDimensions (r,rel);
    _SetVisibleSize (rel);
}

//__________________________________________________________________
void        _HYPlatformLabel::_SetVisibleSize (_HYRect rel)
{
    _HYLabel* theParent = (_HYLabel*) this;
    labelRect.left=rel.left;
    labelRect.top = rel.top;
    _HYRect s = theParent->_SuggestDimensions();
    labelRect.right = labelRect.left+s.right;
    labelRect.bottom = labelRect.top+s.bottom;

    AlignRectangle (rel, labelRect, theParent->GetAlignFlags());
}


//__________________________________________________________________
void        _HYPlatformLabel::_Paint (Ptr p)
{

    _HYLabel * theParent = (_HYLabel*)this;

    _HYRect * relRect = (_HYRect*)p;
    Rect    cRect;
    cRect.left = relRect->left;
    cRect.right = relRect->right;
    cRect.top = relRect->top;
    cRect.bottom = relRect->bottom;

    if (!(theParent->settings.width&HY_COMPONENT_TRANSP_BG)) {
        FillCRect (&cRect,backFill);
    } else {
        EraseRect (&cRect);
    }

    if (!(theParent->settings.width&HY_COMPONENT_WELL)) {
        InsetRect (&cRect,2,2);
    }

    GrafPtr thisPort;
    GetPort (&thisPort);

#ifdef OPAQUE_TOOLBOX_STRUCTS
    short   savedFace = GetPortTextFont (thisPort),
            savedSize = GetPortTextSize (thisPort);

    Style   savedStyle = GetPortTextFace (thisPort);
#else
    short   savedFace = thisPort->txFont,
            savedSize = thisPort->txSize;

    Style   savedStyle = thisPort->txFace;
#endif


    TextFont (fontID);
    TextSize (theParent->GetFont().size);
    TextFace (theParent->GetFont().style);
    RgnHandle saveRgn = NewRgn();
    GetClip (saveRgn);
    ClipRect (&cRect);
    RGBColor oldColor;
    GetForeColor (&oldColor);
    if (theParent->HasShadow()) {
        MoveTo (labelRect.left+3,labelRect.bottom-3);
        RGBColor sc = fc;
        sc.red = fc.red/4;
        sc.blue  = fc.blue/4;
        sc.green = fc.green/4;
        RGBForeColor (&sc);
        DrawText (theParent->GetText().sData,0,theParent->GetText().sLength);
        MoveTo (labelRect.left+1,labelRect.bottom-1);
        sc.red = fc.red/2;
        sc.blue  = fc.blue/2;
        sc.green = fc.green/2;
        RGBForeColor (&sc);
        DrawText (theParent->GetText().sData,0,theParent->GetText().sLength);/*
        MoveTo (labelRect.left+1,labelRect.bottom-3);
        RGBColor sc = fc;
        sc.red = fc.red>127?255:fc.red*2;
        sc.blue  = fc.blue>127?255:fc.blue*2;
        sc.green = fc.green>127?255:fc.green*2;
        RGBForeColor (&sc);
        DrawText (theParent->GetText().sData,0,theParent->GetText().sLength);
        MoveTo (labelRect.left+3,labelRect.bottom-1);
        sc.red = fc.red/2;
        sc.blue  = fc.blue/2;
        sc.green = fc.green/2;
        RGBForeColor (&sc);
        DrawText (theParent->GetText().sData,0,theParent->GetText().sLength);*/

    }
    RGBForeColor (&fc);
    MoveTo (labelRect.left+2,labelRect.bottom-2);
    DrawText (theParent->GetText().sData,0,theParent->GetText().sLength);

    RGBForeColor (&oldColor);
    SetClip (saveRgn);
    DisposeRgn (saveRgn);
    TextFont (savedFace);
    TextSize (savedSize);
    TextFace (savedStyle);

    (*theParent)._HYPlatformComponent::_Paint(p);
}

//__________________________________________________________________
_HYRect _HYLabel::_SuggestDimensions (void)
{
    _HYRect res = {10,100,10,100,HY_COMPONENT_NO_SCROLL};
    GrafPtr thisPort;
    GetPort (&thisPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    short   savedFace = GetPortTextFont (thisPort),
            savedSize = GetPortTextSize (thisPort);

    Style   savedStyle = GetPortTextFace (thisPort);
#else
    short   savedFace = thisPort->txFont,
            savedSize = thisPort->txSize;

    Style   savedStyle = thisPort->txFace;
#endif
    TextFont (fontID);
    TextSize (labelFont.size);
    TextFace (labelFont.style);
    res.top = res.bottom = labelFont.size+4;
    res.left = res.right = TextWidth (labelText.sData,0,labelText.sLength) + 5;
    TextFont (savedFace);
    TextSize (savedSize);
    TextFace (savedStyle);
    return res;
}

// EOF