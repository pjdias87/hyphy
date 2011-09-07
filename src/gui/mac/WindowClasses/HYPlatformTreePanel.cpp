/*
    Mac OS Portions of the tree panel class

    Sergei L. Kosakovsky Pond, Spring 2000 - December 2002.
*/

#include "HYTreePanel.h"
#include "HYDataPanel.h"
#include "HYUtils.h"

#include "QuickTimeComponents.h"
#include "ImageCompression.h"
#include "Scrap.h"


//__________________________________________________________________

_String                 saveForTreesPrompt          ("Save Tree As:"),
                        saveFormatsForTreesPrompt   ("File Type:");

//__________________________________________________________________

void _HYTreePanel::_DisplayBranchFloater (void)
{
    long h,v;

#ifdef  TARGET_API_MAC_CARBON
    GrafPtr thisPort = GetWindowPort(theWindow),
            savePort;
#else
    GrafPtr thisPort = (GrafPtr)theWindow,
            savePort;
#endif

    GetPort (&savePort);
    SetPort (thisPort);
    Point p = {saveMouseV,saveMouseH};

    _HYCanvas   *theTree    = (_HYCanvas*)GetObject (0);
    GlobalToLocal (&p);
    if (IsVertical()) {
        v = p.h-componentL.lData[0]+theTree->hOrigin;
        h = p.v-componentT.lData[0]+theTree->vOrigin;
    } else {
        h = p.h-componentL.lData[0]+theTree->hOrigin;
        v = p.v-componentT.lData[0]+theTree->vOrigin;
    }
    _SimpleList   saveSelection;
    saveSelection.Duplicate (&currentSelection);
    v = FindSelectedBranch (h,v);
    if (v) {
        node <nodeCoord>* thisNode = (node <nodeCoord>*) v;
        _String toolTip (thisNode->in_object.branchName);
        if (toolTip.sLength) {
            if (scaleVariable.sLength) {
                toolTip = toolTip & " (" & thisNode->in_object.bL & ')';
            }
            if (toolTip.sLength) {
                RGBColor      toolTipColor = {0xFFFF,0xBFFF,0x4000};
                PixPatHandle  toolTipPixPat = NewPixPat();
                MakeRGBPat (toolTipPixPat,&toolTipColor);
#ifdef OPAQUE_TOOLBOX_STRUCTS
                short   savedFace = GetPortTextFont (thisPort),
                        savedSize = GetPortTextSize (thisPort);

                Style   savedStyle = GetPortTextFace (thisPort);
#else
                short   savedFace = thisPort->txFont,
                        savedSize = thisPort->txSize;

                Style   savedStyle = thisPort->txFace;
#endif
                TextFont (kFontIDHelvetica);
                TextSize (9);
                TextFace (0);
                toolTipBounds.bottom = p.v;
                toolTipBounds.top = toolTipBounds.bottom - 12;
                if (toolTipBounds.top<0) {
                    toolTipBounds.top = p.v;
                    toolTipBounds.bottom = toolTipBounds.top+12;
                }
                h = GetVisibleStringWidth (toolTip);
                toolTipBounds.left = p.h;
                if (toolTipBounds.left<=0) {
                    toolTipBounds.left = 1;
                }
                toolTipBounds.right = toolTipBounds.left+h+2;
#ifdef OPAQUE_TOOLBOX_STRUCTS
                Rect portRect;
                GetPortBounds (thisPort,&portRect);
                h = toolTipBounds.right-portRect.right+portRect.left;
#else
                h = toolTipBounds.right-thisPort->portRect.right+thisPort->portRect.left;
#endif
                if (h>0) {
                    if (h>=toolTipBounds.left) {
                        h = toolTipBounds.left-1;
                    }
                    toolTipBounds.left -= h;
                    toolTipBounds.right-= h;
                }
                toolTipBounds.right += 2;
                Rect tRect = HYRect2Rect (toolTipBounds);
                RGBColor oldColor;
                GetForeColor (&oldColor);
                //toolTipColor.red = toolTipColor.blue = toolTipColor.green = 0;
                //RGBForeColor (&toolTipColor);
                //FrameRect  (&tRect);
                MoveTo (toolTipBounds.left+2,toolTipBounds.bottom-3);
                toolTipColor.red = toolTipColor.blue = toolTipColor.green = 0;
                FillCRect  (&tRect,toolTipPixPat);
                RGBForeColor (&toolTipColor);
                FrameRect (&tRect);
                DrawText (toolTip.sData,0,toolTip.sLength);
                RGBForeColor (&oldColor);
                TextFont (savedFace);
                TextSize (savedSize);
                TextFace (savedStyle);
                DisposePixPat (toolTipPixPat);
                SetPort (savePort);
            }
        }
        SetPort (savePort);
    }
}

//__________________________________________________________________

void _HYTreePanel::_HandleIdleEvent (void)
{
#ifdef TARGET_API_MAC_CARBON
    Point    curMouse;
    GetGlobalMouse (&curMouse);

    unsigned long t;
    GetDateTime(&t);


    if ((abs(curMouse.h-saveMouseH)<=3)
            &&(abs(curMouse.v-saveMouseV)<=3)
            &&(t-lastSave>.5))

    {
        if (!HasToolTip()) {
            GrafPtr curPort;
            GetPort (&curPort);
            SetPort (GetWindowPort (theWindow));
            _DisplayBranchFloater();
            SetPort (curPort);
        }

        lastSave   = t;
    }

    saveMouseH = curMouse.h;
    saveMouseV = curMouse.v;
#endif
}

//__________________________________________________________________

