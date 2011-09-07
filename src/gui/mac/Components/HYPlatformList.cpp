/*
    List component for Mac OS API

    Sergei L. Kosakovsky Pond, May 2000-December 2002
*/

#include "errorfns.h"
#include "HYList.h"
#include "HYUtils.h"
#include "HYEventTypes.h"

#include "ToolUtils.h"
#include "Appearance.h"



_HYPlatformList::_HYPlatformList(void)
{
    listData = nil;
}

//__________________________________________________________________

_HYPlatformList::~_HYPlatformList(void)
{
    if (listData) {
        LDispose (listData);
    }
}

//__________________________________________________________________

void    _HYList::_Activate(void)
{
    if (listData) {
        LActivate (true, listData);
    }
    _HYPlatformComponent::_Activate();
}

//__________________________________________________________________

void    _HYList::_Deactivate(void)
{
    if (listData) {
        LActivate (false, listData);
    }
    _HYPlatformComponent::_Deactivate();
}

//__________________________________________________________________
void        _HYPlatformList::_SetVisibleSize (_HYRect rel)
{
    _HYList* theParent = (_HYList*) this;
    if (listData) {
        LDispose(listData);
    }

    SetWindowFont (fontID, theParent->GetFont().size, theParent->GetFont().style, true);
    Rect     listBox = {rel.top+1,rel.left,rel.bottom,rel.right-HY_SCROLLER_WIDTH},
             listDim = {0,0,theParent->ItemCount(),1};
    Point    cellSize = {0,0};
    listData = LNew(&listBox,&listDim,cellSize,0,(WindowPtr)theParent->parentWindow,true,false,false,true);
    checkPointer ((Ptr)listData);
    (*listData)->selFlags = lNoNilHilite|lUseSense;
    for (long k=0; k<theParent->ItemCount(); k++) {
        _SetItem (*theParent->GetItem(k),k);
    }

    SetWindowFont (0,0,0,false);
}

//__________________________________________________________________
void        _HYPlatformList::_Paint (Ptr p)
{
    _HYRect * relRec = (_HYRect*)p;
    Rect r;
    r.left = relRec->left;
    r.right = relRec->right;
    r.top = relRec->top;
    r.bottom = relRec->bottom;
    EraseRect (&r);
    if (listData) {
        _HYList* theParent = (_HYList*)this;
        SetWindowFont (fontID, theParent->GetFont().size, theParent->GetFont().style, true);
        LActivate (true,listData);
        LUpdate (GetGrayRgn(),listData);
        SetWindowFont (0,0,0,false);
    }
}

//__________________________________________________________________
void        _HYList::_EraseRect (_HYRect relRec)
{
    Rect r;
    r.left = relRec.left;
    r.right = relRec.right;
    r.top = relRec.top;
    r.bottom = relRec.bottom;
    EraseRect (&r);
}

//__________________________________________________________________
void        _HYPlatformList::_Update (Ptr p)
{
    _Paint (p);
}

//__________________________________________________________________
void        _HYPlatformList::_SetDimensions (_HYRect r, _HYRect rel)
{
    _HYList* theParent = (_HYList*) this;
    theParent->_HYPlatformComponent::_SetDimensions (r,rel);
    _SetVisibleSize (rel);
}

//__________________________________________________________________
void        _HYPlatformList::_SetFont (void)
{
    _HYList* theParent = (_HYList*)this;
    Str255 fName;
    StringToStr255 (theParent->GetFont().face,fName);
    GetFNum (fName,&fontID);
}

//__________________________________________________________________

