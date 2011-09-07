/*
    Mac OS Portions of the model window class

    Sergei L. Kosakovsky Pond, Spring 2000 - December 2002.
*/

#include "HYModelWindow.h"
#include "HYParameterTable.h"
#include "HYUtils.h"

#include "Scrap.h"


//__________________________________________________________________

void _HYModelWindow::_UnsetMenuBar(void)
{
    MenuHandle lfMenu   = GetMenuHandle (HY_MODEL_WINDOW_MENU_ID),
               saveMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID+1),
               rateMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID+2),
               fMenu   = GetMenuHandle (129);

    DeleteMenu (HY_MODEL_WINDOW_MENU_ID);
    DeleteMenu (HY_PARAMETER_TABLE_HMENU_ID+1);
    DeleteMenu (HY_PARAMETER_TABLE_HMENU_ID+2);
    DisposeMenu (lfMenu);
    DisposeMenu (saveMenu);
    DisposeMenu (rateMenu);
    SetItemCmd (fMenu,4,'S');
    _HYWindow::_UnsetMenuBar();
}

//__________________________________________________________________


bool        _HYModelWindow::_ProcessMenuSelection (long msel)
{
    long        menuChoice = msel&0x0000ffff;
    MenuHandle  tableMenu;

    HiliteMenu(0);
    InvalMenuBar();

    switch (msel/0xffff) {
    case 129: { // file menu
        if (menuChoice==8) { // print
            _HYTable* t = (_HYTable*)GetCellObject (MODEL_MATRIX_ROW,4);
            t->_PrintTable((_HYTable*)GetCellObject (MODEL_MATRIX_ROW-1,4));
            return true;
        }
        break;
    }
    case 130: { // edit
        if (menuChoice==4) { // copy
            DoCopyCell ();
            return true;
        }
        if (menuChoice==5) { // paste
            DoPasteToCells();
            return true;
        }
        if (menuChoice==8) { // select all
            DoSelectAll();
            return true;
        }
        break;
    }
    case HY_MODEL_WINDOW_MENU_ID: { // model menu
        if (menuChoice==1) { // edit name
            DoEditModelName ();
            return true;
        }
        break;
    }
    case HY_PARAMETER_TABLE_HMENU_ID+1: { // save menu
        DoSave (menuChoice-2);
        return true;
    }
    case HY_PARAMETER_TABLE_HMENU_ID+2: { // rate menu
        if (menuChoice-1!=rateChoice) {
            tableMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID+2);
            SetItemMark     (tableMenu,rateChoice+1,noMark);
            rateChoice = menuChoice-1;
            SetItemMark     (tableMenu,rateChoice+1,checkMark);
            taint = true;
        }
        return true;
    }
    }

    if (_HYTWindow::_ProcessMenuSelection(msel)) {
        return true;
    }

    return false;
}

//__________________________________________________________________

void _HYModelWindow::_UpdateEditMenu (bool c, bool p)
{
    MenuHandle  t = GetMenuHandle (130);
    if (c) {
        EnableMenuItem (t,4);
    } else {
        DisableMenuItem (t,4);
    }
    if (p) {
        EnableMenuItem (t,5);
    } else {
        DisableMenuItem (t,5);
    }
    InvalMenuBar();
}

//__________________________________________________________________

bool _HYModelWindow::_CheckClipboard (void)
{
    Handle scrapHandle = NewHandle (0);
    clipboardString = empty;
    long   rc;

#ifdef TARGET_API_MAC_CARBON
    ScrapRef theScrapRef;
    if (GetCurrentScrap(&theScrapRef) != noErr) {
        return false;
    }
    if (GetScrapFlavorSize(theScrapRef, 'TEXT', &rc) != noErr) {
        return false;
    }
#else
    long    scrapOffset;
    rc = GetScrap( scrapHandle, 'TEXT', &scrapOffset );
#endif
    if ( rc >= 0 ) {
        SetHandleSize( scrapHandle, rc+1 );
        HLock  (scrapHandle);
#ifdef TARGET_API_MAC_CARBON
        long err = GetScrapFlavorData(theScrapRef, 'TEXT', &rc, *scrapHandle);
#endif
        (*scrapHandle)[rc] = 0;
        if (err == noErr) {
            _String cText ((char*)*scrapHandle);
            skipWarningMessages = true;
            _Formula f (cText,nil,false);
            skipWarningMessages = false;
            if (f.GetList().lLength) {
                clipboardString = cText;
                SyncEditBox ();
            }
        }
        HUnlock (scrapHandle);
    }

    DisposeHandle (scrapHandle);
    return clipboardString.sLength;
}

//__________________________________________________________________

void _HYModelWindow::_SetClipboard (void)
{
    if (clipboardString.sLength) {
#ifdef TARGET_API_MAC_CARBON
        ClearCurrentScrap();
        ScrapRef         theScrapRef;
        GetCurrentScrap(&theScrapRef);
        PutScrapFlavor(theScrapRef, 'TEXT', kScrapFlavorMaskNone,clipboardString.sLength+1,clipboardString.sData);
#else
        ZeroScrap();
        PutScrap (clipboardString.sLength+1,'TEXT',clipboardString.sData);
#endif
    }
}

//__________________________________________________________________

void _HYModelWindow::_SetMenuBar(void)
{
    _HYWindow::_SetMenuBar();
    MenuHandle  t = GetMenuHandle (130);
    EnableMenuItem (t,4);
    EnableMenuItem (t,8);
    MenuHandle lfMenu = GetMenuHandle (HY_MODEL_WINDOW_MENU_ID);
    if (!lfMenu) {
        lfMenu = NewMenu(HY_MODEL_WINDOW_MENU_ID,"\pModel");
        if (!lfMenu) {
            warnError (-108);
        }

        MenuHandle saveMenu = NewMenu(HY_PARAMETER_TABLE_HMENU_ID+1,"\pSave"),
                   rateMenu = NewMenu(HY_PARAMETER_TABLE_HMENU_ID+2,"\pRate Variation");

        InsertMenu      (rateMenu,hierMenu);
        InsertMenuItem  (lfMenu,"\pModel Name",10000);
        InsertMenuItem  (lfMenu,"\pRate Variation",10000);
        InsertMenuItem  (saveMenu,"\pSave/S",10000);
        InsertMenuItem  (saveMenu,"\pSave As..",10000);
        InsertMenu      (saveMenu,hierMenu);
        SetItemCmd      (lfMenu,2,hMenuCmd);
        SetItemMark     (lfMenu,2,HY_PARAMETER_TABLE_HMENU_ID+2);

        ListToPopUpMenu (rateOptions, rateMenu);

        SetItemMark (rateMenu,rateChoice+1,checkMark);
        InsertMenu (lfMenu,132);

        t = GetMenuHandle (129);
        SetItemCmd (t,4,hMenuCmd);
        SetItemMark(t,4,HY_PARAMETER_TABLE_HMENU_ID+1);
    }
    InvalMenuBar();
}


//EOF