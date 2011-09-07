/*
    Mac OS Portions of the parameter table class

    Sergei L. Kosakovsky Pond, Spring 2000 - December 2002.
*/

#include "HYParameterTable.h"
#include "HYUtils.h"

//__________________________________________________________________


bool        _HYParameterTable::_ProcessMenuSelection (long msel)
{
    long        menuChoice = msel&0x0000ffff;
    MenuHandle  tableMenu;
    _HYTable*   table = (_HYTable*)GetCellObject(HY_PARAMETER_TABLE_TABLE_ROW,0);

    HiliteMenu(0);
    InvalMenuBar();

    switch (msel/0xffff) {
    case 129: { // file menu
        if (menuChoice==4) { // save
            DoSave ();
            return true;
        }
        if (menuChoice==8) { // print
            _SimpleList columns,
                        sel;
            columns << 0;
            columns << 1;
            columns << 2;
            columns << 3;
            table->GetSelection (sel);
            char    resp = 3;
            if (sel.lLength) {
                _String pr ("Would you like to print only the selected cells? (Click \"No\" to print the entire table).");
                resp = YesNoCancelPrompt (pr);
            }
            if (resp == 3) {
                table->_PrintTable(columns,(_HYTable*)GetCellObject(HY_PARAMETER_TABLE_TABLE_ROW-1,0));
            } else if (resp == 1) {
                _SimpleList rows;
                for (long k = 0; k < sel.lLength; k+=4) {
                    rows << sel.lData[k]/4;
                }
                table->_PrintTable(columns,rows,(_HYTable*)GetCellObject(HY_PARAMETER_TABLE_TABLE_ROW-1,0));
            }
            return true;
        }
        break;
    }
    case 130: { // edit
        if (menuChoice==1) { // undo
            UndoCommand();
            _UpdateUndoMenu (nil,nil);
            return true;
        }
        if (menuChoice==4) { // copy
            return true;
        }
        if (menuChoice==8) { // select all
            SelectAll();
            return true;
        }
        break;
    }
    case HY_PARAMETER_TABLE_HMENU_ID: {
        tableMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID);
        char   toggleFlag;
        switch (menuChoice) {
        case 1:
            toggleFlag = HY_PARAMETER_TABLE_VIEW_LOCAL;
            break;
        case 2:
            toggleFlag = HY_PARAMETER_TABLE_VIEW_GLOBAL;
            break;
        case 3:
            toggleFlag = HY_PARAMETER_TABLE_VIEW_CONSTRAINED;
            break;
        case 4:
            toggleFlag = HY_PARAMETER_TABLE_VIEW_CATEGORY;
            break;

        }
        if (viewOptions&toggleFlag) {
            if (viewOptions-toggleFlag) {
                viewOptions-=toggleFlag;
            } else {
                break;
            }
        } else {
            viewOptions+=toggleFlag;
        }

        SetItemMark (tableMenu,menuChoice,(viewOptions&toggleFlag)?checkMark:noMark);
        ConstructTheTable();
        SetWindowRectangle (top,left,bottom,right);
        return true;
    }
    case HY_PARAMETER_TABLE_MENU_ID: {
        switch (menuChoice) {
        case 4:
            OptimizeLikelihoodFunction();
            break;

        case 6:
            DoEnterConstraint ();
            break;

        case 7:
            DoCleanUp   ();
            break;

        case 9:
            HandleVarianceEstimates ();
            break;

        case 10:
            HandleProfilePlot();
            break;

        case 12:
            HandleCategories();
            break;

        case 14:
            HandleSelectParameters();
            break;

        case 15:
            HandleOpenInChart     ();
            break;
        }
        return true;
    }
    }

    return _HYTWindow::_ProcessMenuSelection(msel);
}

//__________________________________________________________________

void _HYParameterTable::_UpdateViewMenu(void)
{
    MenuHandle viewMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID);

    if (!(avViewOptions&HY_PARAMETER_TABLE_VIEW_GLOBAL)) {
        DisableMenuItem (viewMenu,2);
    } else {
        EnableMenuItem (viewMenu,2);
    }


    if (!(avViewOptions&HY_PARAMETER_TABLE_VIEW_CONSTRAINED)) {
        DisableMenuItem (viewMenu,3);
    } else {
        EnableMenuItem (viewMenu,3);
    }

    if (!(avViewOptions&HY_PARAMETER_TABLE_VIEW_CATEGORY)) {
        DisableMenuItem (viewMenu,4);
    } else {
        EnableMenuItem (viewMenu,4);
    }

    if (!(avViewOptions&HY_PARAMETER_TABLE_VIEW_TREES)) {
        DisableMenuItem (viewMenu,5);
    } else {
        EnableMenuItem (viewMenu,5);
    }

    CheckMenuItem (viewMenu,1,viewOptions&HY_PARAMETER_TABLE_VIEW_LOCAL);
    CheckMenuItem (viewMenu,2,viewOptions&HY_PARAMETER_TABLE_VIEW_GLOBAL);
    CheckMenuItem (viewMenu,3,viewOptions&HY_PARAMETER_TABLE_VIEW_CONSTRAINED);
    CheckMenuItem (viewMenu,4,viewOptions&HY_PARAMETER_TABLE_VIEW_CATEGORY);
    CheckMenuItem (viewMenu,5,viewOptions&HY_PARAMETER_TABLE_VIEW_TREES);
    //CheckMenuItem (viewMenu,(viewOptions&HY_PARAMETER_TABLE_VIEW_ALPHABETICAL)?7:6,true);

}

