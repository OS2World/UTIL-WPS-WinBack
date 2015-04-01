/*-----------------------------------------------------------------
  This module loads and releases the System Hook DLL, WINHOOK.DLL.
------------------------------------------------------------------*/

#define INCL_DOS
#define INCL_ERRORS
#define INCL_PM

#include <os2.h>
#include <stdio.h>
#include <string.h>

#include "winback.h"

HAB hab;
HMODULE hmodule = NULLHANDLE;
PFN pfnInputHook;

INT main()
{
   HMQ hmq;
   HWND hwndFrame,hwndClient;
   QMSG qmsg;
   ULONG frameflags ;
   HMTX hmtx;                                    /* handle to semaphore */
   if (DosCreateMutexSem("\\SEM32\\WINBACK.SEM", /* name of semaphore   */
                    &hmtx,                       /* address of handle   */
                    0,                           /* system-wide known   */
                    TRUE))                       /* initially owned     */
   {
      DosBeep(600,200);                          /* Already running     */
      DosExit(EXIT_PROCESS,1);                   /* Terminate           */
   }

   hab = WinInitialize (0);
   hmq = WinCreateMsgQueue (hab,0);

   WinRegisterClass (hab,                    /* Anchor block handle            */
                     "WinBackClass",         /* Name of class being registered */
                      WinbackWndProc,        /* Window procedure for class     */
                      0,                     /* Class style                    */
                      0);                    /* Extra bytes to reserve         */

   hwndFrame = WinCreateStdWindow (HWND_DESKTOP,  /* parent */
                                   0,             /* Invisible */
                                   &frameflags,   /* Pointer to control data */
                                   "WinBackClass",/* Client window class name */
                                   NULL,          /* titlebar text */
                                   0L,            /* Style of client window*/
                                   NULLHANDLE,    /* Module handle for resources */
                                   0,             /* resource id */
                                   &hwndClient);  /* Pointer to client window handle */

   {
     SWCNTRL swctl ;
     PID pid ;

     WinQueryWindowProcess ( hwndFrame, &pid, NULL ) ;
     swctl.hwnd = hwndFrame ;
     swctl.hwndIcon = NULLHANDLE ;
     swctl.hprog = NULLHANDLE ;
     swctl.idProcess = pid ;
     swctl.idSession = 0 ;
     swctl.uchVisibility = SWL_VISIBLE ;
     swctl.fbJump = SWL_JUMPABLE ;
     strcpy ( swctl.szSwtitle, "WinBack 2.0" ) ;

     WinAddSwitchEntry ( &swctl ) ;
   }

  while (WinGetMsg(hab,&qmsg,NULLHANDLE,0,0))
    WinDispatchMsg(hab,&qmsg);

  WinReleaseHook (hab,                       /* Make sure hook is released */
                  NULLHANDLE,
                  HK_INPUT,
                  pfnInputHook,
                  hmodule);

  WinDestroyWindow (hwndFrame);

  WinDestroyMsgQueue (hmq);

  WinTerminate (hab);

  return 0;
  }

MRESULT EXPENTRY WinbackWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{

   switch (msg)
   {
   case WM_CREATE:
   {
      ULONG rc;                              /* return code */
      WinPostMsg(hwnd,WM_USER,0L,0L);

      rc = DosLoadModule( NULL,              /* Load the DLL module */
                      0,                     /* containing the Hook */
                      "Winhook",             /* procedure (must use a DLL). */
                      &hmodule);             /* Get module handle */
      if (rc)
      {
          WinMessageBox (HWND_DESKTOP,hwnd,
                         "There was an error in loading WINHOOK.DLL",
                         "DLL could not be loaded",
                         0,
                         MB_OK | MB_ICONEXCLAMATION);

          WinPostMsg(hwnd,WM_CLOSE,0L,0L);
      }
      else
      {
          rc = DosQueryProcAddr( hmodule,      /* Get address of Hook Proc. */
                                 0,
                                 "WINHOOKPROC",     /* FOR THE WINSETHOOK AND */
                                 &pfnInputHook );   /* WinReleaseHook calls.     */
          if(rc)
          {
             WinMessageBox (HWND_DESKTOP,
                            hwnd,
                            "There is an error in WINHOOK.DLL",
                            "Error in DLL",
                            0,
                            MB_OK | MB_ICONEXCLAMATION);

             WinPostMsg(hwnd,WM_CLOSE,0L,0L);
          }
          else
          {
             WinSetHook (hab,
                         NULLHANDLE,         /* set hook to capture */
                         HK_INPUT,           /* input messages for filtering. */
                         pfnInputHook,
                         hmodule);
         }
      }
      return FALSE;
   }
   case WM_USER:
   {
      WinDlgBox (HWND_DESKTOP,hwnd,InfoDlgProc,NULLHANDLE,InfoBox,NULL);
      return (MRESULT) TRUE;
   }
   case WM_QUIT:
   case WM_CLOSE:
   case WM_DESTROY:
   {
      WinReleaseHook (hab,                   /* Make sure hook is released */
                      NULLHANDLE,
                      HK_INPUT,
                      pfnInputHook,
                      hmodule);
   }

   }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

MRESULT EXPENTRY InfoDlgProc(HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)

{
   switch (msg)
   {
      case WM_INITDLG:
         WinStartTimer (hab,hwnd,1,4000);
         break;
      case WM_TIMER:
         WinStopTimer (hab,hwnd,1);
         WinDismissDlg (hwnd,0);
         break;
   }
   return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

