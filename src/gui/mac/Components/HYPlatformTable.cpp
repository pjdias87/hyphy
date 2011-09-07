/*
    Table component for Mac OS API

    Sergei L. Kosakovsky Pond, May 2000-December 2002
*/

#include "errorfns.h"
#include "HYTableComponent.h"
#include "HYUtils.h"
#include "HYEventTypes.h"
#include "HYPlatformWindow.h"
#include "HYPlatformGraphicPane.h"

#include "ToolUtils.h"
#include "Appearance.h"

//__________________________________________________________________

#if TARGET_API_MAC_CARBON
#include <PMApplication.h>
extern PMPageFormat     gPageFormat;
extern PMPrintSettings  gPrintSettings;
extern Handle           gFlattenedFormat;
extern Handle           gFlattenedSettings;
#else
#include <Printing.h>
extern THPrint prRecHdl;
#endif


//__________________________________________________________________

extern      Cursor      hSizeCursor,
            editStateCursor;

extern      CCrsrHandle pickUpCursor,
            dropOffCursor;

extern      Pattern     penHatchPattern,
            vertPenHatchPattern;

extern      RGBColor    menuLine1,
            menuLine2;

extern      CIconHandle tablePDMenuIcon;


//__________________________________________________________________

_HYPlatformTable::_HYPlatformTable(void)
{
    fontID = 0;
    backPattern  = NewPixPat();
    backPattern2 = NewPixPat();
    cursorState  = false;
    editBox      = nil;
}

//__________________________________________________________________

_HYPlatformTable::~_HYPlatformTable(void)
{
    DisposePixPat (backPattern);
    DisposePixPat (backPattern2);
}

//__________________________________________________________________

void        _HYPlatformTable::_SetFont (void)
{
    _HYTable* parent = (_HYTable*)this;
    Str255 fName;
    StringToStr255 (parent->textFont.face,fName);
    short fNum=0;
    GetFNum (fName,&fNum);
    fontID = fNum;
}