bool _HYTreePanel::_ProcessOSEvent (Ptr vEvent)
{
    EventRecord* theEvent = (EventRecord*)vEvent;
    static  UInt32  lastClick = 0;
    static  int     lastH = 0, lastV = 0;
//  static  long    lastSelection = -1;

    if ((theEvent->what==mouseDown)&&(theEvent->modifiers&controlKey)) {
        Point localClick = theEvent->where;
        GrafPtr savedPort;
        GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
        SetPort(GetWindowPort(theWindow));
#else
        SetPort(theWindow);
#endif
        GlobalToLocal (&localClick);
        SetPort(savedPort);
        if (FindClickedCell(localClick.h,localClick.v) == 0) {
            HandleContextPopup (theEvent->where.h, theEvent->where.v);
            return true;
        }
    }


    if(!_HYTWindow::_ProcessOSEvent (vEvent)) {
        if (theEvent->what==mouseDown) {
            Point localClick = theEvent->where;
            GrafPtr savedPort;
            GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
            SetPort(GetWindowPort(theWindow));
#else
            SetPort(theWindow);
#endif
            GlobalToLocal (&localClick);
            int   c = FindClickedCell(localClick.h,localClick.v),ch,cv;
            if (c<0) {
                return false;
            }
            _HYComponent* thisComponent = (_HYComponent*)components(c);
            if (c==1) { // navBar
                ch = localClick.h-componentL.lData[1];
                cv = localClick.v-componentT.lData[1];
                if (navRect.Contains(ch,cv)) {
                    Point   oldPt, newPt;
                    oldPt=localClick;
                    GlobalToLocal(&oldPt);
                    if (StillDown()) {
                        while (WaitMouseUp()) {
                            GetMouse( &newPt);
                            if ( DeltaPoint(oldPt, newPt) ) {
                                oldPt=newPt;
                                ch = newPt.h-componentL.lData[1];
                                cv = newPt.v-componentT.lData[1];
                                SetNavRectCenter (ch,cv);
                            }
                        }
                    }
                } else {
                    SetNavRectCenter (ch,cv);
                }
                SetPort(savedPort);
                return true;
            } else if (c==0) { // tree panel
                ch = localClick.h-componentL.lData[0];
                cv = localClick.v-componentT.lData[0];

                if ((theEvent->modifiers&cmdKey)&&(theEvent->modifiers&optionKey)) {
                    _HYCanvas   *theTree    = (_HYCanvas*)GetObject (0);
                    _HYCanvas   *viewFinder = (_HYCanvas*)GetObject (1);
                    Point oldPt = {-1,-1};

                    if (StillDown()) {
                        while (WaitMouseUp()) {
                            GetMouse(&localClick);
                            GlobalToLocal (&localClick);
                            if (DeltaPoint (oldPt,localClick)) {
                                if (IsVertical()) {
                                    cv = localClick.h-componentL.lData[0]+theTree->hOrigin;
                                    ch = localClick.v-componentT.lData[0]+theTree->vOrigin;
                                } else {
                                    ch = localClick.h-componentL.lData[0]+theTree->hOrigin;
                                    cv = localClick.v-componentT.lData[0]+theTree->vOrigin;
                                }
                                if ((ch<theTree->_HYComponent::GetMaxW())&&(cv<theTree->_HYComponent::GetMaxH())) {
                                    FishEyeProjection (ch,theTree->_HYComponent::GetMaxH()-cv,theTree->_HYComponent::GetMaxW(),
                                                       theTree->_HYComponent::GetMaxH(),coordTree);
                                    treeFlags |= HY_TREEPANEL_PROJECTION;
                                    forceUpdateForScrolling = true;
                                    RenderTree(false);
                                    forceUpdateForScrolling = false;
                                }
                            }
                            oldPt = localClick;
                        }
                        RenderNavTree();
                        viewFinder->_MarkForUpdate();
                    }
                    SetPort(savedPort);
                    return true;
                }

                char shiftFlag = ((0x0200&theEvent->modifiers)>0);
                if ((0x0800&theEvent->modifiers)>0) {
                    shiftFlag|=0x02;
                }
                if ((cmdKey&theEvent->modifiers)>0) {
                    shiftFlag|=0x04;
                }
                if (IsVertical()) {
                    c = ch;
                    ch = cv+thisComponent->vOrigin;
                    cv = c+thisComponent->hOrigin;
                } else {
                    ch+=thisComponent->hOrigin;
                    cv+=thisComponent->vOrigin;
                }
                if (theEvent->when-lastClick<GetDblTime()) {
                    if ((abs(ch-lastH)<5)&&(abs(cv-lastV)<5)&&(currentSelection.lLength))
                        if (theEvent->modifiers&cmdKey) {
                            DisplayParameterTable();
                        } else {
                            InvokeNodeEditor();
                        }
                    SetPort(savedPort);
                    return true;
                }
                lastH = ch;
                lastV = cv;
                if(FindSelection (ch,cv,shiftFlag)) {
                    _UpdateOperationsMenu();
                    RenderTree();
                }

                lastClick = theEvent->when;
            }
            SetPort(savedPort);
        } else {
            if (theEvent->what == keyDown) {
                unsigned char keyCode = (theEvent->message&keyCodeMask)>>8;

                if (currentSelection.lLength) {
                    if ((keyCode == 0x33)||(keyCode == 0x75)) { // delete
                        DeleteCurrentSelection();
                        _UpdateOperationsMenu();
                        return true;
                    } else if ((keyCode == 0x4C)||(keyCode == 0x24)) { // return
                        InvokeNodeEditor ();
                        _UpdateOperationsMenu();
                        return true;
                    }
                }
            } else if (theEvent->what == osEvt) {
                unsigned long t;
                GetDateTime(&t);
#ifndef TARGET_API_MAC_CARBON
                if ((abs(theEvent->where.h-saveMouseH)<=3)
                        &&(abs(theEvent->where.v-saveMouseV)<=3)) {
                    if ((t-lastSave>=1)&&(!toolTipBounds.left)) {
                        lastSave = t;
                        _DisplayBranchFloater ();
                        return true;

                    }
                    lastSave = t;
                } else
#endif
                {
                    if (toolTipBounds.left) {
                        Rect     tRect;
                        tRect = HYRect2Rect (toolTipBounds);
#ifdef OPAQUE_TOOLBOX_STRUCTS
                        InvalWindowRect (theWindow,&tRect);
#else
                        InvalRect (&tRect);
#endif
                        toolTipBounds.left = 0;
                    }
                    saveMouseH = theEvent->where.h;
                    saveMouseV = theEvent->where.v;
                    lastSave = t;
                }
                return true;
            }
        }
        return false;
    }
    return true;
}


//__________________________________________________________________

