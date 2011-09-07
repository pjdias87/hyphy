/*
    Mac OS Portions of the picture window class

    Sergei L. Kosakovsky Pond, Spring 2000 - December 2002.
*/

#include "HYGWindow.h"

#include "QuickTimeComponents.h"
#include "ImageCompression.h"
#include "Scrap.h"

#include "errorfns.h"

#define  HY_PWINDOW_MENU_ID 7333
//__________________________________________________________________
// _HYPlatformPWindow
//__________________________________________________________________

_HYPlatformPWindow::_HYPlatformPWindow          (void)
{
    savedPic = nil;
    savedClip = nil;
}

//__________________________________________________________________

_HYPlatformPWindow::~_HYPlatformPWindow         (void)
{
    if (savedPic) {
        KillPicture (savedPic);
    }
    if (savedClip) {
        DisposeRgn (savedClip);
    }
}

//__________________________________________________________________

void    _HYPlatformPWindow::_StartPicture   (void)
{
    _HYPWindow*     theParent = (_HYPWindow*)    this;
    Rect            picDim = {0,0,0,0};
    picDim.right  = theParent->w;
    picDim.bottom = theParent->h;

    if (savedPic) {
        KillPicture (savedPic);
    }

    if (savedClip) {
        DisposeRgn (savedClip);
    }

    savedClip = NewRgn ();
    GetClip      (savedClip);
    ClipRect     (&picDim);
    OpenCPicParams pp;
    pp.srcRect = picDim;
    pp.hRes = pp.vRes = 75;
    pp.version = -2;
    checkPointer (savedPic = OpenPicture (&picDim));
}

//__________________________________________________________________

void    _HYPlatformPWindow::_EndPicture     (void)
{
    if (savedPic) {
        ClosePicture ();
        _HYPWindow*     theParent = (_HYPWindow*)    this;
        Rect            picDim = {0,0,0,0};
        picDim.right  = theParent->w;
        picDim.bottom = theParent->h;
        DrawPicture  (savedPic,&picDim);
    }
    if (savedClip) {
        SetClip (savedClip);
        DisposeRgn (savedClip);
        savedClip = nil;
    }
}

//__________________________________________________________________

void    _HYPlatformPWindow::_DrawPicture    (_HYRect r)
{
    Rect R = HYRect2Rect (r);
    if (savedPic) {
        DrawPicture (savedPic,&R);
    }
}

//__________________________________________________________________
// _HYPWindow
//__________________________________________________________________

void        _HYPWindow::_PrintPWindow(void)
{
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

    GetPort(&savePort);

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
                return;
            }

            theStatus = PMSessionBeginDocument(hyPC,gPrintSettings, gPageFormat);
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
        _String errMsg ("Could not print the picture. Error Code:");
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
#ifdef TARGET_API_MAC_CARBON
            theStatus = PMSessionBeginPage(hyPC,gPageFormat, NULL);
            if (theStatus != noErr) {
                return;
            }
            GrafPtr ppPort;
            PMSessionGetGraphicsContext (hyPC, NULL, (void**)&ppPort);
            SetPort (ppPort);
#else
            PrOpenPage      (printPort, nil);
#endif

            _Parameter sx = 1., sy = 1.;

            if (printW<w) {
                sx = (_Parameter)printW/w;
            }
            if (printH<h) {
                sy = (_Parameter)printH/h;
            }

            if (sx>sy) {
                sx = sy;
            } else {
                sy = sx;
            }

            _HYRect         drect = {0,0,h*sy,w*sx,0};
            drect.left   = (printW-drect.right)/2;
            drect.right += drect.left;
            drect.top    = (printH-drect.bottom)/2;
            drect.bottom+= drect.top;

            _DrawPicture    (drect);

#ifdef TARGET_API_MAC_CARBON
            theStatus = PMSessionEndPage(hyPC);
            if (theStatus != noErr) {
                return;
            }
#else
            PrClosePage     (printPort);
#endif


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

