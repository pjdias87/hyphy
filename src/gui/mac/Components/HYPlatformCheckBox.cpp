/*
    Check box for Mac OS.

    Sergei L. Kosakovsky Pond, May 2000-December 2002
*/

#include "HYLabel.h"
#include <ControlDefinitions.h>


//__________________________________________________________________

void        _HYCheckbox::_Activate (void)
{
    if (!activationFlag)
        if (isEnabled&&checkboxControl) {
            HiliteControl (checkboxControl,0);
            //Draw1Control  (checkboxControl);
            _PaintMe();
        }

    _HYPlatformComponent::_Activate();
}

//__________________________________________________________________

void        _HYCheckbox::_Deactivate (void)
{
    if (activationFlag)
        if (isEnabled&&checkboxControl) {
            HiliteControl (checkboxControl, kControlInactivePart);
            //Draw1Control  (checkboxControl);
            _PaintMe();
        }

    _HYPlatformComponent::_Deactivate();
}

//__________________________________________________________________

bool _HYCheckbox::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord*    theEvent = (EventRecord*)vEvent;
    WindowPtr       dummy;
    if (checkboxControl)
        switch (theEvent->what) {
        case mouseDown: {
            long evtType = FindWindow (theEvent->where,&dummy);
            if ((evtType == inContent)&&(isEnabled)) {
                Point localClick = theEvent->where;
                GlobalToLocal (&localClick);
                if (checkboxControl&&(PtInRect (localClick,&checkboxRect)))
                    if (!(isRadio&&checkState))
                        if (TrackControl (checkboxControl,localClick,nil)) {
                            SetState (!checkState, true);
                        }
            }
            return true;
        }
        }

    return _HYPlatformComponent::_ProcessOSEvent (vEvent);
}

//__________________________________________________________________

_HYRect     _HYCheckbox::_SuggestDimensions (void)
{
    _HYRect res = _HYLabel::_SuggestDimensions();
    if (aquaInterfaceOn) {
        res.left += checkSpacing*2 + 16;
    } else {
        res.left += checkSpacing*2 + 15;
    }

    res.right = res.left;
    return res;
}

//__________________________________________________________________
void        _HYPlatformCheckbox::_SetVisibleSize (_HYRect rel)
{
    _HYCheckbox * theParent = (_HYCheckbox*) this;

    theParent->labelRect.left= rel.left;
    theParent->labelRect.top = rel.top;

    _HYRect s = theParent->_SuggestDimensions();

    theParent->labelRect.right =  theParent->labelRect.left+s.right;
    theParent->labelRect.bottom = theParent->labelRect.top+s.bottom;

    AlignRectangle (rel, theParent->labelRect, theParent->GetAlignFlags());

    checkboxRect.left = theParent->labelRect.left + theParent->checkSpacing;
    checkboxRect.bottom = theParent->labelRect.bottom;
    if (aquaInterfaceOn) {
        checkboxRect.right = checkboxRect.left+16;
        checkboxRect.top = checkboxRect.bottom-15;

    } else {
        checkboxRect.right = checkboxRect.left+15;
        checkboxRect.top = checkboxRect.bottom-15;
    }

    if (aquaInterfaceOn) {
        theParent->labelRect.left += 16+2*theParent->checkSpacing;
    } else {
        theParent->labelRect.left += 15+2*theParent->checkSpacing;
    }

    if (checkboxControl) {
        MoveControl (checkboxControl,checkboxRect.left,checkboxRect.top);
    } else {
        GrafPtr      savePort;
        GetPort (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort (GetWindowPort(theParent->parentWindow));
#else
        SetPort (theParent->parentWindow);
#endif
        checkboxControl = NewControl (theParent->parentWindow,&checkboxRect,"\p",true,theParent->checkState,0,1,
                                      isRadio?kControlRadioButtonProc:kControlCheckBoxProc,0);
        SetPort (savePort);
    }
}

//__________________________________________________________________

void        _HYPlatformCheckbox::_Enable (bool e)
{
    if (checkboxControl) {
        if (e) {
            HiliteControl (checkboxControl,0);
        } else {
            HiliteControl (checkboxControl,kControlInactivePart);
        }

        _HYCheckbox * theParent = (_HYCheckbox*) this;
        if (e&&theParent->activationFlag)
            //Draw1Control  (checkboxControl);
        {
            _PaintMe();
        }
    }
}

//__________________________________________________________________
void        _HYPlatformCheckbox::_Update (Ptr p)
{
    _Paint (p);
}

//__________________________________________________________________
void        _HYPlatformCheckbox::_Paint (Ptr p)
{
    _HYCheckbox *theParent = (_HYCheckbox*) this;
    theParent->_HYPlatformLabel::_Paint(p);
    if (checkboxControl) {
        _PaintMe();
    }
}

//__________________________________________________________________
void        _HYPlatformCheckbox::_PaintMe (void)
{
    _HYCheckbox * theParent = (_HYCheckbox*)this;
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
        newBG.red = theParent->GetBackColor().R*256;
        newBG.blue = theParent->GetBackColor().B*256;
        newBG.green = theParent->GetBackColor().G*256;
        RGBBackColor (&newBG);
    }

    Draw1Control (checkboxControl);
    if (drawBk) {
        RGBBackColor (&saveColor);
    }

    SetPort     (thisPort);
}

//__________________________________________________________________
_HYPlatformCheckbox::_HYPlatformCheckbox (bool isR)
{
    checkboxControl = nil;
    isRadio = isR;
    checkboxRect = (Rect) {
        0,0,100,100
    };
}

//__________________________________________________________________
_HYPlatformCheckbox::~_HYPlatformCheckbox (void)
{
    if (checkboxControl) {
        DisposeControl (checkboxControl);
        checkboxControl = nil;
    }
}

//__________________________________________________________________
void    _HYPlatformCheckbox::_SetState (bool v)
{
    if (checkboxControl) {
        SetControlValue (checkboxControl, v);
    }
}


//EOF