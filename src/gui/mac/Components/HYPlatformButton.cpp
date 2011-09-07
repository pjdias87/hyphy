/*
    Button component for Mac OS.

    Sergei L. Kosakovsky Pond, May 2000 - December 2002
*/

#include "errorfns.h"
#include "HYButton.h"
#include "HYUtils.h"
#include "HYEventTypes.h"

#include <ControlDefinitions.h>

//__________________________________________________________________

_HYPlatformButton::_HYPlatformButton    (void)
{
    backFill = NewPixPat();

    if (!backFill) {
        warnError (-108);
    }

    RGBColor    wht = {0xffff,0xffff,0xffff};
    fontID          = 0;
    buttonControl   = nil;
    buttonRect      = (Rect) {
        0,0,100,100
    };

    MakeRGBPat (backFill,&wht);
}

//__________________________________________________________________

_HYPlatformButton::~_HYPlatformButton   (void)
{
    if (backFill) {
        DisposePixPat (backFill);
    }

    if (buttonControl) {
        DisposeControl(buttonControl);
    }
}

//__________________________________________________________________

void    _HYPlatformButton::_SetBackColor (_HYColor& c)
{
    RGBColor newBG;
    newBG.red = c.R*256;
    newBG.blue = c.B*256;
    newBG.green = c.G*256;
    MakeRGBPat (backFill,&newBG);
}

//__________________________________________________________________

void        _HYPlatformButton::_SetFont (_HYFont& f)
{
    Str255 fName;
    StringToStr255 (f.face,fName);
    short fNum=0;
    GetFNum (fName,&fNum);
    fontID = fNum;
    if (buttonControl) {
        _ApplyFont ();
    }
}

//__________________________________________________________________

void        _HYPlatformButton::_ApplyFont(void)
{
#ifdef TARGET_API_MAC_CARBON
    ControlFontStyleRec fontData;
    fontData.flags = kControlUseFontMask|kControlUseFaceMask|kControlUseSizeMask;
    fontData.font  = fontID;
    fontData.style  = ((_HYButton*)this)->buttonFont.style;
    fontData.size   = ((_HYButton*)this)->buttonFont.size;
    SetControlFontStyle (buttonControl, &fontData);
#endif
}
//__________________________________________________________________
void        _HYPlatformButton::_Update (Ptr p)
{
    _Paint (p);
}

//__________________________________________________________________
void        _HYPlatformButton::_SetDimensions (_HYRect r, _HYRect rel)
{
    _HYButton* theParent = (_HYButton *) this;
    theParent->_HYPlatformComponent::_SetDimensions (r,rel);
    _SetVisibleSize (rel);
}

//__________________________________________________________________
void        _HYPlatformButton::_EnableButton (bool e)
{
    if (buttonControl) {
        HiliteControl (buttonControl,e?kControlNoPart:kControlInactivePart);
    }
}

//__________________________________________________________________
void        _HYPlatformButton::_SetButtonKind (unsigned char k)
{
    if (buttonControl) {
        //_HYButton * theParent = (_HYButton*) this;
        Boolean     onOff = (k==HY_BUTTON_OK);

        SetControlData (buttonControl,
                        kControlNoPart,
                        kControlPushButtonDefaultTag,
                        sizeof(Boolean),
                        (Ptr)&onOff);

    }
}

//__________________________________________________________________
void        _HYPlatformButton::_SetVisibleSize (_HYRect rel)
{
    _HYButton * theParent = (_HYButton*) this;

    buttonRect.left=rel.left;
    buttonRect.top = rel.top;

    _HYRect s = theParent->_SuggestDimensions();

    buttonRect.right =  buttonRect.left+s.right;
    buttonRect.bottom = buttonRect.top+s.bottom;

    AlignRectangle (rel, buttonRect, theParent->GetAlignFlags());

    if (buttonControl) {
        SizeControl (buttonControl,buttonRect.right-buttonRect.left+1,buttonRect.bottom-buttonRect.top+1);
        MoveControl (buttonControl,buttonRect.left,buttonRect.top);
    }
}


//__________________________________________________________________
void        _HYPlatformButton::_Paint (Ptr p)
{
    _HYButton * theParent = (_HYButton*)this;
    _HYRect   * relRect   = (_HYRect*)p;
    Rect        cRect;

    cRect.left   = relRect->left;
    cRect.right  = relRect->right;
    cRect.top    = relRect->top;
    cRect.bottom = relRect->bottom;

    bool         drawBk = (!(theParent->settings.width&HY_COMPONENT_TRANSP_BG));

    if (drawBk) {
        FillCRect (&cRect,backFill);
    } else {
        EraseRect (&cRect);
    }

    if (buttonControl) {
        _PaintMe();
    }

    (*theParent)._HYPlatformComponent::_Paint(p);
}

