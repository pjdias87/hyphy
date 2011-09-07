/*
    Button component for Mac OS.

    Sergei L. Kosakovsky Pond, May 2000.
*/

#include "errorfns.h"
#include "HYPullDown.h"
#include "HYUtils.h"
#include "HYEventTypes.h"

#include "ToolUtils.h"


long    menuIDCounter = 20000;


//__________________________________________________________________

_HYPlatformPullDown::_HYPlatformPullDown(void)
{
    myID = menuIDCounter++;
    myMenu = NewMenu(myID,"\p");
    backFill = NewPixPat();
    if ((!backFill)||(!myMenu)) {
        warnError (-108);
    }
    InsertMenu (myMenu,hierMenu);
    RGBColor  wht = {0xffff,0xffff,0xffff};
#ifdef TARGET_API_MAC_CARBON
    EnableMenuItem (myMenu,0);
#else
    EnableItem (myMenu,0);
#endif
    MakeRGBPat (backFill,&wht);
    selection = 0;

}

//__________________________________________________________________

_HYPlatformPullDown::~_HYPlatformPullDown(void)
{
    if (myMenu) {
        DeleteMenu (myID);
        DisposeMenu (myMenu);
    }
    if (backFill) {
        DisposePixPat (backFill);
    }
}

//__________________________________________________________________
void        _HYPlatformPullDown::_AddMenuItem   (_String& newItem, long index)
{
    if (!myMenu) {
        return;
    }

    if (index<=0) {
        index = 10000;
    }

    if (!newItem.Equal(&menuSeparator)) {
        Str255          menuText;
        bool            disableMe = false;
        if (newItem.sLength && newItem.sData[0] == '(') {
            _String         skipString (newItem,1,-1);
            StringToStr255  (skipString,menuText);
            disableMe = true;
        } else {
            StringToStr255 (newItem,menuText);
        }

        InsertMenuItem (myMenu,"\pa",index);

        long            mc = ((_HYPullDown*)this)->MenuItemCount();

        if (index<mc-1) {
            index++;
        } else {
            index = mc;
        }

        if (menuText[0]>250) {
            menuText[0] = 250;
        }

        SetMenuItemText (myMenu, index, menuText);
        if (disableMe) {
            DisableMenuItem (myMenu, index);
        }
    } else {
        InsertMenuItem (myMenu,"\p(-",index);
    }
}

//__________________________________________________________________
void        _HYPlatformPullDown::_SetMenuItem   (_String& newItem, long index)
{
    if (!myMenu) {
        return;
    }
    index++;
    if (!newItem.Equal(&menuSeparator)) {
        Str255 menuText;
        StringToStr255 (newItem,menuText);
        SetMenuItemText (myMenu,index,menuText);
    } else {
        SetMenuItemText (myMenu,index,"\p(-");
    }
}

//__________________________________________________________________
void        _HYPlatformPullDown::_MarkItem      (long index, char mark)
{
    if (!myMenu) {
        return;
    }

    switch (mark) {
    case HY_PULLDOWN_CHECK_MARK:
        mark = checkMark;
        break;

    case HY_PULLDOWN_DIAMOND_MARK:
        mark = diamondMark;
        break;

    case HY_PULLDOWN_BULLET_MARK:
        mark = 0xA5;
        break;

    default:
        mark = noMark;
    }
    SetItemMark (myMenu,index+1,mark);
}

//__________________________________________________________________
char        _HYPlatformPullDown::_ItemMark      (long index)
{
    if (!myMenu) {
        return 0;
    }

    short   mark;
    GetItemMark (myMenu, index+1,&mark);

    switch (mark) {
    case checkMark:
        return HY_PULLDOWN_CHECK_MARK;

    case diamondMark:
        return HY_PULLDOWN_DIAMOND_MARK;

    case 0xA5:
        return HY_PULLDOWN_BULLET_MARK;

    }
    return HY_PULLDOWN_NO_MARK;
}

//__________________________________________________________________
void        _HYPlatformPullDown::_DeleteMenuItem  (long index)
{
    if (!myMenu) {
        return;
    }
    DeleteMenuItem (myMenu,index+1);
}

//__________________________________________________________________

void        _HYPlatformPullDown::_SetBackColor (_HYColor& c)
{
    RGBColor newBG;
    newBG.red = c.R*256;
    newBG.blue = c.B*256;
    newBG.green = c.G*256;
    MakeRGBPat (backFill,&newBG);

}

//__________________________________________________________________
long        _HYPlatformPullDown::_GetSelection (void)
{
    return selection;
}