//__________________________________________________________________
void        _HYTable::_HScrollTable (long h)
{
    if (h) {
        GrafPtr   savePort;
        GetPort   (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort   (GetWindowPort(parentWindow));
#else
        SetPort   (parentWindow);
#endif
        long        vsShift = ((settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0),
                    hsShift = ((settings.width&HY_COMPONENT_V_SCROLL)?HY_SCROLLER_WIDTH:0);
        EditBoxHandler  (-1,rel);
        if (abs(h)>(rel.right-rel.left)/2) {
            _Paint((Ptr)&rel);
        } else {
            RgnHandle updateRgn = NewRgn();
            checkPointer (updateRgn);
            Rect      scrollRect;
            _HYRect   paintRect;
            scrollRect.top = rel.top;
            scrollRect.bottom = rel.bottom-vsShift;
            scrollRect.right = rel.right-hsShift-1;
            scrollRect.left  = rel.left;
            paintRect.top = scrollRect.top;
            paintRect.bottom = rel.bottom;
            if (h>0) {
                ScrollRect (&scrollRect,-h,0,updateRgn);
                paintRect.right = scrollRect.right+hsShift;
                paintRect.left = scrollRect.right-h;
                hOrigin+=paintRect.left-rel.left;
                Paint((Ptr)&paintRect);
                hOrigin-=paintRect.left-rel.left;
            } else {
                ScrollRect (&scrollRect,-h,0,updateRgn);
                paintRect.left = scrollRect.left;
                paintRect.right = paintRect.left-h+hsShift;
                Paint((Ptr)&paintRect);
            }
            DisposeRgn (updateRgn);
        }
        SetPort (savePort);
    }
}

//__________________________________________________________________
void        _HYTable::_VScrollTable (long v)
{
    if (v) {
        GrafPtr   savePort;
        GetPort   (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort   (GetWindowPort(parentWindow));
#else
        SetPort   (parentWindow);
#endif
        long        vsShift = ((settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0),
                    hsShift = ((settings.width&HY_COMPONENT_V_SCROLL)?HY_SCROLLER_WIDTH:0);
        EditBoxHandler  (-1,rel);
        if (abs(v)>(rel.bottom-rel.top)/2) {
            _Paint((Ptr)&rel);
        } else {
            RgnHandle updateRgn = NewRgn();
            checkPointer (updateRgn);
            Rect      scrollRect;
            _HYRect   paintRect;
            scrollRect.left = rel.left;
            scrollRect.right = rel.right-hsShift;
            paintRect.left = scrollRect.left;
            paintRect.right = rel.right;
            scrollRect.top = rel.top;
            scrollRect.bottom  = rel.bottom-vsShift;
            if (v>0) {
                ScrollRect (&scrollRect,0,-v,updateRgn);
                paintRect.top = rel.bottom-vsShift-v-1;
                paintRect.bottom = rel.bottom;
                vOrigin+=paintRect.top-rel.top;
                Paint((Ptr)&paintRect);
                vOrigin-=paintRect.top-rel.top;
            } else {
                ScrollRect (&scrollRect,0,-v,updateRgn);
                paintRect.top = rel.top;
                paintRect.bottom = rel.top-v+vsShift+2;
                Paint((Ptr)&paintRect);
            }
            DisposeRgn (updateRgn);
        }
        SetPort (savePort);
    }
}


//__________________________________________________________________

void        _HYTable::_ComponentMouseExit (void)
{
    if (cursorState) {
        cursorState = 0;
#ifdef TARGET_API_MAC_CARBON
        Cursor arrow;
        SetCursor(GetQDGlobalsArrow(&arrow));
#else
        SetCursor (&qd.arrow);
#endif
    }
}
//__________________________________________________________________

bool        _HYTable::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord*    theEvent = (EventRecord*)vEvent;
    WindowPtr       dummy;

    static  UInt32  lastClick = 0;
    static  int     lastH = 0, lastV = 0;

    long            k,h,v,
                    vsShift = ((settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0),
                    hsShift = ((settings.width&HY_COMPONENT_V_SCROLL)?HY_SCROLLER_WIDTH:0);

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

        if ((selectionType&HY_TABLE_FOCUSABLE)&&messageRecipient&&((selectionType&HY_TABLE_IS_FOCUSED)==0)) {
            messageRecipient->ProcessEvent(generateKeyboardFocusEvent (GetID()));
            //return true;
        }

        long evtType = FindWindow (theEvent->where,&dummy);
        switch (evtType) {
        case inContent: {
            if (cursorState == HY_TABLE_SIZE_CURSOR) {
                EditBoxHandler (-1,rel);
                RgnHandle       dragRgn = NewRgn();
                checkPointer    (dragRgn);
                Rect  limits;
                limits.left = p.h;
                limits.right = limits.left+1;
                limits.top = rel.top;
                limits.bottom = rel.bottom-vsShift;
                RectRgn       (dragRgn, &limits);
                limits.right = rel.right-hsShift;

                p.h+=hOrigin;
                for (evtType = 0; evtType<horizontalSpaces.lLength-1; evtType++)
                    if (horizontalSpaces.lData[evtType]>p.h-2-rel.left) {
                        break;
                    }
                if (evtType) {
                    limits.left = rel.left+horizontalSpaces.lData[evtType-1]+3-hOrigin;
                } else {
                    limits.left = rel.left+3;
                }

                long dragRes = horizontalSpaces.lData[horizontalSpaces.lLength-1]-
                               rel.right+rel.left-hOrigin+hsShift;

                if (dragRes<p.h-limits.left) {
                    limits.left = p.h-dragRes;
                }
                p.h-=hOrigin;
                dragRes = DragGrayRgn(dragRgn,p,&limits,&limits,hAxisOnly,nil);
                DisposeRgn (dragRgn);
                cursorState = false;
#ifdef TARGET_API_MAC_CARBON
                Cursor arrow;
                SetCursor(GetQDGlobalsArrow(&arrow));
#else
                SetCursor (&qd.arrow);
#endif
                if (dragRes!=kMouseUpOutOfSlop) {
                    SetColumnSpacing (evtType,(short)(dragRes&0x0000ffff),true);
                    if (messageRecipient)
                        messageRecipient->ProcessEvent (generateTableResizeCEvent(GetID(),
                                                        evtType,(short)(dragRes&0x0000ffff)));
                }
            } else {
                if (editBox&&PtInRect (p,&textBoxRect)) {
                    TEClick (p,theEvent->modifiers&shiftKey, editBox);
                } else {
                    if (((k=FindClickedTableCell (p.h-rel.left,p.v-rel.top,h,v))>-1)&&
                            (p.v<rel.bottom-vsShift)&&(p.h<rel.right-hsShift)) {
                        if (dblClick) {
                            if (cellTypes.lData[k]&HY_TABLE_EDIT_TEXT) {
                                EditBoxHandler (k,rel);
                            } else if (messageRecipient) {
                                messageRecipient->ProcessEvent (generateTableDblClickEvent(GetID()));
                            }
                            break;
                        }
                        ModifySelection (h,v,theEvent->modifiers&shiftKey,theEvent->modifiers&controlKey, true);
                    }

                    if (k==-2)
                        // process pull-down
                    {
                        if (messageRecipient) {
                            messageRecipient->ProcessEvent (generateTablePullDownEvent(GetID(),v*horizontalSpaces.lLength+h,
                                                            (((long)theEvent->where.h)<<16)+theEvent->where.v));
                        }
                        //_HandlePullDown (sampleMenu,theEvent->where.h,theEvent->where.v,2);
                        break;
                    }

                    if ((p.h<rel.right-hsShift)&&(p.v<rel.bottom-vsShift))
                        if (StillDown()) {
                            PenState    savePen;
                            GetPenState (&savePen);
                            bool    first = true;
                            Point   newPt, oldPt = p;
                            long    t,t2 = -1;
                            FindClickedTableCell(p.h-rel.left,p.v-rel.top,h,t);

                            if (cursorState == HY_TABLE_DRAG_CURSOR) {
                                SetCCursor (dropOffCursor);
                                //PenMode      (patXor);
                                PenSize    (1,1);
                                while ( WaitMouseUp() ) {
                                    GetMouse(&newPt);
                                    if ((newPt.v>rel.bottom-vsShift)||(newPt.v<rel.top)) {
                                        if (t2>=0) {
                                            _HiliteRowForDrag (t2,t);
                                            t2 = -1;
                                        }
                                        h = verticalSpaces.lData[verticalSpaces.lLength-1]/verticalSpaces.lLength;
                                        _ScrollVPixels ((newPt.v<rel.top)?-h:h);
                                        continue;
                                    }
                                    if (newPt.h>rel.right-hsShift) {
                                        newPt.h=rel.right-hsShift;
                                    }
                                    if ( DeltaPoint(oldPt, newPt) ) {
                                        k = FindClickedTableCell(newPt.h-rel.left,newPt.v-rel.top,h,v);
                                        if ((v!=t2)&&(k>-1)) {
                                            if (t2>=0) {
                                                _HiliteRowForDrag (t2,t);
                                            }
                                            if ((v!=t)&&(!(cellTypes.lData[k]&HY_TABLE_CANTSELECT))) {
                                                _HiliteRowForDrag (v,t);
                                                t2 = v;
                                            } else {
                                                t2 = -1;
                                            }
                                        }
                                        oldPt = newPt;
                                    }
                                }
                                if (t2>=0) {
                                    _HiliteRowForDrag (t2,t);
                                }

                                SetCCursor (pickUpCursor);
                                if (t!=t2) {
                                    EditBoxHandler (-1,rel);
                                    DragRow (t,t2);
                                }
                            } else {
                                if (((selectionType&HY_TABLE_SINGLE_SELECTION)==0)
                                        &&((selectionType&HY_TABLE_NODRAG_SELECTION)==0)) {
                                    PenSize (3,3);
                                    PenMode (patXor);
                                    Pattern pPat;
                                    GetIndPattern (&pPat,0,4);
                                    PenPat  (&pPat);

                                    Rect    outlineRect,
                                            paintRect,
                                            clippingRect;

                                    clippingRect = HYRect2Rect (rel);

                                    outlineRect.right  = outlineRect.left = p.h;
                                    outlineRect.bottom = outlineRect.top  = p.v;

                                    while ( WaitMouseUp() ) {
                                        GetMouse(&newPt);
                                        if (newPt.v>rel.bottom-vsShift) {
                                            if (rel.bottom-rel.top-vsShift+vOrigin <  verticalSpaces.lData[verticalSpaces.lLength-1]-1) {
                                                long h = verticalSpaces.lData[verticalSpaces.lLength-1]/verticalSpaces.lLength;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                PenMode (patCopy);
                                                SetPenState (&savePen);
                                                _ScrollVPixels (h);
                                                PenSize (3,3);
                                                PenMode (patXor);
                                                PenPat  (&pPat);
                                                outlineRect.top -= h;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                p.v -= h;
                                                oldPt.v -= h;
                                                continue;
                                            }
                                            newPt.v=rel.bottom-vsShift;
                                        }
                                        if (newPt.h>rel.right-hsShift) {
                                            if (rel.right-rel.left-hsShift+hOrigin <  horizontalSpaces.lData[horizontalSpaces.lLength-1]-1) {
                                                long h = horizontalSpaces.lData[horizontalSpaces.lLength-1]/horizontalSpaces.lLength;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                PenMode (patCopy);
                                                SetPenState (&savePen);
                                                _ScrollHPixels (h);
                                                PenSize (3,3);
                                                PenMode (patXor);
                                                PenPat  (&pPat);
                                                outlineRect.left -= h;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                p.h -= h;
                                                oldPt.h -= h;
                                                continue;
                                            }
                                            newPt.h=rel.right-hsShift;
                                        }
                                        if (newPt.v<rel.top) {
                                            if (vOrigin>0) {
                                                long h = verticalSpaces.lData[verticalSpaces.lLength-1]/verticalSpaces.lLength;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                PenMode (patCopy);
                                                SetPenState (&savePen);
                                                _ScrollVPixels (-h);
                                                PenSize (3,3);
                                                PenMode (patXor);
                                                PenPat  (&pPat);
                                                outlineRect.top += h;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                p.v += h;
                                                oldPt.v += h;
                                                continue;
                                            }
                                            newPt.v=rel.top;
                                        }
                                        if (newPt.h<rel.left) {
                                            if (hOrigin>0) {
                                                long h = horizontalSpaces.lData[horizontalSpaces.lLength-1]/horizontalSpaces.lLength;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                PenMode (patCopy);
                                                SetPenState (&savePen);
                                                _ScrollHPixels (-h);
                                                PenSize (3,3);
                                                PenMode (patXor);
                                                PenPat  (&pPat);
                                                outlineRect.left += h;
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                                p.h += h;
                                                oldPt.h += h;
                                                continue;
                                            }
                                            newPt.h=rel.left;
                                        }

                                        if ( DeltaPoint(oldPt, newPt) ) {
                                            if (!first) {
                                                SectRect (&outlineRect,&clippingRect,&paintRect);
                                                FrameRect (&paintRect);
                                            }

                                            if (newPt.h > p.h) {
                                                outlineRect.right = newPt.h;
                                            } else {
                                                outlineRect.right = p.h;
                                                outlineRect.left = newPt.h;
                                            }
                                            if (newPt.v > p.v) {
                                                outlineRect.bottom = newPt.v;
                                            } else {
                                                outlineRect.bottom = p.v;
                                                outlineRect.top = newPt.v;
                                            }
                                            SectRect (&outlineRect,&clippingRect,&paintRect);
                                            FrameRect (&paintRect);
                                            first = false;
                                            oldPt = newPt;
                                        }
                                    }

                                    if (!first) {
                                        SectRect (&outlineRect,&clippingRect,&paintRect);
                                        FrameRect (&paintRect);
                                        _HYRect     outlineHRect;
                                        outlineHRect.left   = outlineRect.left;
                                        outlineHRect.right  = outlineRect.right+hsShift;
                                        outlineHRect.top    = outlineRect.top;
                                        outlineHRect.bottom = outlineRect.bottom+vsShift;
                                        long    hs,hf,vs,vf;
                                        hOrigin += outlineRect.left-rel.left;
                                        vOrigin += outlineRect.top-rel.top;
                                        GetDisplayRange (&outlineHRect,hs,hf,vs,vf);
                                        hOrigin -= outlineRect.left-rel.left;
                                        vOrigin -= outlineRect.top-rel.top;
                                        ExpungeSelection();
                                        if ((hf>=hs)||(vs>=vf)) {
                                            _SimpleList sel;
                                            if (selectionType&HY_TABLE_SEL_ROWS) {
                                                sel.RequestSpace (vf-vs+1);
                                                for (h=vs; h<=vf; h++) {
                                                    sel<<h;
                                                }
                                                SetRowSelection (sel);
                                            } else if (selectionType&HY_TABLE_SEL_COLS) {
                                                sel.RequestSpace (hf-hs+1);
                                                for (v=hs; v<=hf; h++) {
                                                    sel<<v;
                                                }
                                                SetColumnSelection(sel);

                                            } else {
                                                sel.RequestSpace ((vf-vs+1)*(hf-hs+1));
                                                for (h=hs; h<=hf; h++)
                                                    for (v=vs; v<=vf; v++) {
                                                        sel << v*horizontalSpaces.lLength + h;
                                                    }
                                                SetSelection (sel,true);
                                                _MarkCellsForUpdate (sel);
                                            }
                                        }
                                    }
                                }
                                SetPenState (&savePen);
                            }
                        }
                }
            }
        }
        }
        break;
    }
    case keyDown:
    case autoKey: {
        char    c = theEvent->message&charCodeMask,
                k = (theEvent->message&keyCodeMask)>>8;

        if (editBox) {
            if (theEvent->modifiers&cmdKey) {
                if ((c=='c')||(c=='C')) {
                    TECopy (editBox);
                } else if ((c=='x')||(c=='X')) {
                    TECut (editBox);
                } else if ((c=='v')||(c=='V')) {
                    TEPaste (editBox);
                }
            } else {
                if ((k==0x24)||(k==0x4C)) {
                    EditBoxHandler (-1,rel);
                } else {
                    TEKey (c,editBox);
                }
            }
            return true;
        } else
            switch (k) {
            case 0x7E: // up
                HandleKeyMove (0,theEvent->modifiers & cmdKey);
                break;
            case 0x7D: // down
                HandleKeyMove (1,theEvent->modifiers & cmdKey);
                break;
            case 0x7B: // left
                HandleKeyMove (2,theEvent->modifiers & cmdKey);
                break;
            case 0x7C: // right
                HandleKeyMove (3,theEvent->modifiers & cmdKey);
                break;
            }

        break;
    }
    default: {
        Point p = theEvent->where;
        GlobalToLocal (&p);
        if (cursorState == HY_TABLE_EDIT_CURSOR) {
            if (!(PtInRect (p,&textBoxRect)&&editBox)) {
#ifdef TARGET_API_MAC_CARBON
                Cursor arrow;
                SetCursor(GetQDGlobalsArrow(&arrow));
#else
                SetCursor (&qd.arrow);
#endif
                cursorState = 0;
            }
        } else {
            if (editBox&&PtInRect (p,&textBoxRect)) {
                SetCursor (&editStateCursor);
                cursorState = HY_TABLE_EDIT_CURSOR;
                break;
            }
        }
        bool  ch = (!(selectionType&HY_TABLE_DONT_SIZE))&&(CheckForHSizeLocation(p.h-rel.left))&&(p.v<rel.bottom-vsShift);
        if (ch&&(cursorState!=HY_TABLE_SIZE_CURSOR)) {
            cursorState = HY_TABLE_SIZE_CURSOR;
            SetCursor (&hSizeCursor);
        } else if ((!ch)&&(cursorState==HY_TABLE_SIZE_CURSOR)) {
            cursorState = 0;
#ifdef TARGET_API_MAC_CARBON
            Cursor arrow;
            SetCursor(GetQDGlobalsArrow(&arrow));
#else
            SetCursor (&qd.arrow);
#endif
        }
        if (selectionType & HY_TABLE_SEL_ROWS) {
            if (!ch) {
                k = FindClickedTableCell(p.h-rel.left,p.v-rel.top,h,v);
                if (k>=0) {
                    if ((cursorState != HY_TABLE_DRAG_CURSOR)&&(cursorState != HY_TABLE_EDIT_CURSOR)&&
                            ((selectionType&HY_TABLE_NODRAG_SELECTION)==0)) {
                        if (cellTypes.lData[k]&HY_TABLE_SELECTED) {
                            if (IsRowSelectionSimple()) {
                                SetCCursor (pickUpCursor);
                                cursorState = HY_TABLE_DRAG_CURSOR;
                            }
                        }
                    } else {
                        if (((selectionType&HY_TABLE_NODRAG_SELECTION)==0)&&(!(cellTypes.lData[k]&HY_TABLE_SELECTED))) {
#ifdef TARGET_API_MAC_CARBON
                            Cursor arrow;
                            SetCursor(GetQDGlobalsArrow(&arrow));
#else
                            SetCursor (&qd.arrow);
#endif
                            cursorState = 0;
                        }
                    }
                }
            }
        }
        //if (editBox)
        //TEIdle (editBox);
        return true;
    }
    }
    return _HYPlatformComponent::_ProcessOSEvent (vEvent);
}



//__________________________________________________________________
void        _HYTable::_Paint (Ptr p)
{
    _HYRect         *relRect    = (_HYRect*)p;
    GrafPtr         thisPort;
    RGBColor        oldColor,
                    oldBColor,
                    whiteC = {0xffff,0xffff,0xffff},
                    fillColor = {0x8fff,0x8fff,0x8fff};

    PixPatHandle    themeFill   = nil;
    RgnHandle       saveRgn     = NewRgn();

    checkPointer    (saveRgn);
    GetPort         (&thisPort);
    GetForeColor    (&oldColor);
    GetBackColor    (&oldBColor);
    GetClip         (saveRgn);

#ifdef OPAQUE_TOOLBOX_STRUCTS
    short   savedFace = GetPortTextFont (thisPort),
            savedSize = GetPortTextSize (thisPort);

    Style   savedStyle = GetPortTextFace (thisPort);
#else
    short   savedFace = thisPort->txFont,
            savedSize = thisPort->txSize;

    Style   savedStyle = thisPort->txFace;
#endif


    long    hs, // starting column
            hf, // ending column
            vs, // starting row
            vf, // ending row
            k,  // loop index
            t,  // aux variable
            t2,
            st; // a few more auxs

    bool    chop,
            chopv;

    GetDisplayRange (relRect, hs, hf, vs, vf);

    long            vsShift = ((settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0),
                    hsShift = ((settings.width&HY_COMPONENT_V_SCROLL)?HY_SCROLLER_WIDTH:0);


    _HYRect         saveRel;
    Rect            bRect;

    GWorldPtr       offScreenPtr = nil;
    CGrafPtr        savedCPtr;
    GDHandle        savedDevice;

    if ((vf-vs>2)&&(aquaInterfaceOn==false)) {
        bRect.left    = bRect.top = 0;
        bRect.right   = relRect->right-relRect->left-hsShift;
        bRect.bottom  = relRect->bottom-relRect->top-vsShift;
        short errCode = NewGWorld (&offScreenPtr,0,&bRect,0,GetMainDevice(),noNewDevice);
        if (errCode != noErr) {
            offScreenPtr = nil;
        } else {
            if (!LockPixels (GetGWorldPixMap(offScreenPtr))) {
                ReportWarning ("Failed to lock pixels in _HYTable::_Paint");
                DisposeGWorld (offScreenPtr);
                offScreenPtr = nil;
            } else {
                GetGWorld (&savedCPtr,&savedDevice);
                SetGWorld (offScreenPtr,nil);
                //SetPort    ((GrafPtr)offScreenPtr);
                //SetOrigin (clipRect.left,clipRect.top);
                saveRel = *relRect;
                relRect->bottom -= relRect->top;
                relRect->top = 0;
                relRect->right  -= relRect->left;
                relRect->left = 0;
            }
        }
    }

    Rect clipRect = {relRect->top,relRect->left,
                     relRect->bottom-vsShift,
                     relRect->right-hsShift
                    },
         anotherRect,
         clipRect2,
         clipRect3;

    /*if (settings.width & HY_COMPONENT_BORDER_L)
        clipRect.left ++;
    if (settings.width & HY_COMPONENT_BORDER_T)
        clipRect.top ++;
    if (settings.width & HY_COMPONENT_BORDER_B)
        if (!(settings.width&HY_COMPONENT_H_SCROLL))
            clipRect.bottom--;
    if (settings.width & HY_COMPONENT_BORDER_R)
        if (!(settings.width&HY_COMPONENT_V_SCROLL))
            clipRect.right--;*/

    ClipRect (&clipRect);

    anotherRect = clipRect;
    t = relRect->top-vOrigin;

    RGBForeColor (&whiteC);

    if (verticalSpaces.lLength)
        for (k=vs; k<=vf; k++) {
            anotherRect.top = k?verticalSpaces.lData[k-1]+t:relRect->top;
            anotherRect.bottom = verticalSpaces.lData[k]+t;
            if (cellTypes.lData[k*horizontalSpaces.lLength]&HY_TABLE_BEVELED) {
                FillCRect (&anotherRect,backPattern2);
                if ((k==vs)||(k<vf)) {
                    RGBForeColor (&menuLine2);
                    MoveTo (anotherRect.left,anotherRect.bottom-1);
                    LineTo (anotherRect.right,anotherRect.bottom-1);
                    RGBForeColor (&menuLine1);
                    MoveTo (anotherRect.left,anotherRect.bottom-2);
                    LineTo (anotherRect.right,anotherRect.bottom-2);
                    RGBForeColor (&whiteC);
                }
            } else {
                FillCRect (&anotherRect,backPattern);
                if ((k==vs)||(k<vf)) {
                    MoveTo (anotherRect.left,anotherRect.bottom-1);
                    LineTo (anotherRect.right,anotherRect.bottom-1);
                }
            }
        }


    if (hf<horizontalSpaces.lLength-1) {
        st = hf;
    } else {
        st = hf-1;
    }

    t = relRect->left-hOrigin-2;
    if ((selectionType & HY_TABLE_NO_COLS_LINES) == 0) {
        RGBForeColor (&menuLine1);
        for (k=hs; k<=st; k++) {
            t2 = t+horizontalSpaces.lData[k];
            MoveTo (t2,relRect->top);
            LineTo (t2,relRect->bottom);
        }
        RGBForeColor (&menuLine2);
        t++;
        for (k=hs; k<=st; k++) {
            t2 = t+horizontalSpaces.lData[k];
            MoveTo (t2,relRect->top);
            LineTo (t2,relRect->bottom);
        }
    }


    RGBColor     textRGB = {((long)textColor.R) * 256, ((long)textColor.G) * 256, ((long)textColor.B) * 256};
    RGBForeColor (&textRGB);

    TextFont (fontID);
    TextFace (textFont.style);
    TextSize (textFont.size);
    st = 0;
    if (verticalSpaces.lLength)
        for (k=vs; k<=vf; k++) {
            anotherRect.top = relRect->top-vOrigin+1;
            anotherRect.bottom = anotherRect.top+verticalSpaces.lData[k]-1;
            if (k) {
                anotherRect.top+=verticalSpaces.lData[k-1];
            }
            long    t3,st2,
                    w = anotherRect.bottom-anotherRect.top,
                    w2,
                    shift = (w-textFont.size)/2-1;

            if (anotherRect.bottom>relRect->bottom-vsShift) {
                anotherRect.bottom = relRect->bottom-vsShift;
                chopv = false;
            } else {
                chopv = true;
            }

            for (t2=hs; t2<=hf; t2++) {
                t3 = k*horizontalSpaces.lLength+t2;
                if (t3 == editCellID) {
                    continue;
                }
                anotherRect.left = relRect->left-hOrigin+1;
                anotherRect.right = anotherRect.left+horizontalSpaces.lData[t2]-1;
                if (t2) {
                    anotherRect.left+=horizontalSpaces.lData[t2-1];
                }
                clipRect2 = anotherRect;
                w2= anotherRect.right-anotherRect.left;
                chop = true;
                if (anotherRect.right>relRect->right-hsShift) {
                    anotherRect.right=relRect->right-hsShift;
                    chop = false;
                } else {
                    chop = true;
                }
                if ((t2==hs)||(k==vs)) {
                    SectRect (&anotherRect,&clipRect,&clipRect3);
                    ClipRect (&clipRect3);
                } else {
                    ClipRect (&anotherRect);
                }
                if (cellTypes.lData[t3]&HY_TABLE_SELECTED) {
                    if (!themeFill) {
                        themeFill = NewPixPat ();
                        checkPointer (themeFill);
                        Collection clcn = NewCollection();
                        GetTheme (clcn);
                        SInt32   itemSize = sizeof (RGBColor);
                        GetTaggedCollectionItem (clcn,kThemeHighlightColorTag,1,&itemSize,(void*)&fillColor);
                        //GetThemeBrushAsColor (kThemeBrushFocusHighlight,32,true,&fillColor);
                        MakeRGBPat (themeFill,&fillColor);
                        DisposeCollection (clcn);
                    }
                    if (chopv) {
                        anotherRect.bottom--;
                    }
                    if (chop) {
                        anotherRect.right-=2;
                        FillCRect (&anotherRect,themeFill);
                        anotherRect.right+=2;
                    } else {
                        FillCRect (&anotherRect,themeFill);
                    }
                    if (chopv) {
                        anotherRect.bottom++;
                    }
                }
                if (cellTypes.lData[t3]&HY_TABLE_ICON) {
                    _SimpleList     * cellList = (_SimpleList*)cellData.lData[t3];

                    if (w2-4>cellList->lData[1]) {
                        t3 = (w2-cellList->lData[1])/2;
                        clipRect2.left+=t3;
                    }
                    clipRect2.right = clipRect2.left+cellList->lData[1];
                    if (w-2>cellList->lData[2]) {
                        t3 = (w-cellList->lData[2])/2;
                        clipRect2.top+=t3;
                    }
                    clipRect2.bottom=clipRect2.top+cellList->lData[2];

                    if (cellList->lLength==3) {
                        PlotCIconHandle (&clipRect2, kAlignNone,kTransformNone, (CIconHandle)cellList->lData[0]);
                    } else {
                        if ((cellList->lData[3]==HY_TABLE_COLOR_BOX)||(cellList->lData[3]==HY_TABLE_COLOR_CIRCLE)) {
                            PixPatHandle clr = NewPixPat ();
                            checkPointer (clr);
                            _HYColor      c = LongToHYColor (cellList->lData[0]);
                            RGBColor      sysColor;
                            sysColor.red   = c.R*256;
                            sysColor.green = c.G*256;
                            sysColor.blue  = c.B*256;
                            MakeRGBPat (clr, &sysColor);
                            if (cellList->lData[3]==HY_TABLE_COLOR_BOX) {
                                FillCRect  (&clipRect2,clr);
                            } else {
                                RGBColor        trColor = {0,0,0},
                                                saveFGColor;

                                ::GetForeColor (&saveFGColor);


                                Rect           circRect = clipRect2;

                                RGBForeColor   (&trColor);
                                FrameArc       (&circRect,0,360);


                                InsetRect    (&circRect,1,1);
                                trColor.red = sysColor.red/2;
                                trColor.blue = sysColor.blue/2;
                                trColor.green = sysColor.green/2;
                                RGBForeColor   (&trColor);
                                FrameArc       (&circRect,0,360);

                                InsetRect    (&circRect,1,1);
                                trColor.red = sysColor.red/1.5;
                                trColor.blue = sysColor.blue/1.5;
                                trColor.green = sysColor.green/1.5;
                                RGBForeColor   (&trColor);
                                FrameArc       (&circRect,0,360);

                                InsetRect    (&circRect,1,1);
                                FillCArc     (&circRect,0,360,clr);
                                RGBForeColor   (&saveFGColor);
                            }
                            DisposePixPat (clr);
                        }
                    }

                    //PlotCIcon (&clipRect2, (CIconHandle)cellList->lData[0]);
                } else { // text
                    st2 = cellTypes.lData[t3]&HY_TABLE_STYLEMASK;
                    if (st!=st2) {
                        st = st2;
                        st2 = normal;
                        if (st&HY_TABLE_BOLD) {
                            st2 = bold;
                        }
                        if (st&HY_TABLE_ITALIC) {
                            st2 |= italic;
                        }
                        TextFace (st2);
                    }


                    _String  *thisCell = (_String*)cellData.lData[t3];
                    MoveTo   (anotherRect.left+textFont.size/3, anotherRect.top+shift+textFont.size);
                    DrawText (thisCell->sData,0,thisCell->sLength);

                    if (cellTypes.lData[t3]&HY_TABLE_PULLDOWN) {
                        clipRect2.right-=4;
                        clipRect2.left=clipRect2.right-tPDMw;
                        if (w-2>tPDMh) {
                            t3 = (w-tPDMh)/2;
                            clipRect2.top+=t3;
                        }
                        clipRect2.bottom=clipRect2.top+tPDMh;
                        PlotCIconHandle (&clipRect2, kAlignNone,kTransformNone, tablePDMenuIcon);
                        //PlotCIcon (&clipRect2, tablePDMenuIcon);
                    }
                }
            }
        }
    if (themeFill) {
        DisposePixPat (themeFill);
    }


    if (offScreenPtr) {
        //SetPort     (thisPort);
        SetGWorld (savedCPtr,savedDevice);
        whiteC.red = whiteC.green = whiteC.blue = 0xffff;
        RGBBackColor (&whiteC);
        whiteC.red = whiteC.green = whiteC.blue = 0;
        RGBForeColor (&whiteC);
        *relRect = saveRel;
        OffsetRect   (&clipRect,relRect->left, relRect->top);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        CopyBits (GetPortBitMapForCopyBits(offScreenPtr),GetPortBitMapForCopyBits(GetWindowPort(parentWindow)) ,&bRect,&clipRect,srcCopy,(RgnHandle)nil);
#else
        CopyBits (&(GrafPtr(offScreenPtr)->portBits),&(parentWindow->portBits),&bRect,&clipRect,srcCopy,(RgnHandle)nil);
#endif
        UnlockPixels (GetGWorldPixMap(offScreenPtr));
        DisposeGWorld (offScreenPtr);
    }
    RGBBackColor (&oldBColor);
    RGBForeColor (&oldColor);
    if (editBox) {
        Rect    temp;
        SectRect(&textBoxRect,&clipRect,&temp);
        if (!EmptyRect(&temp)) {
            ClipRect (&clipRect);
            if (settings.width&HY_COMPONENT_TRANSP_BG) {
                SetThemeWindowBackground (parentWindow,kThemeBrushWhite,false);
                TEUpdate (&textBoxRect,editBox);
                SetThemeWindowBackground (parentWindow,kThemeBrushDialogBackgroundActive,false);
            } else {
                TEUpdate (&textBoxRect,editBox);
            }
            InsetRect(&textBoxRect,1,1);
            DrawThemeFocusRect (&textBoxRect,true);
            InsetRect(&textBoxRect,-1,-1);
        }
    }
    SetClip (saveRgn);
    TextFont (savedFace);
    TextSize (savedSize);
    TextFace (savedStyle);
    DisposeRgn (saveRgn);

    _HYPlatformComponent::_Paint(p);
}

//__________________________________________________________________

void        _HYPlatformTable::_SetBackColor (_HYColor& c)
{
    RGBColor newBG;
    newBG.red = c.R*0x00ff;
    newBG.blue = c.B*0x00ff;
    newBG.green = c.G*0x00ff;
    MakeRGBPat (backPattern,&newBG);
}

//__________________________________________________________________

void        _HYPlatformTable::_SetBackColor2 (_HYColor& c)
{
    RGBColor newBG;
    newBG.red = c.R*0x00ff;
    newBG.blue = c.B*0x00ff;
    newBG.green = c.G*0x00ff;
    MakeRGBPat (backPattern2,&newBG);
}



//__________________________________________________________________

void        _HYPlatformTable::_CreateTextBox (_HYRect& tBox,_String& textIn)
{
    Rect      destRect;
    _HYTable* parent = (_HYTable*)this;

    textBoxRect.left    = tBox.left;
    textBoxRect.right   = tBox.right;
    textBoxRect.top     = tBox.top;
    textBoxRect.bottom  = tBox.bottom;
    destRect            = textBoxRect;
    destRect.right      += 10000;
    editBox             = TEStyleNew(&destRect,&textBoxRect);
    checkPointer        ((Ptr)editBox);
    TESetText           (textIn.sData,textIn.sLength,editBox);
    TESetSelect         (0,30000,editBox);
    TextStyle           tStyle;
    tStyle.tsFont       = fontID;
    tStyle.tsFace       = parent->textFont.style;
    tStyle.tsSize       = parent->textFont.size;
    tStyle.tsColor.red  = parent->textColor.R*0x00FF;
    tStyle.tsColor.green= parent->textColor.G*0x00FF;
    tStyle.tsColor.blue = parent->textColor.B*0x00FF;

    TESetStyle          (doAll,&tStyle,false,editBox);
    TEAutoView          (true,editBox);
    TECalText           (editBox);
    TEActivate          (editBox);
    //TEUpdate          (&textBoxRect, editBox);

}

//__________________________________________________________________

_String     _HYPlatformTable::_RetrieveTextValue (void)
{
    if (editBox) {
        long            textLen = (*editBox)->teLength;
        if (textLen) {
            CharsHandle     theText = TEGetText (editBox);
            _String         result  (textLen,false);
            BlockMove       (*theText,result.sData,textLen);
            return          result;
        }
    }
    return      empty;
}

//__________________________________________________________________

void        _HYPlatformTable::_KillTextBox (void)
{
    if (editBox) {
        TEDispose (editBox);
        editBox = nil;
    }
}

//__________________________________________________________________

Rect        _HYPlatformTable::_GetVisibleRowRect (long h)
{
    _HYTable*   parent = (_HYTable*)this;
    Rect        res;

    long        w = (parent->settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0;
    res.left   = parent->rel.left;
    res.right  = (parent->settings.width&HY_COMPONENT_H_SCROLL)?parent->rel.right-HY_SCROLLER_WIDTH:parent->rel.right;

    res.bottom = parent->verticalSpaces.lData[h]-parent->vOrigin+parent->rel.top;
    if (res.bottom>parent->rel.bottom-w) {
        res.bottom=parent->rel.bottom-w;
    }

    if (h) {
        res.top = parent->verticalSpaces.lData[h-1]-parent->vOrigin+parent->rel.top;
    } else {
        res.top = parent->rel.top-parent->vOrigin;
    }

    return res;
}

//__________________________________________________________________

void        _HYPlatformTable::_HiliteRowForDrag (long row, long old)
{
    Rect        cellRect = _GetVisibleRowRect (row);
    if (row<old) {
        cellRect.bottom = cellRect.top+2;
    } else {
        cellRect.top = cellRect.bottom-2;
    }
    InvertRect  (&cellRect);
}

//__________________________________________________________________

void        _HYTable::_MarkCellsForUpdate (_SimpleList& cells)
{
    GrafPtr   savePort;
    GetPort   (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort   (GetWindowPort(parentWindow));
#else
    SetPort   (parentWindow);
#endif
    long hs,hf,vs,vf,t,t2,k;
    GetDisplayRange (&rel, hs, hf, vs, vf);
    Rect  clipRect;

    clipRect.left = rel.left;
    clipRect.right = rel.right;
    if (settings.width&HY_COMPONENT_V_SCROLL) {
        clipRect.right -= HY_SCROLLER_WIDTH;
    }
    clipRect.top = rel.top;
    clipRect.bottom = rel.bottom;
    if (settings.width&HY_COMPONENT_H_SCROLL) {
        clipRect.bottom -= HY_SCROLLER_WIDTH;
    }

    for (k=0; k<cells.lLength; k++) {
        t2 = cells.lData[k]/horizontalSpaces.lLength;
        t = cells.lData[k]%horizontalSpaces.lLength;

        if ((t>=hs)&&(t<=hf)&&(t2<=vf)&&(t2>=vs)) {
            Rect invalRect;
            if (t) {
                invalRect.left = horizontalSpaces.lData[t-1];
            } else {
                invalRect.left = 0;
            }
            invalRect.right = horizontalSpaces.lData[t];
            if (t2) {
                invalRect.top = verticalSpaces.lData[t2-1];
            } else {
                invalRect.top = 0;
            }
            invalRect.bottom = verticalSpaces.lData[t2];
            OffsetRect (&invalRect, rel.left-hOrigin, rel.top-vOrigin);
            SectRect (&invalRect, &clipRect, &invalRect);
            if (!EmptyRect (&invalRect))
#ifdef TARGET_API_MAC_CARBON
                InvalWindowRect (parentWindow,&invalRect);
#else
                InvalRect (&invalRect);
#endif
        }
    }
    SetPort   (savePort);
}

//__________________________________________________________________

void        _HYTable::_MarkColumnForUpdate (long index)
{
    long hs,hf,vs,vf;
    GetDisplayRange (&rel, hs, hf, vs, vf);

    if ((index>=hs)&&(index<=hf)) {
        GrafPtr   savePort;
        GetPort   (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort   (GetWindowPort(parentWindow));
#else
        SetPort   (parentWindow);
#endif
        Rect  clipRect;

        clipRect.left = rel.left;
        clipRect.right = rel.right;
        if (settings.width&HY_COMPONENT_V_SCROLL) {
            clipRect.right -= HY_SCROLLER_WIDTH;
        }
        clipRect.top = rel.top;
        clipRect.bottom = rel.bottom;
        if (settings.width&HY_COMPONENT_H_SCROLL) {
            clipRect.bottom -= HY_SCROLLER_WIDTH;
        }

        Rect  invalRect;
        invalRect.bottom = clipRect.bottom;
        invalRect.top = clipRect.top;
        invalRect.left = index?horizontalSpaces.lData[index-1]:0;
        invalRect.right = horizontalSpaces.lData[index];
        OffsetRect (&invalRect, rel.left-hOrigin,0);
        SectRect (&invalRect, &clipRect, &invalRect);
        if (!EmptyRect (&invalRect))
#ifdef TARGET_API_MAC_CARBON
            InvalWindowRect (parentWindow,&invalRect);
#else
            InvalRect (&invalRect);
#endif
        SetPort   (savePort);
    }
}

//__________________________________________________________________

void        _HYTable::_MarkRowForUpdate (long index)
{
    long hs,hf,vs,vf;
    GetDisplayRange (&rel, hs, hf, vs, vf);

    if ((index>=vs)&&(index<=vf)) {
        GrafPtr   savePort;
        GetPort   (&savePort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort   (GetWindowPort(parentWindow));
#else
        SetPort   (parentWindow);
#endif
        Rect  clipRect;

        clipRect.left = rel.left;
        clipRect.right = rel.right;
        if (settings.width&HY_COMPONENT_V_SCROLL) {
            clipRect.right -= HY_SCROLLER_WIDTH;
        }
        clipRect.top = rel.top;
        clipRect.bottom = rel.bottom;
        if (settings.width&HY_COMPONENT_H_SCROLL) {
            clipRect.bottom -= HY_SCROLLER_WIDTH;
        }

        Rect  invalRect;
        invalRect.right = clipRect.right;
        invalRect.left = clipRect.left;
        invalRect.top = index?verticalSpaces.lData[index-1]:0;
        invalRect.bottom = verticalSpaces.lData[index];
        OffsetRect (&invalRect, 0, rel.top-vOrigin);
        SectRect (&invalRect, &clipRect, &invalRect);
#ifdef TARGET_API_MAC_CARBON
        InvalWindowRect (parentWindow,&invalRect);
#else
        InvalRect (&invalRect);
#endif
        SetPort   (savePort);
    }
}

//__________________________________________________________________

void        _HYTable::_MarkCellForUpdate (long index)
{
    _SimpleList     dummy (index);
    _MarkCellsForUpdate (dummy);
}

//__________________________________________________________________

void        _HYTable::_IdleHandler (void)
{
    if (editBox) {
        TEIdle (editBox);
    }
}

//__________________________________________________________________

void        _HYTable::_FocusComponent (void)
{
}

//__________________________________________________________________
long        _HYTable::_HandlePullDown (_List& data, long h, long v, long currentS)
{
    if (data.lLength) {
        return HandlePullDownWithFont (data,h,v,currentS,textFont.face,textFont.size);
    }
    return -1;
}

//__________________________________________________________________

void        _HYTable::_ScrollVPixels (long offset)
{
    long     voff = ((settings.width&HY_COMPONENT_H_SCROLL)?HY_SCROLLER_WIDTH:0);
    offset = offset/(_Parameter)(GetMaxH()- rel.bottom+rel.top+1-voff)*MAX_CONTROL_VALUE;
    ProcessEvent (generateScrollEvent(0,offset));
    _SetVScrollerPos((double)MAX_CONTROL_VALUE*vOrigin/(verticalSpaces.lData[verticalSpaces.lLength-1]-vSize+voff));
}

//__________________________________________________________________

void        _HYTable::_ScrollHPixels (long offset)
{
    long     hoff = ((settings.width&HY_COMPONENT_V_SCROLL)?HY_SCROLLER_WIDTH:0);
    offset = offset/(_Parameter)(GetMaxW()- rel.right+rel.bottom+1-hoff)*MAX_CONTROL_VALUE;
    ProcessEvent (generateScrollEvent(offset,0));
    _SetHScrollerPos((double)MAX_CONTROL_VALUE*hOrigin/(horizontalSpaces.lData[horizontalSpaces.lLength-1]-hSize+hoff));
}

//__________________________________________________________________

void        _HYTable::_ScrollRowIntoView (long index)
{
    if ((index>=0)&&(index<verticalSpaces.lLength)) {
        long hs, hf, vs, vf;
        GetDisplayRange (&rel,hs,hf,vs,vf);
        if ((index>vf)||(index<vs)) {
            _ScrollVPixels ((index?verticalSpaces.lData[index-1]:0)-vOrigin);
        }
    }
}

//__________________________________________________________________

void        _HYTable::_PrintTable (_SimpleList& columns, _SimpleList& rows, _HYTable* ch)
{

    if ((columns.lLength == 0)||(rows.lLength == 0)) {
        return;
    }

    GrafPtr     savePort;
#ifdef      TARGET_API_MAC_CARBON
    PMRect prRect;
#else
    TPrStatus   prStatus;
    TPPrPort    printPort;
    OSErr       err;
#endif

#ifdef TARGET_API_MAC_CARBON
    OSStatus theStatus;
    Boolean isAccepted;

    PMPrintSession hyPS;
    theStatus = PMCreateSession(&hyPS);
    if (theStatus != noErr) {
        return;
    }
#endif

    if (!InitPrint(hyPS)) {
        _String errMsg ("Could not initialize printing variables.");
        WarnError (errMsg);
        terminateExecution = false;
        return;
    }

    GetPort(&savePort);

#ifdef TARGET_API_MAC_CARBON
    if (gPrintSettings != kPMNoPrintSettings) {
        theStatus = PMSessionValidatePrintSettings(hyPS,gPrintSettings, kPMDontWantBoolean);
    } else {
        theStatus = PMCreatePrintSettings(&gPrintSettings);

        if ((theStatus == noErr) && (gPrintSettings != kPMNoPrintSettings)) {
            theStatus = PMSessionDefaultPrintSettings(hyPS,gPrintSettings);
        }
    }

    if (theStatus == noErr) {
        theStatus = PMSessionPrintDialog(hyPS,gPrintSettings, gPageFormat, &isAccepted);

        if (isAccepted) {
            theStatus = PMGetAdjustedPageRect(gPageFormat, &prRect);
            if (theStatus != noErr) {
                return;
            }

            theStatus = PMSessionBeginDocument(hyPS,gPrintSettings, gPageFormat);
            if (theStatus != noErr) {
                return;
            }

            long     printW    = prRect.right-prRect.left-2,
                     printH    = prRect.bottom-prRect.top-2;

            UInt32   startPage,
                     endPage;

            PMGetFirstPage (gPrintSettings,&startPage);
            PMGetLastPage  (gPrintSettings,&endPage);
#else
    PrOpen();
    if (err=PrError()) {
        _String errMsg ("Could not print the table. Error Code:");
        errMsg = errMsg & (long)err;
        WarnError (errMsg);
        terminateExecution = false;
        return;
    }

    if (PrJobDialog(prRecHdl)) {
        printPort = PrOpenDoc(prRecHdl, nil, nil);
        SetPort((GrafPtr)printPort);
        long     printW = (*prRecHdl)->prInfo.rPage.right-2,
                 printH = (*prRecHdl)->prInfo.rPage.bottom-2,
                 startPage = (*prRecHdl)->prJob.iFstPage,
                 endPage = (*prRecHdl)->prJob.iLstPage;
#endif
            long
            tW = 0,
            cC = 0,
            cE = 0,
            cP = 1,
            t,
            i,
            pH = ch?ch->GetRowSpacing (0):0;


            if (ch) {
                printH -= pH;
                if (printH <= 0) {
                    _String errMsg ("Table header is too tall to fit on the page.");
                    WarnError (errMsg);
                    terminateExecution = false;
                    return;
                }
            }

            _HYRect  relDim = {0,0,0,0,HY_COMPONENT_NO_SCROLL};

            if (ch)
                for (i=0; i<columns.lLength; i++) {
                    tW += ch->GetColumnSpacing (columns.lData[i]);
                }
            else
                for (i=0; i<columns.lLength; i++) {
                    tW += GetColumnSpacing (columns.lData[i]);
                }

            relDim.left    = relDim.right   = tW > printW ? printW : tW;

            while (cP < startPage) {
#ifdef TARGET_API_MAC_CARBON
                theStatus = PMSessionBeginPage(hyPS,gPageFormat, NULL);
                if (theStatus != noErr) {
                    break;
                }
#else
                PrOpenPage      (printPort, nil);
#endif

                t = 0;
                while (cC < rows.lLength) {
                    i = GetRowSpacing (rows.lData[cC]);
                    if ( t+i > printH) {
                        cP ++;
                        if (i > printH) {
                            cC++;
                        }
                        break;
                    } else {
                        t += i;
                        cC ++;
                    }
                }
#if TARGET_API_MAC_CARBON
                theStatus = PMSessionEndPage(hyPS);
#else
                PrClosePage(printPort);
#endif
            }

            cE = cC;

            for (long pageCount = startPage; pageCount<=endPage && (cC < rows.lLength); pageCount++) {
                t = 0;
                while (cE < rows.lLength) {
                    i = GetRowSpacing (rows.lData[cE]);
                    if ( t+i > printH) {
                        if (i > printH) {
                            t = printH;
                            cE++;
                        }
                        break;
                    } else {
                        t += i;
                        cE ++;
                    }
                }

                relDim.top = relDim.bottom = t+pH;

#ifndef TARGET_API_MAC_CARBON
                _HYTable        *thisPage = new _HYTable (relDim,(Ptr)printPort, cE-cC+(ch?1:0),columns.lLength,20,20,HY_TABLE_STATIC_TEXT);
#else
                GrafPtr         prPortPtr;
                PMSessionGetGraphicsContext (hyPS,NULL,(void**)&prPortPtr);
                _HYTable        *thisPage = new _HYTable (relDim,(Ptr)prPortPtr, cE-cC+(ch?1:0),columns.lLength,20,20,HY_TABLE_STATIC_TEXT);
#endif
                checkPointer    (thisPage);
#ifdef TARGET_API_MAC_CARBON
                theStatus = PMSessionBeginPage(hyPS,gPageFormat, NULL);
                if (theStatus != noErr) {
                    break;
                }
#else
                PrOpenPage      (printPort, nil);
#endif

                thisPage->SetFont       (textFont);
                thisPage->SetBackColor  (backColor);
                thisPage->SetBackColor2 (backColor2);
                thisPage->SetTextColor  (textColor);

                if (ch)
                    for (i=0; i<columns.lLength; i++) {
                        thisPage->SetColumnSpacing (i,ch->GetColumnSpacing (columns.lData[i])-20, false);
                    }
                else
                    for (i=0; i<columns.lLength; i++) {
                        thisPage->SetColumnSpacing (i,GetColumnSpacing (columns.lData[i])-20, false);
                    }

                t = 0;
                if (ch) {
                    thisPage->SetRowSpacing (0,pH-20,false);
                    for (i=0; i<columns.lLength; i++) {
                        BaseRef cellData = ch->GetCellData(columns.lData[i],0);
                        cellData->nInstances++;
                        thisPage->SetCellData(cellData,0,i,ch->cellTypes.lData[columns.lData[i]]&HY_TABLE_DESELECT,false);
                    }
                    t = 1;
                }

                for (cP = cC; cP < cE; cP++,t++) {
                    long     rI = rows.lData[cP];
                    thisPage->SetRowSpacing (t,GetRowSpacing(rI)-20,false);
                    for (i=0; i<columns.lLength; i++) {
                        BaseRef cellData = GetCellData(columns.lData[i],rI);
                        cellData->nInstances++;
                        thisPage->SetCellData(cellData,t,i,cellTypes.lData[rI*horizontalSpaces.lLength+columns.lData[i]]&HY_TABLE_DESELECT,false);
                    }
                }
                _HYRect relDim2 = relDim;
                relDim2.left = relDim2.top = 1;
                relDim2.right ++;
                relDim2.bottom ++;
                thisPage->_Paint ((Ptr)&relDim2);
                PenSize (1,1);
                RGBColor blk = {0,0,0};
                RGBForeColor (&blk);
                Rect         relDim22 = HYRect2Rect (relDim2);
                FrameRect (&relDim22);
#if TARGET_API_MAC_CARBON
                theStatus = PMSessionEndPage(hyPS);
#else
                PrClosePage(printPort);
#endif
                DeleteObject (thisPage);
                cC = cE;
            }
#ifdef TARGET_API_MAC_CARBON
            theStatus = PMSessionEndDocument(hyPS);
            SetPort(savePort);
            if (theStatus == noErr) {
                if (gFlattenedFormat != NULL) {
                    DisposeHandle(gFlattenedFormat);
                    gFlattenedFormat = NULL;
                }

                theStatus = PMFlattenPageFormat(gPageFormat, &gFlattenedFormat);
            }

            if (theStatus == noErr) {
                if (gFlattenedSettings != NULL) {
                    DisposeHandle(gFlattenedSettings);
                    gFlattenedSettings = NULL;
                }

                theStatus = PMFlattenPrintSettings(gPrintSettings, &gFlattenedSettings);
            }

            if (gPageFormat != kPMNoPageFormat) {
                theStatus = PMRelease(gPageFormat);
                gPageFormat = kPMNoPageFormat;
            }

            if (gPrintSettings != kPMNoPrintSettings) {
                theStatus = PMRelease(gPrintSettings);
                gPrintSettings = kPMNoPrintSettings;
            }

            theStatus = PMRelease(hyPS);

#else
            PrCloseDoc(printPort);
            if (((*prRecHdl)->prJob.bJDocLoop = bSpoolLoop) && (!PrError() ) ) {
                PrPicFile(prRecHdl, nil, nil, nil, &prStatus);
            }
#endif
        }
#ifdef TARGET_API_MAC_CARBON
        else {
            theStatus = PMRelease(hyPS);
        }
#endif

#ifdef TARGET_API_MAC_CARBON
    }
#else
        PrClose();
        SetPort(savePort);
#endif

}

//EOF