//__________________________________________________________________
void        _HYPlatformButton::_PaintMe (void)
{
    _HYButton * theParent = (_HYButton*)this;
    bool        drawBk = (!(theParent->settings.width&HY_COMPONENT_TRANSP_BG));

    GrafPtr     thisPort;
    GetPort     (&thisPort);

#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort   (GetWindowPort(theParent->parentWindow));
#else
    SetPort   (theParent->parentWindow);
#endif

    RGBColor newBG,
             saveColor;

    if (drawBk) {
        GetBackColor (&saveColor);
        newBG.red = theParent->backColor.R*256;
        newBG.blue = theParent->backColor.B*256;
        newBG.green = theParent->backColor.G*256;
        RGBBackColor (&newBG);
    }

    Draw1Control (buttonControl);
    if (drawBk) {
        RGBBackColor (&saveColor);
    }

    SetPort     (thisPort);
}
//__________________________________________________________________

_HYRect _HYButton::_SuggestDimensions (void)
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
    TextSize (buttonFont.size);
    TextFace (buttonFont.style);

    res.top = res.bottom = buttonFont.size+8;
    res.left= res.right  = TextWidth (buttonText.sData,0,buttonText.sLength) + 15;

    TextFont (savedFace);
    TextSize (savedSize);
    TextFace (savedStyle);

    return res;
}

//__________________________________________________________________

void        _HYButton::_Activate (void)
{
    if (!activationFlag)
        if (buttonControl&&isEnabled) {
            HiliteControl (buttonControl,0);
            _PaintMe();
            //Draw1Control (buttonControl);
        }

    _HYPlatformComponent::_Activate();
}

//__________________________________________________________________

void        _HYButton::_Deactivate (void)
{
    if (activationFlag)
        if (buttonControl&&isEnabled) {
            HiliteControl (buttonControl, kControlInactivePart);
            _PaintMe();
            //Draw1Control (buttonControl);
        }

    _HYPlatformComponent::_Deactivate();
}

//__________________________________________________________________

void        _HYPlatformButton::_SetText (void)
{
    _HYButton *parent = (_HYButton*)this;

    Str255 buffer;
    StringToStr255 (parent->buttonText, buffer);
    if (buttonControl) {
        SetControlTitle (buttonControl,buffer);
    } else {
        GrafPtr      savePort;
        GetPort (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort (GetWindowPort(parent->parentWindow));
#else
        SetPort (parent->parentWindow);
#endif

        buttonControl = NewControl (parent->parentWindow,&buttonRect,buffer,true,0,0,0,
                                    kControlPushButtonProc /*+(1<<3)*/ ,0);

        checkPointer ((Ptr)buttonControl);
        _ApplyFont ();
        SetPort (savePort);
    }
}

//__________________________________________________________________

bool _HYButton::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord*    theEvent = (EventRecord*)vEvent;
    WindowPtr       dummy;
    if(buttonControl&&isEnabled)
        switch (theEvent->what) {
        case mouseDown: {
            long evtType = FindWindow (theEvent->where,&dummy);
            if (evtType == inContent) {
                Point localClick = theEvent->where;
                GlobalToLocal (&localClick);
                if (buttonControl&&PtInRect (localClick,&buttonRect))
                    if (TrackControl (buttonControl,localClick,nil))
                        if (messageRecipient) {
                            messageRecipient->ProcessEvent (generateButtonPushEvent (GetID(),0));
                        }
            }
            return true;
        }
        case keyDown:
        case autoKey: {
            int     keyCode = (theEvent->message&keyCodeMask)>>8;
            bool    good    = false;

            if (buttonKind == HY_BUTTON_OK) {
                good = ((keyCode==0x24) || (keyCode==0x4C));
            } else if (buttonKind == HY_BUTTON_CANCEL) {
                good = ((keyCode==0x35) || ((keyCode==0x2F)&&(theEvent->modifiers&cmdKey)));
            }

            if (good) {
                HiliteControl (buttonControl, kControlButtonPart);
                _PaintMe();
                //Draw1Control (buttonControl);
                if (messageRecipient) {
                    messageRecipient->ProcessEvent (generateButtonPushEvent (GetID(),0));
                }
                _PaintMe();
                //Draw1Control (buttonControl);
            }
            if (good) {
                return true;
            }
        }
        }

    return _HYPlatformComponent::_ProcessOSEvent (vEvent);
}
// EOF