bool        _HYTreePanel::_ProcessMenuSelection (long msel)
{
    if (_HYWindow::_ProcessMenuSelection(msel)) {
        return true;
    }

    long        menuChoice = msel&0x0000ffff;
    MenuHandle  treeMenu;
    _String     prompt;

    switch (msel/0xffff) {
    case 129: { // file menu
        if (menuChoice==4) { // save
            _String           filePath;
            _List             saveFormatsForTrees;

            long pidOptions = GetUniversalSaveOptions (saveFormatsForTrees)+1;

            InitializeQTExporters ();
            saveFormatsForTrees&& & menuSeparator;
            saveFormatsForTrees << graphicsFormats;

            menuChoice = SaveFileWithPopUp (filePath, saveForTreesPrompt,treeName,
                                            saveFormatsForTreesPrompt,saveFormatsForTrees);
            if (menuChoice>=0) {
                _HYCanvas   *theTree = (_HYCanvas*)GetObject (0);
                Str255 buff;
                {
                    if (menuChoice <  pidOptions) {
                        HandleTreeSave (menuChoice, filePath);
                    } else {
                        menuChoice -= pidOptions;
                        ComponentInstance grexc = OpenComponent ((Component)qtGrexComponents(menuChoice));
                        GWorldPtr         comboGW = nil;
                        if (scaleVariable.sLength&&(!IsVertical())) {
                            Rect  bRect, sRect;
                            bRect.left = bRect.top = 0;
                            bRect.right = theTree->_HYComponent::GetMaxW();
                            bRect.bottom = theTree->_HYComponent::GetMaxH()+HY_TREEPANEL_RULER_EXPANDED;
                            short errCode = NewGWorld (&comboGW,2,&bRect,0,nil,0);
                            if (errCode!=noErr) {
                                _String errMsg ("MacOS Error ");
                                errMsg = errMsg & (long)errCode &" while trying to allocate memory for GraphicPane";
                                WarnError (errMsg);
                                return true;
                            }
                            sRect = bRect;
                            sRect.bottom -= HY_TREEPANEL_RULER_EXPANDED;
                            bRect.top = HY_TREEPANEL_RULER_EXPANDED;
                            LockPixels (GetGWorldPixMap(theTree->thePane));
                            CopyBits ((BitMap*)*GetGWorldPixMap(theTree->thePane),(BitMap*)*GetGWorldPixMap(comboGW),&sRect,&bRect,srcCopy,(RgnHandle)nil);
                            UnlockPixels (GetGWorldPixMap(theTree->thePane));
                            bRect.bottom = bRect.top;
                            bRect.top = 0;
                            sRect.bottom = HY_TREEPANEL_RULER_EXPANDED-1;
                            _HYCanvas   *theRuler = (_HYCanvas*)GetObject (2);
                            LockPixels (GetGWorldPixMap(theRuler->thePane));
                            CopyBits ((BitMap*)*GetGWorldPixMap(theRuler->thePane),(BitMap*)*GetGWorldPixMap(comboGW),&sRect,&bRect,srcCopy,(RgnHandle)nil);
                            UnlockPixels (GetGWorldPixMap(theRuler->thePane));
                            GraphicsExportSetInputGWorld (grexc,comboGW);

                        } else {
                            GraphicsExportSetInputGWorld (grexc,theTree->thePane);
                        }
                        FSSpec  fs;
                        StringToStr255 (filePath,buff);
                        FSMakeFSSpec(0,0,buff,&fs);
                        GraphicsExportSetOutputFile (grexc,&fs);
                        GraphicsExportRequestSettings (grexc,nil,nil);
                        unsigned long dummy;
                        OSType t,c;
                        GraphicsExportGetDefaultFileTypeAndCreator (grexc,&t,&c);
                        GraphicsExportSetOutputFileTypeAndCreator (grexc,t,c);
                        GraphicsExportDoExport (grexc,&dummy);
                        CloseComponent (grexc);
                        if (comboGW) {
                            DisposeGWorld (comboGW);
                        }
                    }
                }
            }
            return true;
        }
        if (menuChoice==8) { // print
            _PrintTree();
            return true;
        }
        break;
    }
    case 130: { // edit
        HiliteMenu(0);
        if (menuChoice==4) { // copy
            if (treeFlags&HY_TREEPANEL_CLIPBOARD_READY) {
                CutSelectionToClipboard (false);
            } else {
#ifdef TARGET_API_MAC_CARBON
                ClearCurrentScrap();
#else
                ZeroScrap();
#endif
                //_String res = GetTreeString();
                //PutScrap (res.sLength,'TEXT',res.sData);
                _HYCanvas   *theTree = (_HYCanvas*)GetObject (0);
                Rect  bRect;
                if (scaleVariable.sLength&&(!IsVertical())) { // have ruler
                    _HYCanvas   *theRuler = (_HYCanvas*)GetObject (2);
                    Rect        bRect2;
                    bRect.left          = bRect.top = 0;
                    bRect.right         = theTree->_HYComponent::GetMaxW();
                    bRect.bottom        = theTree->_HYComponent::GetMaxH();
                    bRect2              = bRect;
                    bRect2.bottom       += theRuler->_HYComponent::GetMaxH();
                    PicHandle    pic    = OpenPicture (&bRect2);

                    /*bRect2.top            = theRuler->_HYComponent::GetMaxH();
                    GrafPtr      topPort;
                    GetPort      (&topPort);

                    LockPixels (GetGWorldPixMap(theTree->thePane));
                    CopyBits ((BitMap*)*GetGWorldPixMap(theTree->thePane),
                              (BitMap*)&(topPort->portBits),&bRect,&bRect2,
                                   srcCopy,(RgnHandle)nil);

                    UnlockPixels (GetGWorldPixMap(theTree->thePane));
                    bRect.bottom = bRect2.top;
                    bRect2.top = 0;
                    bRect2.bottom = bRect.bottom;

                    LockPixels (GetGWorldPixMap(theRuler->thePane));
                    CopyBits ((BitMap*)*GetGWorldPixMap(theRuler->thePane),
                              (BitMap*)&(topPort->portBits),&bRect,&bRect2,
                                   srcCopy,(RgnHandle)nil);
                    UnlockPixels (GetGWorldPixMap(theRuler->thePane));*/

                    ShiftScreenCoordinates (0,theRuler->_HYComponent::GetMaxH(),coordTree);
                    RenderTree (false,true);
                    ShiftScreenCoordinates (0,-theRuler->_HYComponent::GetMaxH(),coordTree);

                    ClosePicture ();
                    HLock   ((Handle)pic);
#ifdef TARGET_API_MAC_CARBON
                    ClearCurrentScrap();
                    ScrapRef         theScrapRef;
                    GetCurrentScrap(&theScrapRef);
                    PutScrapFlavor(theScrapRef, 'PICT', kScrapFlavorMaskNone,GetHandleSize((Handle)pic),*pic);
#else
                    PutScrap (GetHandleSize((Handle)pic),'PICT',*pic);
#endif
                    KillPicture (pic);

                } else {
                    bRect.left = bRect.top = 0;
                    bRect.right = theTree->_HYComponent::GetMaxW();
                    bRect.bottom = theTree->_HYComponent::GetMaxH();
                    PicHandle    pic = OpenPicture (&bRect);
                    GrafPtr      topPort;
                    GetPort      (&topPort);

                    RenderTree  (true,true);
                    /*LockPixels (GetGWorldPixMap(theTree->thePane));
                    CopyBits ((BitMap*)*GetGWorldPixMap(theTree->thePane),
                              (BitMap*)&(topPort->portBits),&bRect,&bRect,
                                   srcCopy,(RgnHandle)nil);
                    UnlockPixels (GetGWorldPixMap(theTree->thePane));*/
                    ClosePicture ();
                    HLock   ((Handle)pic);
#ifdef TARGET_API_MAC_CARBON
                    ClearCurrentScrap();
                    ScrapRef         theScrapRef;
                    GetCurrentScrap(&theScrapRef);
                    PutScrapFlavor(theScrapRef, 'PICT', kScrapFlavorMaskNone,GetHandleSize((Handle)pic),*pic);
#else
                    PutScrap (GetHandleSize((Handle)pic),'PICT',*pic);
#endif
                    KillPicture (pic);
                }
            }
            return true;
        } else if (menuChoice==6) { // delete selection
            DeleteCurrentSelection();
            _UpdateOperationsMenu();
            return true;
        } else if (menuChoice==3) { // cut selection
            CutSelectionToClipboard ();
            _UpdateOperationsMenu();
            return true;
        } else if (menuChoice==5) { // paste selection
            PasteClipboardTree();
            _UpdateOperationsMenu();
            return true;
        } else if (menuChoice==8) { // select all
            SelectAllBranches();
            return true;
        }
        if (menuChoice==1) { // undo
            UndoLastOperation();
            _UpdateOperationsMenu();
            return true;
        } else if (menuChoice==11) { // s&r
            HandleSearchAndReplace(false);
            return true;
        } else if (menuChoice==12) { // s&r in selection
            HandleSearchAndReplace(true);
            return true;
        }
        return false;
    }
    case HY_TREEPANEL_MENU_ID: {
        treeMenu = GetMenuHandle (HY_TREEPANEL_MENU_ID);
        switch (menuChoice) {
            unsigned short newF;
        case 1:
            if (treeFlags&HY_TREEPANEL_TIP_LABELS) {
                newF = treeFlags - HY_TREEPANEL_TIP_LABELS;
                SetFlags (newF);
            } else {
                newF = treeFlags + HY_TREEPANEL_TIP_LABELS;
                SetFlags (newF);
            }
            CheckMenuItem (treeMenu,1,(treeFlags&HY_TREEPANEL_TIP_LABELS));
            break;

        case 2:
            if (treeFlags&HY_TREEPANEL_INT_LABELS) {
                newF = treeFlags - HY_TREEPANEL_INT_LABELS;
                SetFlags (newF);
            } else {
                newF = treeFlags + HY_TREEPANEL_INT_LABELS;
                SetFlags (newF);
            }
            CheckMenuItem (treeMenu,2,(treeFlags&HY_TREEPANEL_INT_LABELS));
            break;

        case 4: { // SWAP SUBTREES
            SwapSelectedSubTrees();
            break;
        }

        case 5: { // COLLAPSE BRANCH
            CollapseSelectedBranch ();
            break;
        }

        case 6: { // COLLAPSE BRANCH
            JoinSelectedBranches();
            break;
        }

        case 7: { // Graft A Tip
            GraftATip();
            break;
        }

        case 8: { // Reroot Tree
            RerootTree();
            break;
        }

        case 9: { // Flip tree
            FlipSelectedBranches();
            break;
        }

        case 13: { // Invoke node editor
            InvokeNodeEditor();
            break;
        }

        case 15: { // Optimize again
            RecalculateLikelihood();
            break;
        }

        case 16: { // Display Parameter Table
            DisplayParameterTable();
            break;
        }

        case 18: {
            HandleViewOptions ();
            break;
        }

        case 21:
        case 22: {
            ShowModelMatrix (menuChoice-21);
            break;
        }

        case 24:
        case 25: {
            GenerateDistanceTable (menuChoice-24);
            break;
        }

        case 28: {
            MatchToDataSet ();
            break;
        }

        }
        if ((menuChoice>2)&&(menuChoice<15)) {
            _UpdateOperationsMenu();
        }
        break;
    }
    case HY_TREEPANEL_HMENU_ID: { // edit
        HandleLabels(menuChoice-1);
        break;
    }
    case HY_TREEPANEL_HMENU_ID+1: { // select
        HandleSelection (menuChoice-1);
        _UpdateOperationsMenu();
        break;
    }
    case HY_TREEPANEL_HMENU_ID+2: { // compare
        HandleComparison (menuChoice-1);
        _UpdateOperationsMenu();
        break;
    }
    case HY_TREEPANEL_HMENU_ID+3: { // compare
        ExecuteProcessor (menuChoice-1);
        _UpdateOperationsMenu();
        break;
    }

    }

    HiliteMenu(0);
    InvalMenuBar();
    return false;
}