//__________________________________________________________________
void        _HYPlatformPullDown::_Duplicate (Ptr p)
{
    _HYPullDown * theSource = (_HYPullDown*) p;
    myMenu                  = theSource->myMenu;
    myID                    = theSource->myID;
    selection               = theSource->selection;
    backFill                = theSource->backFill;
    theSource->backFill     = nil;
    theSource->myMenu       = nil;
}

//__________________________________________________________________
void        _HYPlatformPullDown::_Update (Ptr p)
{
    _Paint (p);
}

//__________________________________________________________________
void        _HYPlatformPullDown::_SetDimensions (_HYRect r, _HYRect rel)
{
    _HYPullDown* theParent = (_HYPullDown*) this;
    theParent->_HYPlatformComponent::_SetDimensions (r,rel);
    _SetVisibleSize (rel);
}

//__________________________________________________________________
void        _HYPlatformPullDown::_SetVisibleSize (_HYRect rel)
{
    _HYPullDown* theParent = (_HYPullDown*) this;
    menuRect.left   = rel.left+3;
    menuRect.bottom = rel.bottom;
    menuRect.right  = rel.right-3;
    menuRect.top    = rel.top;
    if (myMenu) {
        CalcMenuSize (myMenu);
        if (menuRect.bottom-menuRect.top>20) {
            menuRect.bottom = menuRect.top+20;
        }
#ifdef OPAQUE_TOOLBOX_STRUCTS
        if (menuRect.right-menuRect.left>GetMenuWidth (myMenu)+22) {
            menuRect.right = menuRect.left+GetMenuWidth (myMenu) +22;
        }
#else
        if (menuRect.right-menuRect.left>(*myMenu)->menuWidth+22) {
            menuRect.right = menuRect.left+(*myMenu)->menuWidth+22;
        }
#endif
    }

    AlignRectangle (rel, menuRect, theParent->GetAlignFlags());
}
//__________________________________________________________________
void        _HYPlatformPullDown::_EnableItem (long theItem, bool toggle)
{
    if (myMenu)
        if (toggle)
#ifdef TARGET_API_MAC_CARBON
            EnableMenuItem (myMenu,theItem+1);
#else
            EnableItem (myMenu,theItem+1);
#endif
        else
#ifdef TARGET_API_MAC_CARBON
            DisableMenuItem (myMenu,theItem+1);
#else
            DisableItem (myMenu,theItem+1);
#endif
}

//__________________________________________________________________
void        _HYPlatformPullDown::_Paint (Ptr p)
{
    _HYPullDown * theParent = (_HYPullDown*)this;

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

    if (theParent->MenuItemCount()) {
        DrawMenuPlaceHolder (menuRect,*theParent->GetMenuItem(selection),theParent->IsEnabled()&&theParent->activationFlag);
    }

    (*theParent)._HYPlatformComponent::_Paint(p);
}

//__________________________________________________________________
_HYRect _HYPullDown::_SuggestDimensions (void)
{
    _HYRect res = {25,100,25,100,HY_COMPONENT_NO_SCROLL};
#ifdef OPAQUE_TOOLBOX_STRUCTS
    if (myMenu) {
        res.right = GetMenuWidth (myMenu)+22;
    }
#else
    if (myMenu) {
        res.right = (*myMenu)->menuWidth+22;
    }
#endif
    return res;
}

//__________________________________________________________________

void    _HYPullDown::_SetMenuItemTextStyle (long ID, char style)
{
    if (myMenu) {
        SetItemStyle (myMenu,ID+1,style);
    }
}

//__________________________________________________________________

bool _HYPullDown::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord*    theEvent = (EventRecord*)vEvent;
    if (myMenu)
        switch (theEvent->what) {
        case mouseDown: {
            if (enabledFlag) {
                Point localClick = theEvent->where;
                GlobalToLocal (&localClick);
                if (PtInRect(localClick,&menuRect)) {
                    if (messageRecipient) {
                        messageRecipient->ProcessEvent (generateMenuOpenEvent (GetID()));
                    }

                    localClick.v = menuRect.top;
                    localClick.h = menuRect.left;
                    LocalToGlobal(&localClick);
                    SetItemMark (myMenu,selection+1,checkMark);
                    unsigned long res = PopUpMenuSelect (myMenu,localClick.v, localClick.h, selection+1);
                    SetItemMark (myMenu,selection+1,noMark);
                    if (HiWord(res)!=0) {
                        selection = LoWord(res)-1;
                        SendSelectionChange();
                        _MarkForUpdate();
                    }
                }
            }
            return true;
        }
        }
    return _HYPlatformComponent::_ProcessOSEvent (vEvent);
}

//EOF