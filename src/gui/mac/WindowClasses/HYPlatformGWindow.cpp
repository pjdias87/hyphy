/*
    Mac OS Portions of the graphics window class

    Sergei L. Kosakovsky Pond, Spring 2000 - December 2002.
*/

#include "HYGWindow.h"

#include "QuickTimeComponents.h"
#include "ImageCompression.h"
#include "Scrap.h"

#include "errorfns.h"


//__________________________________________________________________

void    _HYGWindow::_Paint (Ptr )
{
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
#else
    SetPort(theWindow);
#endif
    Rect srcRect,
#ifdef OPAQUE_TOOLBOX_STRUCTS
         destRect;
    GetWindowBounds (theWindow,kWindowGlobalPortRgn,&destRect);
    OffsetRect (&destRect,-destRect.left,-destRect.top);
#else
         destRect = theWindow->portRect;
#endif
    destRect.right-=15;
    destRect.bottom-=15;
    int a,b,c,d;
    _VisibleContents (a,b,c,d);
    srcRect.top=a;
    srcRect.left=b;
    srcRect.bottom=c;
    srcRect.right=d;
    LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
    CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(GetWindowPort(theWindow)),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#else
    CopyBits ((BitMap*)*GetGWorldPixMap(thePane),&theWindow->portBits,&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#endif
    UnlockPixels (GetGWorldPixMap(thePane));
    DrawGrowIcon (theWindow);
    ShowControl (hScroll);
    ShowControl (vScroll);
    DrawControls(theWindow);
    SetPort (savedPort);
}

//__________________________________________________________________

void    _HYGWindow::_Update (Ptr )
{
    Rect srcRect,
#ifdef OPAQUE_TOOLBOX_STRUCTS
         destRect;
    GetWindowBounds (theWindow,kWindowGlobalPortRgn,&destRect);
    OffsetRect (&destRect,-destRect.left,-destRect.top);
#else
         destRect = theWindow->portRect;
#endif
    destRect.right-=15;
    destRect.bottom-=15;
    GrafPtr savedPort;
    GetPort(&savedPort);
#ifdef OPAQUE_TOOLBOX_STRUCTS
    SetPort(GetWindowPort(theWindow));
    InvalWindowRect(theWindow,&destRect);
#else
    SetPort(theWindow);
    InvalRect(&theWindow->portRect);
#endif
    BeginUpdate(theWindow);
    int     li, ri, ti, bi;
    _VisibleContents (ti,li,bi,ri);
    srcRect.right = ri;
    srcRect.left = li;
    srcRect.top = ti;
    srcRect.bottom = bi;
    LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
    CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(GetWindowPort(theWindow)),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#else
    CopyBits (&(GrafPtr(thePane)->portBits),&(theWindow->portBits),&srcRect,&destRect,srcCopy,(RgnHandle)nil);
#endif
    UnlockPixels (GetGWorldPixMap(thePane));
    DrawGrowIcon (theWindow);
    DrawControls(theWindow);
    EndUpdate(theWindow);
    SetPort (savedPort);
}


//__________________________________________________________________


bool        _HYGWindow::_ProcessMenuSelection (long msel)
{
    if (_HYWindow::_ProcessMenuSelection(msel)) {
        return true;
    }

    long        menuChoice = msel&0x0000ffff;
    bool        done = false;

    switch (msel/0xffff) {
    case 129: { // file menu
        if (menuChoice==4) { // save
            _SaveGWindow ();
            done = true;
        } else if (menuChoice==8) { // print
            _PrintGWindow();
            done = true;
        }
        break;
    }
    case 130: {
        if (menuChoice == 4) {
            done = true;
            _CopyToClipboard();
            break;
        }
    }
    }

    HiliteMenu(0);
    InvalMenuBar();

    return done;
}


//__________________________________________________________________

void        _HYGWindow::_SaveGWindow (void)
{
    _SavePicture(GetTitle());
}

//__________________________________________________________________

void        _HYGWindow::_PrintGWindow (void)
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
#else
            PrOpenPage      (printPort, nil);
#endif

            Rect    dest,
                    src = {0,0,h,w};

            if (w<printW) {
                dest.left = (printW-w)/2;
                dest.right = dest.left+w;
            } else {
                dest.left = 0;
                dest.right = printW;
            }
            if (h<printH) {
                dest.top = (printH-h)/2;
                dest.bottom = dest.left+h;
            } else {
                dest.top = 0;
                dest.bottom = printH;
            }

            LockPixels (GetGWorldPixMap(thePane));
#ifdef OPAQUE_TOOLBOX_STRUCTS
            GrafPtr         prPortPtr;
            PMSessionGetGraphicsContext (hyPC, NULL, (void**)&prPortPtr);
            CopyBits (GetPortBitMapForCopyBits(thePane),GetPortBitMapForCopyBits(prPortPtr),&src,&dest,srcCopy,(RgnHandle)nil);
#else
            CopyBits (&(GrafPtr(thePane)->portBits),&(printPort->gPort.portBits),&src,&dest,srcCopy,(RgnHandle)nil);
#endif
            UnlockPixels (GetGWorldPixMap(thePane));

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

void _HYGWindow::_SetMenuBar(void)
{
    _HYWindow::_SetMenuBar();
    MenuHandle  t = GetMenuHandle (130);
    EnableMenuItem (t,4);
    InvalMenuBar();
}

//__________________________________________________________________

void _HYGWindow::_UnsetMenuBar(void)
{
    MenuHandle  t = GetMenuHandle (130);
    DisableMenuItem (t,4);
    _HYWindow::_UnsetMenuBar();
}
//EOF