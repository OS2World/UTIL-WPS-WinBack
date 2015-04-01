/*------------------------------------------------------------------
   This module is the input hook.  Since it is a system input hook,
   it must reside in a DLL.
   This hook checks if the 2nd button double clicks on the
   title bar.  If it was, then put the window at the bottom.
   After we get a WM_BUTTON2DBLCLK:
       get the anchor block of the window that was double clicked
       get the desktop window handle
       get the parent of the window that was double clicked
       if the parent of the window that was double clicked
          is the desktop then we don't want it - we want the title bar
       if the title bar was double-clicked then send the window
          to the bottom of the pile
-----------------------------------------------------------------*/

#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#include <os2.h>


int            _acrtused = 0;

BOOL EXPENTRY WinHookProc(HAB hab,PQMSG pqmsg,USHORT usRemove )
  {
  HWND        hwndDesktop;          /* desktop window handle    */
  HWND        hwndTop;              /* top window handle    */
  HWND        hwndTitle;            /* title bar  window handle    */
  HAB         habWin;               /* anchor block of this window   */

    /* Only care if 2nd button double clicks */

  if( pqmsg->msg == WM_BUTTON2DBLCLK)
  {

      /* get the anchor block of the window that was double clicked */
      habWin = WinQueryAnchorBlock(pqmsg->hwnd);
      /* get the desktop window handle */
      hwndDesktop = WinQueryDesktopWindow(habWin,NULLHANDLE);
      /* get the parent of the window that was double clicked */
      hwndTop = WinQueryWindow(pqmsg->hwnd,QW_PARENT);
      /*------------------------------------------------------
         if the parent of the window that was double clicked
         is the desktop then we don't want it - we want the
         title bar
      -------------------------------------------------------*/
      if (hwndTop != hwndDesktop)
      {
         hwndTitle = WinWindowFromID ( hwndTop,FID_TITLEBAR);
         /* if the title bar was double-clicked then process */
         if (hwndTitle == pqmsg->hwnd)
               /* send the window to the bottom of the pile */
               WinSetWindowPos(hwndTop,HWND_BOTTOM,0,0,0,0,SWP_ZORDER|SWP_DEACTIVATE);
      }
  }
  return(FALSE);

}