void _HYPWindow::_SetWindowRectangle(int top, int left, int bottom, int right, bool ss)
{
    _HYPlatformWindow::_SetWindowRectangle (top,left,bottom,right, ss);
    SetPaneSize     (bottom-top,right-left,depth);
    SetContentSize  (right-left,bottom-top);
}

//__________________________________________________________________

void    _HYPWindow::_Paint (Ptr )
{
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
#else
    SetPort(theWindow);
#endif
    Rect srcRect  = {0,0,h,w},
         destRect = srcRect;

    LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
    CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(GetWindowPort(theWindow)),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#else
    CopyBits ((BitMap*)*GetGWorldPixMap(thePane),&theWindow->portBits,&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#endif
    UnlockPixels (GetGWorldPixMap(thePane));
    DrawGrowIcon (theWindow);
    SetPort (savedPort);
}

//__________________________________________________________________

void    _HYPWindow::_Update (Ptr )
{
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
#else
    SetPort(theWindow);
#endif
    BeginUpdate(theWindow);

    Rect srcRect  = {0,0,h,w},
         destRect = srcRect;

    LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
    CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(GetWindowPort(theWindow)),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#else
    CopyBits ((BitMap*)*GetGWorldPixMap(thePane),&theWindow->portBits,&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#endif
    UnlockPixels (GetGWorldPixMap(thePane));
    DrawGrowIcon (theWindow);

    EndUpdate(theWindow);
    SetPort (savedPort);
}

//__________________________________________________________________

long _HYPWindow::_Grow(Ptr theData)
{
    EventRecord* theEvent = (EventRecord*)theData;
    Rect         sizeRect;
    sizeRect.top    = 10;
    sizeRect.left   = 10;
    sizeRect.bottom = 0x7fff;
    sizeRect.right  = 0x7fff;

    return       GrowWindow  (theWindow,theEvent->where,&sizeRect);
}

//__________________________________________________________________

bool        _HYPWindow::_ProcessMenuSelection (long msel)
{
    if (_HYWindow::_ProcessMenuSelection(msel)) {
        return true;
    }

    long        menuChoice = msel&0x0000ffff;
    bool        done = false;

    switch (msel/0xffff) {
    case 129: { // file menu
        if (menuChoice==8) { // print
            _PrintPWindow();
            done = true;
        }
        break;
    }

    case HY_PWINDOW_MENU_ID: {
        done = true;
        if (menuChoice == 1) {
            Zoom (1.1);
        } else if (menuChoice == 2) {
            Zoom (.9);
        } else if (menuChoice == 3) {
            OriginalSize();
        }
        break;
    }
    }

    if (!done) {
        return _HYGWindow::_ProcessMenuSelection (msel);
    }

    HiliteMenu(0);
    InvalMenuBar();

    return done;
}

//__________________________________________________________________

void _HYPWindow::_SetMenuBar(void)
{
    _HYGWindow::_SetMenuBar();
    MenuHandle pMenu = GetMenuHandle (HY_PWINDOW_MENU_ID);
    if (!pMenu) {
        pMenu = NewMenu(HY_PWINDOW_MENU_ID,"\pImage");
        if (!pMenu) {
            warnError (-108);
        }

        InsertMenuItem (pMenu,"\pEnlarge/1",0);
        InsertMenuItem (pMenu,"\pShrink/2",10000);
        InsertMenuItem (pMenu,"\pOriginal Size/3",10000);
        InsertMenu (pMenu,132);
    }
    InvalMenuBar();
}

//__________________________________________________________________

void _HYPWindow::_UnsetMenuBar(void)
{
    MenuHandle t = GetMenuHandle (HY_PWINDOW_MENU_ID);

    DeleteMenu  (HY_PWINDOW_MENU_ID);
    DisposeMenu (t);

    _HYGWindow::_UnsetMenuBar();
}

//__________________________________________________________________
bool _HYPWindow::_ProcessOSEvent (Ptr vEvent)
{
    return _HYPlatformWindow::_ProcessOSEvent (vEvent);
}

//EOF