//__________________________________________________________________

void        _HYTreePanel::_PaintNavRect(void)
{
    navRect = ComputeNavRect();
    _HYCanvas* theCanvas = (_HYCanvas*)GetObject (1);
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
#else
    SetPort(theWindow);
#endif
    Rect r;
    r.left = navRect.left+theCanvas->rel.left+HY_TREEPANEL_NAVSPACING;
    r.right = navRect.right+theCanvas->rel.left;
    r.top = navRect.top+theCanvas->rel.top+HY_TREEPANEL_NAVSPACING;
    r.bottom = navRect.bottom+theCanvas->rel.top;
    RGBColor saveColor, newColor = {255*256,151*256,51*256};
    PenState ps;
    GetPenState (&ps);
    PenSize (2,2);
    GetForeColor (&saveColor);
    RGBForeColor (&newColor);
    FrameRect (&r);
    _PaintLFStatus ();
    RGBForeColor (&saveColor);
    SetPenState (&ps);
    SetPort(savedPort);
}

//__________________________________________________________________

void        _HYTreePanel::_PaintLFStatus(Ptr)
{
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
#else
    SetPort(theWindow);
#endif
    if (likeFuncID<0) {
        _PaintTheCircle (redButtonIcon,theWindow);
    } else {
        if (dubiousNodes.lLength) {
            _PaintTheCircle (yellowButtonIcon,theWindow);
        } else {
            _PaintTheCircle (greenButtonIcon,theWindow);
        }
    }
    SetPort(savedPort);
}