//__________________________________________________________________

void _HYParameterTable::_SetMenuBar(void)
{
    _HYWindow::_SetMenuBar();

    MenuHandle  t  = GetMenuHandle (130);

    EnableMenuItem (GetMenuHandle (129),1);
    EnableMenuItem (t,8);
    if (undoCommands.lLength) {
        EnableMenuItem (GetMenuHandle (130),1);
    }

    MenuHandle lfMenu = GetMenuHandle (HY_PARAMETER_TABLE_MENU_ID);
    if (!lfMenu) {
        lfMenu = NewMenu(HY_PARAMETER_TABLE_MENU_ID,"\pLikelihood");
        if (!lfMenu) {
            warnError (-108);
        }

        MenuHandle viewMenu = NewMenu(HY_PARAMETER_TABLE_HMENU_ID,"\pView Options");

        InsertMenuItem (viewMenu,"\pLocal Parameters",0);
        InsertMenuItem (viewMenu,"\pGlobal Parameters",10000);
        InsertMenuItem (viewMenu,"\pConstrained Parameters",10000);
        InsertMenuItem (viewMenu,"\pRate Classes",10000);
        InsertMenuItem (viewMenu,"\pTrees",10000);
        InsertMenu (viewMenu,hierMenu);
        InsertMenuItem (lfMenu,"\pView Options",10000);
        SetItemCmd (lfMenu,1,hMenuCmd);
        SetItemMark(lfMenu,1,HY_PARAMETER_TABLE_HMENU_ID);

        InsertMenuItem (lfMenu,"\p(-",10000);
        InsertMenuItem (lfMenu,"\pRecalculate LF/U",10000);
        InsertMenuItem (lfMenu,"\pOptimize LF/T",10000);
        InsertMenuItem (lfMenu,"\p(-",10000);
        InsertMenuItem (lfMenu,"\pEnter Command",10000);
        InsertMenuItem (lfMenu,"\pRemove Unused Parameters",10000);
        InsertMenuItem (lfMenu,"\p(-",10000);
        InsertMenuItem (lfMenu,"\pCovariance, Sampler and CI",10000);
        InsertMenuItem (lfMenu,"\pLikelihood Profile Plot",10000);
        InsertMenuItem (lfMenu,"\p(-",10000);
        InsertMenuItem (lfMenu,"\pCategories Processor",10000);
        InsertMenuItem (lfMenu,"\p(-",10000);
        InsertMenuItem (lfMenu,"\pSelect Parameters",10000);
        InsertMenuItem (lfMenu,"\pOpen Selection in Table",10000);

        InsertMenu (lfMenu,132);
        _UpdateViewMenu();
    }
    InvalMenuBar();
}

//__________________________________________________________________

void _HYParameterTable::_UnsetMenuBar(void)
{
    MenuHandle lfMenu = GetMenuHandle (HY_PARAMETER_TABLE_MENU_ID),
               viewMenu = GetMenuHandle (HY_PARAMETER_TABLE_HMENU_ID);


    DeleteMenu (HY_PARAMETER_TABLE_MENU_ID);
    DeleteMenu (HY_PARAMETER_TABLE_HMENU_ID);
    DisposeMenu (viewMenu);
    DisposeMenu (lfMenu);
    _HYWindow::_UnsetMenuBar();
}

//__________________________________________________________________

void _HYParameterTable::_UpdateUndoMenu(_String* command, _String* desc)
{
    if (command&&desc) {
        EnableMenuItem (GetMenuHandle (130),1);
        undoCommands        &&  command;
        undoDescriptions    &&  desc;
        Str255 s255;
        StringToStr255 (*desc, s255);
        SetMenuItemText (GetMenuHandle (130),1,s255);
    } else {
        if (undoDescriptions.lLength==0) {
            Str255 s255 = "\pCan't Undo";
            SetMenuItemText (GetMenuHandle (130),1,s255);
            DisableMenuItem (GetMenuHandle (130),1);
        } else {
            Str255 s255;
            StringToStr255 (*(_String*)undoDescriptions(undoDescriptions.lLength-1), s255);
            EnableMenuItem (GetMenuHandle (130),1);
            SetMenuItemText (GetMenuHandle (130),1,s255);
        }
    }
    InvalMenuBar();
}

//EOF