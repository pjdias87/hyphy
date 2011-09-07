/*
    Sequence Panel for Mac OS API

    Sergei L. Kosakovsky Pond, May 2000-December 2002
*/

#include "errorfns.h"
#include "HYSequencePanel.h"
#include "HYUtils.h"
#include "HYEventTypes.h"
#include "HYPlatformWindow.h"

#include "ToolUtils.h"
#include "Appearance.h"


//__________________________________________________________________

void    _HYSequencePane::_Paint (Ptr p)
{
    long        saveBorder = settings.width & HY_COMPONENT_BORDER;
    settings.width -= saveBorder;
    _HYPlatformComponent::_Paint(p);
    settings.width += saveBorder;
    _HYRect*    destR = (_HYRect*)p;

    Rect        srcRect,
                destRect;

    destRect.right      = destR->right;
    destRect.bottom     = destR->bottom;
    if (HasHScroll()) {
        destRect.bottom-= HY_SCROLLER_WIDTH;
    }

    if (HasVScroll()) {
        destRect.right -= HY_SCROLLER_WIDTH;
    }

    destRect.left       = destR->left;
    destRect.top        = destR->top;
    _HYRect srcR        = _VisibleContents (p);
    srcRect.right       = srcR.right-srcR.left;
    srcRect.left        = 0;
    srcRect.bottom      = srcR.bottom-srcR.top;
    srcRect.top         = 0;

    RGBColor         white = {0xffff,0xffff,0xffff};
    RGBBackColor (&white);
    white.red = white.green = white.blue = 0;
    RGBForeColor (&white);
    LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
    CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(GetWindowPort(parentWindow)),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#else
    CopyBits (&(GrafPtr(thePane)->portBits),&(parentWindow->portBits),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#endif
    UnlockPixels (GetGWorldPixMap(thePane));
}

//__________________________________________________________________

bool _HYSequencePane::_ProcessOSEvent (Ptr vEvent)
{
    if (_HYPlatformComponent::_ProcessOSEvent (vEvent)) {
        return true;
    }
    EventRecord*    theEvent = (EventRecord*)vEvent;

    if (!active) {
        return false;
    }

    static  UInt32  lastClick = 0;

    static  int     lastH = 0,
                    lastV = 0;

    switch (theEvent->what) {
    case mouseDown: {
        Point p = theEvent->where;
        GlobalToLocal (&p);
        bool    dblClick = (theEvent->when-lastClick<GetDblTime())&&
                           (abs(p.h-lastH)<5)&&
                           (abs(p.v-lastV)<5);
        lastClick = theEvent->when;
        lastH = p.h;
        lastV = p.v;


        Point localClick = theEvent->where;
        bool  vertical;
        GlobalToLocal (&localClick);
        if ((!(theEvent->modifiers&shiftKey)||(theEvent->modifiers&cmdKey))) {
            forceUpdateForScrolling = true;
        }
        localClick.h -= rel.left;
        localClick.v -= rel.top;
        vertical = (localClick.h<headerWidth)&&(localClick.v>=(GetSlotHeight()+1));
        if (theEvent->modifiers&controlKey) {
            if ((vertical&&vselection.lLength)||((!vertical)&&selection.lLength)) {
                ProcessContextualPopUp (theEvent->where.h, theEvent->where.v);
                return true;
            }
        }
        if (vertical) {
            ProcessVSelectionChange (localClick.h,localClick.v,theEvent->modifiers&shiftKey,theEvent->modifiers&cmdKey, false, dblClick);
        } else {
            ProcessSelectionChange (localClick.h,localClick.v,theEvent->modifiers&shiftKey,theEvent->modifiers&cmdKey);
        }

        if (((vertical&&(vselection.lLength==1))||((!vertical)&&(selection.lLength==1)))&&StillDown()) {
            Point localClick = theEvent->where;
            GlobalToLocal (&localClick);
            localClick.h -= rel.left;
            localClick.v -= rel.top;
            long  lastClick = -2, slotHeight = GetSlotHeight(),
                  firstClick = (localClick.v-(GetSlotHeight()+1))/slotHeight,
                  wHeight = rel.bottom-rel.top-HY_SCROLLER_WIDTH,
                  originalStart = startRow,
                  originalSpan  = endRow-startRow;

            if (vertical) {
                PenMode (patXor);
            }
            forceUpdateForScrolling = true;

            while (WaitMouseUp()) {
                Point mousePos;
                GetMouse (&mousePos);
                mousePos.h -= rel.left;
                mousePos.v -= rel.top;
                if (vertical) {
                    if ((mousePos.v<(GetSlotHeight()+1))||(localClick.v!=mousePos.v)||(mousePos.v>wHeight)) {
                        localClick = mousePos;
                        if (mousePos.v>wHeight) {
                            // scroll down
                            if ((endRow<=speciesIndex.lLength)&&(vselection.lData[0]!=speciesIndex.lLength-1)) {
                                if (endRow-startRow<originalSpan) {
                                    continue;
                                }
                                startRow++;
                                endRow++;
                                _SetVScrollerPos(((double)MAX_CONTROL_VALUE*startRow)/
                                                 (speciesIndex.lLength-endRow+startRow+1));
                                BuildPane();
                                _MarkForUpdate();
                                lastClick = -2;
                            }
                            continue;
                        } else {
                            mousePos.v-=(GetSlotHeight()+1);
                            if (mousePos.v<=slotHeight) {
                                if (mousePos.v>=0) {
                                    if (mousePos.v<slotHeight/2) {
                                        mousePos.v = -1;
                                    } else {
                                        mousePos.v = 0;
                                    }
                                } else {
                                    // scroll up
                                    if (startRow>0) {
                                        startRow--;
                                        endRow--;
                                        _SetVScrollerPos(((double)MAX_CONTROL_VALUE*startRow)/(speciesIndex.lLength-endRow+startRow+1));
                                        BuildPane();
                                        _MarkForUpdate();
                                        lastClick = -2;
                                    }
                                    continue;
                                }
                            } else {
                                mousePos.v=(mousePos.v-(GetSlotHeight()+1))/slotHeight;
                            }
                        }

                        if ((mousePos.v<-1)||(mousePos.v>=(endRow-startRow))) {
                            continue;
                        }
                        if (mousePos.v!=lastClick) {
                            if (lastClick>=-1) {
                                lastClick = (GetSlotHeight()+1)+slotHeight*(lastClick+1)+rel.top+1;
                                MoveTo(rel.left+1,lastClick);
                                LineTo(rel.left+headerWidth-1,lastClick);
                            }
                            lastClick = mousePos.v;
                            if (lastClick+startRow!=firstClick+originalStart) {
                                mousePos.v = (GetSlotHeight()+1)+slotHeight*(lastClick+1)+rel.top+1;
                                MoveTo(rel.left+1,mousePos.v);
                                LineTo(rel.left+headerWidth-1,mousePos.v);
                            }
                        }
                    }
                } else {
                    if ((mousePos.h<headerWidth)||(localClick.h!=mousePos.h)||(mousePos.h>_HYCanvas::GetMaxW()-5)) {
                        ProcessSelectionChange (mousePos.h,mousePos.v,true,true,true);
                        localClick = mousePos;
                    }
                }
            }
            if  (vertical) {
                Rect invalRect = {rel.top+(GetSlotHeight()+1)+1,rel.left,rel.bottom-HY_SCROLLER_WIDTH,rel.left+headerWidth};
#ifdef TARGET_API_MAC_CARBON
                InvalWindowRect (parentWindow,&invalRect);
#else
                InvalRect (&invalRect);
#endif
                if ((localClick.h<headerWidth)&&(localClick.h>0)&&(lastClick>-2)) {
                    MoveSpecies (firstClick+originalStart,lastClick+startRow);
                }
                PenMode (patCopy);
            }
        }
        forceUpdateForScrolling = false;
        return true;
    }
    }
    return false;
}


//EOF