//__________________________________________________________________

void        _HYTreePanel::_PrintTree(long hPages, long vPages)
{
    if ((hPages<0)||(vPages<0)) {
        if (!TreePrintSetup (hPages, vPages, (Ptr)this)) {
            return;
        }
    }

    _HYCanvas* theCanvas = (_HYCanvas*)GetObject (0);

    GrafPtr     savePort;
    GetPort(&savePort);
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

    PMPrintSession hyPC;
    theStatus = PMCreateSession(&hyPC);
    if (theStatus != noErr) {
        return;
    }

#endif

    if (!InitPrint(hyPC)) {
        _String errMsg ("Could not initialize printing variables.");
        WarnError (errMsg);
        terminateExecution = false;
        return;
    }

    //GetPort(&savePort);

#ifdef TARGET_API_MAC_CARBON
    if (gPrintSettings != kPMNoPrintSettings) {
        theStatus = PMSessionValidatePrintSettings(hyPC,gPrintSettings, kPMDontWantBoolean);
    } else {
        theStatus = PMCreatePrintSettings(&gPrintSettings);

        if ((theStatus == noErr) && (gPrintSettings != kPMNoPrintSettings)) {
            theStatus = PMSessionDefaultPrintSettings(hyPC,gPrintSettings);
        }
    }

    if (theStatus == noErr) {
        theStatus = PMSessionPrintDialog(hyPC,gPrintSettings, gPageFormat, &isAccepted);

        if (isAccepted) {
            theStatus = PMGetAdjustedPageRect(gPageFormat, &prRect);
            if (theStatus != noErr) {
                PMRelease(hyPC);
                return;
            }

            theStatus = PMSessionBeginDocument(hyPC,gPrintSettings, gPageFormat);
            if (theStatus != noErr) {
                PMRelease(hyPC);
                return;
            }

            //GrafPtr  prPortGPTR;
            //PMGetGrafPtr (thePrintingPort,&prPortGPTR);

            long     pageW    = prRect.right-prRect.left-2,
                     pageH    = prRect.bottom-prRect.top-2;

            UInt32   startPage,
                     endPage;

            //SetPort (prPortGPTR);

            PMGetFirstPage (gPrintSettings,&startPage);
            PMGetLastPage  (gPrintSettings,&endPage);
#else
    PrOpen();
    if (err=PrError()) {
        _String errMsg ("Could not print the tree. Error Code:");
        errMsg = errMsg & (long)err;
        WarnError (errMsg);
        terminateExecution = false;
        {
            PrClose();
            return;
        }
    }

    if (PrJobDialog(prRecHdl)) {
        printPort = PrOpenDoc(prRecHdl, nil, nil);
        SetPort((GrafPtr)printPort);
        long    startPage = (*prRecHdl)->prJob.iFstPage,
                endPage = (*prRecHdl)->prJob.iLstPage,
                pageW = (*prRecHdl)->prInfo.rPage.right-2;
        pageH = (*prRecHdl)->prInfo.rPage.bottom-2;
#endif

            long    visW    = theCanvas->_HYComponent::GetMaxW(),
                    visH  = theCanvas->_HYComponent::GetMaxH(),
                    printW  = pageW * hPages,
                    printH  = pageH * vPages;

            if (hPages <= 0) {
                hPages = visW/pageW;
                if (visW%pageW == 0) {
                    hPages ++;
                }

                printW= pageW * hPages;
            }

            if (vPages <= 0) {
                vPages = visH/pageH;
                if (visH%pageH == 0) {
                    vPages ++;
                }

                printH = pageH * vPages;
            }

            bool    hasRuler = (scaleVariable.sLength)&&(!IsVertical());

            _HYFont oldFont  = treeLabelFont,
                    bf1      = branchLabel1,
                    bf2       = branchLabel2;

            _Parameter hsc   = 1.0,
                       vsc   = 1.0;

            if (visW>printW) {
                hsc = (_Parameter)printW/visW;
            }
            if (hasRuler) {
                printH -= HY_TREEPANEL_RULER_EXPANDED;
            }
            if (visH>printH) {
                vsc = (_Parameter)printH/visH;
            }

            if ((hsc<1.0)||(vsc<1.0)) {
                treeLabelFont.size = ((_Parameter)treeLabelFont.size)*MIN(hsc,vsc);
                if (treeLabelFont.size<4) {
                    treeLabelFont.size = 4;
                }

                bf1.size = ((_Parameter)bf1.size)*MIN(hsc,vsc);
                bf2.size = ((_Parameter)bf2.size)*MIN(hsc,vsc);
                ShiftScreenCoordinates  (-windowTextMarginH,-windowTextMarginV,coordTree);
                Convert2ScreenCoordinates (hsc,vsc,0,coordTree);
            }
            if (visW<printW) {
                visW = (printW-visW)/2;
            } else {
                visW = 0;
            }

            if (visH<printH) {
                visH = (printH-visH)/2;
            } else {
                visH = 0;
            }

            if (IsVertical()) {
                long t = visH;
                visH = visW;
                visW = t;
            }

            //theCanvas->SetFont (treeLabelFont);

            for (long hCount = 0; hCount < hPages; hCount ++)
                for (long vCount = 0; vCount < vPages; vCount ++) {
#ifdef TARGET_API_MAC_CARBON
                    theStatus = PMSessionBeginPage(hyPC,gPageFormat,NULL);
                    if (theStatus != noErr) {
                        PMRelease(hyPC);
                        return;
                    }
                    GrafPtr ppPort;
                    PMSessionGetGraphicsContext (hyPC, NULL, (void**)&ppPort);
                    SetPort (ppPort);
#else
                    PrOpenPage      (printPort, nil);
#endif


                    if (visH||visW) {
                        ShiftScreenCoordinates  (visW,hasRuler?visH+HY_TREEPANEL_RULER_EXPANDED-5:visH,coordTree);
                    } else if (hasRuler&&(vCount==0)) {
                        ShiftScreenCoordinates  (0,HY_TREEPANEL_RULER_EXPANDED-5,coordTree);
                    }

                    if (hCount||vCount) {
                        ShiftScreenCoordinates  (-pageW*hCount,-pageH*vCount, coordTree);
                    }


                    if (hasRuler && (vCount == 0)) {
                        RenderRuler (hsc,true,visW,visH);
                    }

                    theCanvas->SetFont (treeLabelFont);

                    if (treeFlags&HY_TREEPANEL_ARCS) {
                        PaintArcs(theCanvas, coordTree);
                    } else if (treeFlags&(HY_TREEPANEL_STRAIGHT|HY_TREEPANEL_CIRCULAR)) {
                        PaintStraight(theCanvas, coordTree);
                    } else {
                        if (IsVertical()) {
                            PaintVSquare(theCanvas, coordTree);
                        } else {
                            PaintSquare(theCanvas, coordTree);
                            if (treeFlags&HY_TREEPANEL_LABEL1) {
                                theCanvas->SetFont(bf1);
                                PaintSquareBranchLabels (theCanvas,coordTree,true);
                            }
                            if (treeFlags&HY_TREEPANEL_LABEL2) {
                                theCanvas->SetFont(bf2);
                                PaintSquareBranchLabels (theCanvas,coordTree,false);
                            }
                        }
                    }

                    if (visH||visW) {
                        ShiftScreenCoordinates  (-visW,hasRuler?-visH-HY_TREEPANEL_RULER_EXPANDED+5:-visH,coordTree);
                    } else if (hasRuler&&(vCount==0)) {
                        ShiftScreenCoordinates  (0,-HY_TREEPANEL_RULER_EXPANDED+5,coordTree);
                    }

                    if (hCount||vCount) {
                        ShiftScreenCoordinates  (pageW*hCount, pageH*vCount,coordTree);
                    }

#ifdef TARGET_API_MAC_CARBON
                    PMSessionEndPage(hyPC);
#else
                    PrClosePage(printPort);
#endif

                }

            if ((hsc<1.0)||(vsc<1.0)) {
                ShiftScreenCoordinates  (-windowTextMarginH,-windowTextMarginV,coordTree);
                Convert2ScreenCoordinates (1.0/hsc,1.0/vsc,0,coordTree);
            }

            treeLabelFont = oldFont;
            theCanvas->SetFont (treeLabelFont);


#ifdef TARGET_API_MAC_CARBON
            theStatus = PMSessionEndDocument(hyPC);
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

            theStatus = PMRelease(hyPC);

#else
            PrCloseDoc(printPort);
            if (((*prRecHdl)->prJob.bJDocLoop = bSpoolLoop) && (!PrError() ) ) {
                PrPicFile(prRecHdl, nil, nil, nil, &prStatus);
            }
#endif
        }
#ifdef TARGET_API_MAC_CARBON
        else {
            theStatus = PMRelease(hyPC);
        }
#endif

#ifdef TARGET_API_MAC_CARBON
    }
#else
        PrClose();
        SetPort(savePort);
#endif
}

//__________________________________________________________________

void _HYTreePanel::_SetMenuBar(void)
{
    _HYWindow::_SetMenuBar();
    MenuHandle  t = GetMenuHandle (130);
    EnableMenuItem (t,4);
    EnableMenuItem (t,8);


    MenuHandle treeMenu = GetMenuHandle (HY_TREEPANEL_MENU_ID);
    if (!treeMenu) {
        MenuHandle labelMenu  = NewMenu(HY_TREEPANEL_HMENU_ID,"\pLabels");
        MenuHandle selectMenu = NewMenu(HY_TREEPANEL_HMENU_ID+1,"\pSelect Branches");
        MenuHandle compMenu   = NewMenu(HY_TREEPANEL_HMENU_ID+2,"\pTree comparison");
        MenuHandle procMenu   = NewMenu(HY_TREEPANEL_HMENU_ID+3,"\pAdditional Tools");

        treeMenu = NewMenu(HY_TREEPANEL_MENU_ID,"\pTree");

        if (!(treeMenu&&selectMenu&&labelMenu&&compMenu)) {
            warnError (-108);
        }

        InsertMenuItem (treeMenu,"\pInternal Labels",0);        // 1
        InsertMenuItem (treeMenu,"\pTip Labels",0);             // 2
        CheckMenuItem (treeMenu,1,treeFlags&HY_TREEPANEL_TIP_LABELS);
        CheckMenuItem (treeMenu,2,treeFlags&HY_TREEPANEL_INT_LABELS);
        InsertMenuItem (treeMenu,"\p(-",1000);                  // 3
        InsertMenuItem (treeMenu,"\p(Swap Subtrees/1",1000);    // 4
        InsertMenuItem (treeMenu,"\p(Collapse Branch/2",1000);  // 5
        InsertMenuItem (treeMenu,"\p(Join/3",1000);             // 6
        InsertMenuItem (treeMenu,"\p(Graft A Tip/4",1000);      // 7
        InsertMenuItem (treeMenu,"\p(Reroot/5",1000);           // 8
        InsertMenuItem (treeMenu,"\p(Flip tip ordering/6",1000);// 9
        InsertMenuItem (treeMenu,"\p(-",1000);                  // 10
        InsertMenuItem (treeMenu,"\pSelect Branches",1000);     // 11
        InsertMenuItem (treeMenu,"\p(-",1000);                  // 12
        InsertMenuItem (treeMenu,"\pEdit Properties",1000);     // 13
        InsertMenuItem (treeMenu,"\p(-",1000);                  // 14
        InsertMenuItem (treeMenu,"\p(Optimize Again/T",1000);   // 15
        InsertMenuItem (treeMenu,"\p(Show Parameters in Table/H",1000); // 16
        InsertMenuItem (treeMenu,"\p(-",1000);                         // 17
        InsertMenuItem (treeMenu,"\pTree Display Options...",1000);    // 18
        InsertMenuItem (treeMenu,"\pBranch Labels",1000);              // 19
        InsertMenuItem (treeMenu,"\p(-",1000);                         // 20
        InsertMenuItem (treeMenu,"\p(Show Rate Matrix",1000);          // 21
        InsertMenuItem (treeMenu,"\p(Show Transition Matrix",1000);    // 22
        InsertMenuItem (treeMenu,"\p(-",1000);                         // 23
        InsertMenuItem (treeMenu,"\pPairwise Distances",1000);         // 24
        InsertMenuItem (treeMenu,"\pBranch Length Distribution",1000); // 25

        InsertMenuItem (treeMenu,"\p(-",1000);                         // 26
        InsertMenuItem (treeMenu,"\pTree Comparison",1000);            // 27
        InsertMenuItem (treeMenu,"\pMatch Leaves To Sequence Data",1000); // 28

        InsertMenuItem (treeMenu,"\p(-",1000); // 29
        InsertMenuItem (treeMenu,"\pAdditional Tools",1000); // 30

        InsertMenu      (procMenu,hierMenu);


        InsertMenuItem (labelMenu,"\pAbove Branches.../8",1000);
        InsertMenuItem (labelMenu,"\pBelow Branches.../9",1000);

        InsertMenuItem (selectMenu,"\p(Select Entire Subtree",1000);
        InsertMenuItem (selectMenu,"\pSelect Incomplete Branches/I",1000);
        InsertMenuItem (selectMenu,"\pSelect Branches Without Models",1000);
        InsertMenuItem (selectMenu,"\pSelect Branches By Name",1000);
        InsertMenuItem (selectMenu,"\pSelect Branches By Branch Length",1000);
        InsertMenuItem (selectMenu,"\pInvert Selection",1000);
        InsertMenuItem (selectMenu,"\pExtend Selection",1000);
        InsertMenuItem (selectMenu,"\pMap Selection to Data Panel",1000);
        InsertMenuItem (selectMenu,"\pMap Selection to Another Tree",1000);

        InsertMenuItem (compMenu,"\pTest For Equality",1000);
        InsertMenuItem (compMenu,"\pFind Subtree In Another Tree",1000);
        InsertMenuItem (compMenu,"\pFind Maximal Common Subtree",1000);
        InsertMenuItem (compMenu,"\pFind Maximal Common Forest",1000);
        InsertMenuItem (compMenu,"\pMatch To Tree Pattern",1000);

        InsertMenu (selectMenu,hierMenu);
        InsertMenu (labelMenu, hierMenu);
        InsertMenu (compMenu, hierMenu);

        SetItemCmd (treeMenu,19,hMenuCmd);
        SetItemMark(treeMenu,19,HY_TREEPANEL_HMENU_ID);

        SetItemCmd (treeMenu,11,hMenuCmd);
        SetItemMark(treeMenu,11,HY_TREEPANEL_HMENU_ID+1);

        SetItemCmd (treeMenu,27,hMenuCmd);
        SetItemMark(treeMenu,27,HY_TREEPANEL_HMENU_ID+2);

        SetItemCmd (treeMenu,30,hMenuCmd);
        SetItemMark(treeMenu,30,HY_TREEPANEL_HMENU_ID+3);

        InsertMenu (treeMenu,132);
        t = GetMenuHandle (129);
        EnableMenuItem (t,1);

        t = GetMenuHandle (130);
        InsertMenuItem (t,"\p(-",1000);                              // 10
        InsertMenuItem (t,"\pSearch and Replace/F",1000);            // 11
        InsertMenuItem (t,"\pSearch and Replace in Selection",1000); // 12

        if (treeProcessors.lLength == 0) {
            DisableMenuItem (treeMenu,30);
        } else {
            Str255    buffer;
            for (long k=0; k<treeProcessors.lLength; k++) {
                _String *thisItem = (_String*)treeProcessors (k),
                         chopped = thisItem->Cut (thisItem->FindBackwards (':',0,-1)+1,-1);
                StringToStr255  (chopped,buffer);
                InsertMenuItem  (procMenu, buffer,10000);
            }
        }

    }
    _UpdateOperationsMenu();
    InvalMenuBar();
}

//__________________________________________________________________

void _HYTreePanel::_UpdateOperationsMenu (void)
{
    node<nodeCoord>* node1, *node2, *t;

    MenuHandle      treeMenu   = GetMenuHandle (HY_TREEPANEL_MENU_ID),
                    selectMenu = GetMenuHandle (HY_TREEPANEL_HMENU_ID+1),
                    compMenu   = GetMenuHandle (HY_TREEPANEL_HMENU_ID+2),
                    editMenu   = GetMenuHandle (130);

    DisableMenuItem (treeMenu,4);
    DisableMenuItem (treeMenu,5);
    DisableMenuItem (treeMenu,6);
    DisableMenuItem (treeMenu,7);
    DisableMenuItem (treeMenu,8);
    DisableMenuItem (treeMenu,9);
    //DisableMenuItem (treeMenu,12);
    DisableMenuItem (treeMenu,13);
    DisableMenuItem (treeMenu,21);
    DisableMenuItem (treeMenu,22);
    DisableMenuItem (editMenu,6);
    DisableMenuItem (editMenu,12);


    DisableMenuItem (selectMenu,1);
    DisableMenuItem (selectMenu,7);
    DisableMenuItem (selectMenu,8);
    DisableMenuItem (selectMenu,9);

    DisableMenuItem (compMenu,2);

    bool  good = true;
    long  k,j;
    if (currentSelection.lLength==2) {
        node1 = (node<nodeCoord>*)currentSelection(0);
        node2 = (node<nodeCoord>*)currentSelection(1);
        t = node1->parent;
        while (t) {
            if (t==node2) {
                good = false;
                break;
            }
            t = t->parent;
        }
        if (good) {
            t = node2->parent;
            while (t) {
                if (t==node1) {
                    good = false;
                    break;
                }
                t = t->parent;
            }
        }
        if (good) {
            EnableMenuItem (treeMenu,4);
        }
    }
    if (currentSelection.lLength) {
        EnableMenuItem (treeMenu,7);
        EnableMenuItem (treeMenu,13);
        EnableMenuItem (selectMenu,7);
        EnableMenuItem (selectMenu,8);
        EnableMenuItem (selectMenu,9);
        EnableMenuItem (editMenu,12);
        for (k=0; k<currentSelection.lLength; k++) {
            node1 = (node<nodeCoord>*)currentSelection(k);
            if (node1->get_num_nodes()) {
                EnableMenuItem (treeMenu,5);
                break;
            }
        }
        for (k=0; k<currentSelection.lLength; k++) {
            node1 = (node<nodeCoord>*)currentSelection(k);
            if (!node1->get_num_nodes()) {
                EnableMenuItem (editMenu,6);
                break;
            }
        }
        if (currentSelection.lLength>=2) {
            node1 = (node<nodeCoord>*)currentSelection(0);
            t = node1->parent;
            if (t&&(t->get_num_nodes()>currentSelection.lLength)) {
                for (k=1; k<currentSelection.lLength; k++) {
                    node1 = (node<nodeCoord>*)currentSelection(k);
                    if (node1->parent!=t) {
                        break;
                    }
                }
                if (k==currentSelection.lLength) {
                    EnableMenuItem (treeMenu,6);
                }
            }
        } else {
            node1 = (node<nodeCoord>*)currentSelection(0);
            if (node1->parent) {
                EnableMenuItem(treeMenu,8);
                SetMenuItemText (treeMenu,8,"\pReroot");
            }
            if (node1->get_num_nodes()>0) {
                EnableMenuItem(treeMenu,9);
                EnableMenuItem(selectMenu,1);
            }
            if (node1->in_object.varRef>=0) {
                _CalcNode* thisCNode = (_CalcNode*)LocateVar(node1->in_object.varRef);

                if (thisCNode&&(thisCNode->GetModelIndex() != HY_NO_MODEL)) {
                    EnableMenuItem (treeMenu,21);
                    EnableMenuItem (treeMenu,22);
                }
            }
        }
    } else {
        _TheTree *me = LocateMyTreeVariable();
        if (me) {
            if (me->RootedFlag()==UNROOTED) {
                EnableMenuItem(treeMenu,8);
                SetMenuItemText (treeMenu,8,"\pBalance");
            } else {
                EnableMenuItem(treeMenu,8);
                SetMenuItemText (treeMenu,8,"\pUnroot");
            }
        }
    }
    if (likeFuncID!=-1) {
        EnableMenuItem (treeMenu,15);
        EnableMenuItem (treeMenu,16);
    } else {
        DisableMenuItem (treeMenu,15);
        DisableMenuItem (treeMenu,16);
    }

    treeMenu = GetMenuHandle(130);
    if ((treePanelClipboardRoot)&&(currentSelection.lLength==1)) {
        EnableMenuItem (treeMenu,5);
    } else {
        DisableMenuItem(treeMenu,5);
    }

    DisableMenuItem(treeMenu,3);
    // check if can cut/paste
    t = nil;

    for (k=0; k<currentSelection.lLength; k++) {
        node1 = (node<nodeCoord>*)currentSelection.lData[k];
        if (node1->parent) {
            if (currentSelection.Find((long)node1->parent)<0) {
                if (t) {
                    break;
                } else {
                    t = node1;
                }
            }
            for (j=0; j<node1->nodes.length; j++) {
                if (currentSelection.Find((long)node1->nodes.data[j])<0) {
                    break;
                }
            }
            if (j<node1->nodes.length) {
                break;
            }
        } else {
            if (t) {
                break;
            } else {
                t = node1;
            }
        }
    }
    selectionTop = nil;
    treeFlags &= 0xFF7F;
    if (t&&(t->parent!=coordTree)&&(t->parent)) {
        if (k==currentSelection.lLength) {
            EnableMenuItem(treeMenu,3);
            EnableMenuItem(compMenu,2);

            selectionTop = t;
            treeFlags |= HY_TREEPANEL_CLIPBOARD_READY;
        }
    }
    _String undoMessage;
    EnableMenuItem (treeMenu,1);
    switch (undoCode) {
    case 1:
        undoMessage = "Undo Swap";
        break;
    case 2:
        undoMessage = "Undo Flip";
        break;
    case 3:
        undoMessage = "Undo Collapse";
        break;
    case 4:
        undoMessage = "Undo Delete";
        break;
    case 5:
        undoMessage = "Undo Join";
        break;
    case 6:
        undoMessage = "Undo Cut";
        break;
    case 7:
        undoMessage = "Undo Graft";
        break;
    case 8:
        undoMessage = "Undo Paste";
        break;
    case 9:
        undoMessage = "Undo Subtree Move";
        break;
    default:
        undoMessage = "Can't Undo";
        DisableMenuItem (treeMenu,1);
    }
    Str255 s255;
    StringToStr255 (undoMessage, s255);
    SetMenuItemText (treeMenu,1,s255);
}

//__________________________________________________________________

void _HYTreePanel::_UnsetMenuBar(void)
{
    MenuHandle treeMenu     = GetMenuHandle (HY_TREEPANEL_MENU_ID),
               labelMenu     = GetMenuHandle (HY_TREEPANEL_HMENU_ID),
               selectMenu   = GetMenuHandle (HY_TREEPANEL_HMENU_ID+1),
               compMenu        = GetMenuHandle (HY_TREEPANEL_HMENU_ID+2);


    DeleteMenu (HY_TREEPANEL_MENU_ID);
    DeleteMenu (HY_TREEPANEL_HMENU_ID);
    DeleteMenu (HY_TREEPANEL_HMENU_ID+1);
    DeleteMenu (HY_TREEPANEL_HMENU_ID+2);
    DisposeMenu (treeMenu);
    DisposeMenu (labelMenu);
    DisposeMenu (selectMenu);
    DisposeMenu (compMenu);

    compMenu        = GetMenuHandle (130);
    DeleteMenuItem (compMenu,10);
    DeleteMenuItem (compMenu,10);
    DeleteMenuItem (compMenu,10);

    _HYWindow::_UnsetMenuBar();
}

//EOF