bool _HYList::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord*    theEvent = (EventRecord*)vEvent;

    if (listData) {
        if(theEvent->what==mouseDown) {
            Point localClick = theEvent->where;
            GlobalToLocal (&localClick);
            if (LClick (localClick,theEvent->modifiers,listData)) {
                _CheckSelection();
                if (selection.lLength==1) {
                    SendDblClickEvent(selection.lData[0]);
                    return true;
                }
            } else {
                _CheckSelection();
            }
            if (messageRecipient) {
                messageRecipient->ProcessEvent(generateKeyboardFocusEvent(GetID()));
            }
            return true;
        } else if((theEvent->what==keyDown)||(theEvent->what==autoKey)) {
            unsigned char keyCode = (theEvent->message&keyCodeMask)>>8;
            Cell     thisCell= {0,0};
            if (selection.lLength==1) {
                if (keyCode==0x7E) { // up arrow
                    if (selection.lData[0]) {
                        thisCell.v = selection.lData[0];
                        LSetSelect (false,thisCell,listData);
                        selection.lData[0]--;
                        thisCell.v--;
                        LSetSelect (true,thisCell,listData);
                        LAutoScroll(listData);
                        _MarkForUpdate();
                        SendSelectionChange();
                        return true;
                    }
                } else if (keyCode==0x7D) { // down arrow
                    if (selection.lData[0]<items.lLength-1) {
                        thisCell.v = selection.lData[0];
                        LSetSelect (false,thisCell,listData);
                        selection.lData[0]++;
                        thisCell.v++;
                        LSetSelect (true,thisCell,listData);
                        LAutoScroll(listData);
                        _MarkForUpdate();
                        SendSelectionChange();
                        return true;
                    }
                }

            }
        }
    }
    return _HYPlatformComponent::_ProcessOSEvent (vEvent);
}

//__________________________________________________________________

void _HYPlatformList::_ToggleMultSelection (bool flag)
{
    if (listData) {
        if (flag) {
            (*listData)->selFlags&=0x7F;
        } else {
            (*listData)->selFlags|=0x80;
        }
    }
}

//__________________________________________________________________

void _HYPlatformList::_CheckSelection (void)
{
    if (listData) {
        _HYList * theParent = (_HYList*)this;
        _SimpleList newSelection;
        Cell        scrollCell = {0,0};
        while (LGetSelect (true,&scrollCell,listData)) {
            newSelection<<scrollCell.v;
            scrollCell.v++;
        }
        if (!theParent->GetSelection().Equal(newSelection)) {
            theParent->GetSelection().Clear();
            theParent->GetSelection().Duplicate (&newSelection);
            theParent->SendSelectionChange();
        }
    }
}

//__________________________________________________________________

void _HYPlatformList::_SetSelection (_SimpleList& nSel)
{
    if (listData) {
        _KillSelection();
        if (nSel.lLength) {
            Cell scrollCell = {0,0};
            for (long k=0; k<nSel.lLength; k++) {
                scrollCell.v = nSel.lData[k];
                LSetSelect (true,scrollCell,listData);
            }
            LAutoScroll (listData);
        }
    }
}

//__________________________________________________________________

void _HYPlatformList::_KillSelection (void)
{
    if (listData) {
        Cell        scrollCell = {0,0};
        while (LGetSelect (true,&scrollCell,listData)) {
            LSetSelect (false,scrollCell,listData);
            scrollCell.v++;
        }
    }
}

//__________________________________________________________________

void _HYPlatformList::_InsertItem (_String& item, long index)
{
    if (listData) {
        if (index<0) {
            index = 10000;
        }
        index = LAddRow(1,index,listData);
        Cell    thisCell = {index,0};
        LSetCell(item.sData,item.sLength,thisCell,listData);
    }
}

//__________________________________________________________________

void _HYPlatformList::_SetItem (_String& item, long index)
{
    if (listData) {
        Cell    thisCell = {index,0};
        LSetCell(item.sData,item.sLength,thisCell,listData);
    }
}

//__________________________________________________________________

void _HYList::_ToggleDrawing (bool onOff)
{
    if (listData) {
        LSetDrawingMode (onOff,listData);
    }
}

//__________________________________________________________________

void _HYPlatformList::_DeleteItem (long index)
{
    if (listData) {
        LDelRow(1,index,listData);
    }
    //_CheckSelection();
}


//EOF