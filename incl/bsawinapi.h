//--------------------------------------------------------------------
//
// File: bsawinapi.h
// Desc: Windows(tm) emulation for X11
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#if !defined(_BSAWINAPI_H_)&&!defined(Windows)
#define _BSAWINAPI_H_ 1

#define BSA_WINAPI 1

#define FAR

#ifdef __cplusplus
extern "C"
{
#endif

typedef void* 	HDC;
typedef void* 	HWND;
typedef void* 	HINSTANCE;
typedef void* 	HFONT;
typedef void* 	HANDLE, *HGLOBAL;
typedef WORD 	ATOM;
typedef void*	HMENU;
typedef void*	HICON;
typedef void*	HCURSOR;
typedef void*	HBRUSH;
typedef void*	HPEN;
typedef void*	HBITMAP;
typedef void*	HGDIOBJ;

// LPARAM and WPARAM have to pass void*'s so make
// them 64 bits on machines with 64 bit addresses on
// LONG has to agree with LPARAM for things like getwindowslong
// but DWORD has to be 4 bytes still
#if defined(__X86_64__) || _WIN64_ || __amd64__ || __arm64__
typedef long long 	LPARAM, LRESULT, LONG;
typedef unsigned long long 	WPARAM;
#else
typedef long  	LPARAM, LRESULT, LONG;
typedef unsigned int 	WPARAM;
#endif
typedef unsigned int UINT;
typedef int		INT, FAR *LPINT;
#ifdef COCOA
#ifdef __arm64__
typedef bool BOOL;
#else
typedef signed char BOOL;
#endif
#else
typedef int   	BOOL;
#endif
typedef UINT 	*UINT_PTR;

#define MAKEINTRESOURCE(a) ((LPTSTR)a)

#define TRUE	1
#define FALSE	0

#define WINAPI
#define CALLBACK
#define APIENTRY
#define CONST

#define LOWORD(w) ((WORD)((w)&0xffff))
#define HIWORD(w) ((WORD)(((w)>>16)&0xffff))

#define MAKEULONG(l, h) ((DWORD)(((WORD)(l)) | ((DWORD)((WORD)(h))) << 16))
#define MAKELONG(l, h)	((LONG)MAKEULONG(l, h))

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL    (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);

// window class struct for register class
//
typedef struct tagWNDCLASS
{
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCTSTR     lpszMenuName;
    LPCTSTR     lpszClassName;
}
WNDCLASS, *PWNDCLASS, *LPWNDCLASS;

typedef struct
{
	int top, left, bottom, right;
}
RECT, *LPRECT;

typedef struct
{
	long x, y;
}
POINT, FAR *LPPOINT;

typedef struct tagMSG
{
    HWND        	hwnd;
    UINT        	message;
    WPARAM      	wParam;
    LPARAM      	lParam;
    DWORD       	time;
    POINT      		pt;
    struct tagMSG* 	next;
} MSG, *PMSG,FAR *LPMSG;

typedef struct tagCREATESTRUCT {
    void*      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCTSTR     lpszName;
    LPCTSTR     lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCT, *LPCREATESTRUCT;

typedef struct tagNMHDR
{
    HWND  hwndFrom;
    UINT  idFrom;
    UINT  code;
}
NMHDR, FAR *LPNMHDR;


#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3

#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define SW_HIDE             0
#define SW_MAXIMIZE         3
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_RESTORE          9

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005

#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SHOWWINDOW                   0x0018
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_NOTIFY                       0x004E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
#define WM_GETDLGCODE                   0x0087
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_MENUSELECT                   0x011F
#define WM_MENUCHAR                     0x0120
#define WM_ENTERIDLE                    0x0121
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_MOUSELAST                    0x020A
#define WHEEL_DELTA                     120     /* Value for rolling one detent */
#define WHEEL_PAGESCROLL                (UINT_MAX) /* Scroll one page */

#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3

#define WM_CUT                          0x0300
#define WM_COPY                         0x0301
#define WM_PASTE                        0x0302
#define WM_CLEAR                        0x0303
#define WM_UNDO                         0x0304

#define WM_USER                         0x0400

#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010

#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4

// Window Styles
//
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L     /* WS_BORDER | WS_DLGFRAME  */
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_NOPARENTNOTIFY    0x00000004L
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L

#define WS_EX_RIGHT             0x00001000L
#define WS_EX_LEFT              0x00000000L
#define WS_EX_RTLREADING        0x00002000L
#define WS_EX_LTRREADING        0x00000000L
#define WS_EX_LEFTSCROLLBAR     0x00004000L
#define WS_EX_RIGHTSCROLLBAR    0x00000000L

#define WS_EX_CONTROLPARENT     0x00010000L
#define WS_EX_STATICEDGE        0x00020000L
#define WS_EX_APPWINDOW         0x00040000L
#define WS_EX_LAYERED			0x00080000L

#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

// Common Window Styles
//
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)

#define WS_POPUPWINDOW      (WS_POPUP          | \
                             WS_BORDER         | \
                             WS_SYSMENU)

#define WS_CHILDWINDOW      (WS_CHILD)

#define SS_BITMAP 14
#define SS_BLACKFRAME 7
#define SS_BLACKRECT 4
#define SS_CENTER 1
#define SS_CENTERIMAGE 512
#define SS_ENHMETAFILE 15
#define SS_ETCHEDFRAME 18
#define SS_ETCHEDHORZ 16
#define SS_ETCHEDVERT 17
#define SS_GRAYFRAME 8
#define SS_GRAYRECT 5
#define SS_ICON 3
#define SS_LEFT 0
#define SS_LEFTNOWORDWRAP 0xc
#define SS_NOPREFIX 128
#define SS_NOTIFY 256
#define SS_OWNERDRAW 0xd
#define SS_REALSIZEIMAGE 0x800
#define SS_RIGHT 2
#define SS_RIGHTJUST 0x400
#define SS_SIMPLE 11
#define SS_SUNKEN 4096
#define SS_WHITEFRAME 9
#define SS_WHITERECT	6
#define SS_USERITEM	10
#define SS_TYPEMASK	0x0000001FL
#define SS_ENDELLIPSIS	0x00004000L
#define SS_PATHELLIPSIS	0x00008000L
#define SS_WORDELLIPSIS	0x0000C000L
#define SS_ELLIPSISMASK 0x0000C000L

// Class styles
//
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002

// Predefined Clipboard Formats
//
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_DIB              8
#define CF_UNICODETEXT      13

typedef struct tagPAINTSTRUCT {
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[32];
} PAINTSTRUCT, FAR *LPPAINTSTRUCT;

BOOL WINAPI UnionRect(LPRECT lpDest, CONST RECT* lpA, CONST RECT* lpB);
BOOL WINAPI IntersectRect(LPRECT lpDest, CONST RECT* lpA, CONST RECT* lpB);

BOOL WINAPI GetMessage(
					    LPMSG lpMsg,
					    HWND hWnd ,
					    UINT wMsgFilterMin,
					    UINT wMsgFilterMax
					   );

BOOL WINAPI TranslateMessage(CONST MSG *lpMsg);
LONG WINAPI DispatchMessage(CONST MSG *lpMsg);
BOOL WINAPI PeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

#define PM_NOREMOVE     0x0000
#define PM_REMOVE       0x0001
#define PM_NOYIELD      0x0002

#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004
#define MOD_WIN         0x0008


LRESULT WINAPI	SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI		PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI	DefWindowProc(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam);
#define			CallWindowProc(proc, hWnd, Msg, wParam, lParam)	\
					((WNDPROC)proc)(hWnd, Msg, wParam, lParam)
void WINAPI		PostQuitMessage(int nExitCode);

ATOM WINAPI		RegisterClass(CONST WNDCLASS *lpWndClass);
BOOL WINAPI		UnregisterClass(LPCTSTR lpClassName, HINSTANCE hInstance);

HWND WINAPI		CreateWindowEx(
								DWORD 		dwExStyle,
								LPCTSTR 	lpClassName,
								LPCTSTR 	lpWindowName,
								DWORD 		dwStyle,
								int 		X,
								int 		Y,
								int 		nWidth,
								int 		nHeight,
								HWND 		hWndParent,
								HMENU 		hMenu,
								HINSTANCE 	hInstance,
								void* 		lpParam
							  );

#define CW_USEDEFAULT -1
#define CreateWindow(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam) \
		CreateWindowEx(0L, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)

BOOL WINAPI GetClientRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI GetWindowRect(HWND hWnd, LPRECT lpRect);
HWND WINAPI GetDesktopWindow(void);

int WINAPI GetWindowText(
					    HWND hWnd,
					    LPTSTR lpString,
					    int nMaxCount);

int WINAPI SetWindowText(HWND hWnd, LPCTSTR lpString);


BOOL WINAPI DestroyWindow(HWND hWnd);
BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow);
BOOL WINAPI CloseWindow(HWND hWnd);
BOOL WINAPI MoveWindow(
						HWND hWnd,
						int X,
						int Y,
						int nWidth,
						int nHeight,
						BOOL bRepaint);

BOOL WINAPI SetCursorPos(int x, int y);
HCURSOR WINAPI SetCursor(HCURSOR hCursor);
BOOL WINAPI GetCursorPos(LPPOINT lpPoint);

BOOL WINAPI CreateCaret(
					    HWND hWnd,
					    HBITMAP hBitmap,
					    int nWidth,
					    int nHeight);

UINT WINAPI GetCaretBlinkTime(void);
BOOL WINAPI SetCaretBlinkTime(UINT uMSeconds);
BOOL WINAPI DestroyCaret(void);
BOOL WINAPI HideCaret(HWND hWnd);
BOOL WINAPI ShowCaret(HWND hWnd);
BOOL WINAPI SetCaretPos(int X, int Y);
BOOL WINAPI GetCaretPos(LPPOINT lpPoint);
int  WINAPI MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);

typedef void CALLBACK (*TIMERPROC)(HWND, UINT, UINT, DWORD);
UINT_PTR WINAPI SetTimer(HWND hWnd, UINT id, UINT elapse, TIMERPROC proc);
BOOL WINAPI 	KillTimer(HWND hWnd, UINT_PTR id);

typedef struct
{
    DWORD style;
    DWORD dwExtendedStyle;
    WORD cdit;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE;

typedef DLGTEMPLATE *LPDLGTEMPLATE;
typedef CONST DLGTEMPLATE *LPCDLGTEMPLATE;

typedef struct
{
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
}
DLGITEMTEMPLATE;

typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATE;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATE;

typedef struct tagDLGTEMPLATEEX
{
    WORD wDlgVer;
    WORD wSignature;
    DWORD dwHelpID;
    DWORD dwExStyle;
    DWORD dwStyle;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
}
DLGTEMPLATEEX;

typedef DLGTEMPLATEEX *LPDLGTEMPLATEEX;
typedef CONST DLGTEMPLATEEX *LPCDLGTEMPLATEEX;

typedef struct tagDLGITEMTEMPLATEEX
{
    DWORD dwHelpID;
    DWORD dwExStyle;
    DWORD dwStyle;
    short x;
    short y;
    short cx;
    short cy;
    DWORD dwID;
}
DLGITEMTEMPLATEEX;

typedef DLGITEMTEMPLATEEX *PDLGITEMTEMPLATEEX;
typedef DLGITEMTEMPLATEEX *LPDLGITEMTEMPLATEEX;

HWND WINAPI CreateDialogParam(
							HINSTANCE	hInstance,
							LPCTSTR		lpTemplateName,
							HWND		hWndParent,
							DLGPROC		lpDialogFunc,
							LPARAM		dwInitParam);

#define CreateDialog(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

HWND WINAPI CreateDialogIndirectParam(
						    HINSTANCE		hInstance,
						    LPCDLGTEMPLATE	hDialogTemplate,
						    HWND			hWndParent,
						    DLGPROC			lpDialogFunc,
						    LPARAM			dwInitParam);

#define CreateDialogIndirect(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogIndirectParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

int WINAPI DialogBoxParam(
							HINSTANCE	hInstance,
							LPCTSTR		lpTemplateName,
							HWND		hWndParent,
							DLGPROC		lpDialogFunc,
							LPARAM		dwInitParam);

#define DialogBox(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

int WINAPI DialogBoxIndirectParam(
						    HINSTANCE		hInstance,
						    LPCDLGTEMPLATE	hDialogTemplate,
						    HWND			hWndParent,
						    DLGPROC			lpDialogFunc,
						    LPARAM			dwInitParam);

#define DialogBoxIndirect(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxIndirectParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

BOOL WINAPI EndDialog(HWND hDlg, int nResult);
HWND WINAPI GetDlgItem(HWND hDlg, int nIDDlgItem);
int  WINAPI GetDlgItemText(HWND hDlg, int nIDDlgItem, LPTSTR text, int nText);
int  WINAPI IsDlgButtonChecked(HWND hDlg, int nIDButton);
int  WINAPI SetDlgItemText(HWND hDlg, int nIDDlgItem, LPCTSTR text);
int  WINAPI CheckDlgButton(HWND hDlg, int nIDButton, UINT check);

LRESULT WINAPI DefDlgProc(
						    HWND hDlg,
						    UINT Msg,
						    WPARAM wParam,
						    LPARAM lParam);

BOOL WINAPI OpenClipboard(HWND hWndNewOwner);
BOOL WINAPI CloseClipboard(void);
HANDLE WINAPI SetClipboardData(UINT uFormat, HANDLE hMem);
HANDLE WINAPI GetClipboardData(UINT uFormat);
BOOL WINAPI EmptyClipboard(void);

HWND WINAPI SetFocus(HWND hWnd);
HWND WINAPI GetActiveWindow(void);
HWND WINAPI GetFocus(void);

HWND WINAPI GetCapture(void);
HWND WINAPI SetCapture(HWND hWnd);
BOOL WINAPI ReleaseCapture(void);

#define TME_HOVER       0x00000001
#define TME_LEAVE       0x00000002
#define TME_QUERY       0x40000000
#define TME_CANCEL      0x80000000

#define HOVER_DEFAULT   0xFFFFFFFF

typedef struct tagTRACKMOUSEEVENT
{
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
}
TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

BOOL WINAPI TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack);

typedef struct tagDOCINFO
{
    int      cbSize;
    LPCTSTR  lpszDocName;
    LPCTSTR  lpszOutput;
    LPCWSTR  lpszDatatype;
    DWORD    fwType;
}
DOCINFO, FAR* LPDOCINFO;

#define DI_APPBANDING               0x00000001
#define DI_ROPS_READ_DESTINATION    0x00000002

#define DRIVERVERSION 0     // Device driver version
#define TECHNOLOGY    2     // Device classification
#define HORZSIZE      4     // Horizontal size in millimeters
#define VERTSIZE      6     // Vertical size in millimeters
#define HORZRES       8     // Horizontal width in pixels
#define VERTRES       10    // Vertical height in pixels
#define BITSPIXEL     12    // Number of bits per pixel
#define PLANES        14    // Number of planes
#define NUMBRUSHES    16    // Number of brushes the device has
#define NUMPENS       18    // Number of pens the device has
#define NUMMARKERS    20    // Number of markers the device has
#define NUMFONTS      22    // Number of fonts the device has
#define NUMCOLORS     24    // Number of colors the device supports
#define PDEVICESIZE   26    // Size required for device descriptor
#define CURVECAPS     28    // Curve capabilities
#define LINECAPS      30    // Line capabilities
#define POLYGONALCAPS 32    // Polygonal capabilities
#define TEXTCAPS      34    // Text capabilities
#define CLIPCAPS      36    // Clipping capabilities
#define RASTERCAPS    38    // Bitblt capabilities
#define ASPECTX       40    // Length of the X leg
#define ASPECTY       42    // Length of the Y leg
#define ASPECTXY      44    // Length of the hypotenuse

#define LOGPIXELSX    88    // Logical pixels/inch in X
#define LOGPIXELSY    90    // Logical pixels/inch in Y

#define SIZEPALETTE  104    // Number of entries in physical palette
#define NUMRESERVED  106    // Number of reserved entries in palette
#define COLORRES     108    // Actual color resolution

#define PHYSICALWIDTH   110 // Physical Width in device units
#define PHYSICALHEIGHT  111 // Physical Height in device units
#define PHYSICALOFFSETX 112 // Physical Printable Area x margin
#define PHYSICALOFFSETY 113 // Physical Printable Area y margin
#define SCALINGFACTORX  114 // Scaling factor x
#define SCALINGFACTORY  115 // Scaling factor y

#define DT_RASDISPLAY       1   // Raster display
#define DT_RASPRINTER       2   // Raster printer

#define CCHDEVICENAME 32
#define CCHFORMNAME   32

// DEVMODE (not quite like Window's version, holds driver/device info
typedef struct
{
    BYTE	dmDeviceName[CCHDEVICENAME];
    WORD	dmSpecVersion;
    WORD	dmDriverVersion;
    WORD	dmSize;
    WORD	dmDriverExtra;
    DWORD	dmFields;

	short	dmOrientation;
    short	dmPaperSize;
    short	dmPaperLength;
    short	dmPaperWidth;

    short	dmScale;
    short	dmCopies;
    short	dmDefaultSource;
    short	dmPrintQuality;
    short	dmColor;
    short	dmDuplex;
    short	dmYResolution;
    short	dmTTOption;
    short	dmCollate;

    BYTE	dmFormName[CCHFORMNAME];
    WORD	dmLogPixels;
    DWORD	dmBitsPerPel;
    DWORD	dmPelsWidth;
    DWORD	dmPelsHeight;
    DWORD	dmDisplayFlags;
    DWORD	dmDisplayFrequency;
}
DEVMODE, *PDEVMODE, *NPDEVMODEA, FAR* LPDEVMODE;

typedef struct
{
   WORD wDriverOffset;
   WORD wDeviceOffset;
   WORD wOutputOffset;
   WORD wDefault;
}
DEVNAMES, FAR* LPDEVNAMES;

// field in devmode flags
#define DM_ORIENTATION      0x00000001L
#define DM_PAPERSIZE        0x00000002L
#define DM_PAPERLENGTH      0x00000004L
#define DM_PAPERWIDTH       0x00000008L
#define DM_SCALE            0x00000010L
#define DM_COPIES           0x00000100L
#define DM_DEFAULTSOURCE    0x00000200L
#define DM_PRINTQUALITY     0x00000400L
#define DM_COLOR            0x00000800L
#define DM_DUPLEX           0x00001000L
#define DM_YRESOLUTION      0x00002000L
#define DM_TTOPTION         0x00004000L
#define DM_COLLATE          0x00008000L
#define DM_FORMNAME         0x00010000L
#define DM_LOGPIXELS        0x00020000L
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L

#define DMPAPER_LETTER               1		// Letter 8 1/2 x 11 in
#define DMPAPER_LEDGER               4		// Ledger 17 x 11 in
#define DMPAPER_LEGAL                5		// Legal 8 1/2 x 14 in
#define DMPAPER_EXECUTIVE            7		// Executive 7 1/4 x 10 1/2 in
#define DMPAPER_A3                   8		// A3 297 x 420 mm
#define DMPAPER_A4                   9		// A4 210 x 297 mm
#define DMPAPER_A5                  11		// A5 148 x 210 mm
#define DMPAPER_B4                  12		// B4 (JIS) 250 x 354
#define DMPAPER_B5                  13		// B5 (JIS) 182 x 257 mm
#define DMPAPER_10X14               16		// 10x14 in
#define DMPAPER_11X17               17		// 11x17 in
#define DMPAPER_ENV_DL              27		// Envelope DL 110 x 220mm
#define DMPAPER_ENV_C5              28		// Envelope C5 162 x 229 mm
#define DMPAPER_ENV_C3              29		// Envelope C3  324 x 458 mm
#define DMPAPER_ENV_C4              30		// Envelope C4  229 x 324 mm
#define DMPAPER_ENV_C6              31		// Envelope C6  114 x 162 mm
#define DMPAPER_ENV_B4              33		// Envelope B4  250 x 353 mm
#define DMPAPER_ENV_B5              34		// Envelope B5  176 x 250 mm
#define DMPAPER_ENV_B6              35		// Envelope B6  176 x 125 mm
#define DMPAPER_ENV_MONARCH         37		// Envelope Monarch 3.875 x 7.5 in
#define DMPAPER_ISO_B4              42		// B4 (ISO) 250 x 353 mm
#define DMPAPER_JAPANESE_POSTCARD   43		// Japanese Postcard 100 x 148 mm
#define DMPAPER_DBL_JAPANESE_POSTCARD 69	// Japanese Double Postcard 200 x 148 mm
#define DMPAPER_A6                  70		// A6 105 x 148 mm

int WINAPI StartDoc(HDC, CONST DOCINFO*);
int WINAPI EndDoc(HDC);
int WINAPI StartPage(HDC);
int WINAPI EndPage(HDC);

BOOL WINAPI IsWindowEnabled(HWND hWnd);
BOOL WINAPI EnableWindow(HWND hWnd, BOOL bEnable);
BOOL WINAPI UpdateWindow(HWND hWnd);

HDC WINAPI  GetDC(HWND hWnd);
int WINAPI  ReleaseDC(HWND hWnd, HDC hDC);
HDC WINAPI  CreateCompatibleDC(HDC hDC);
HDC WINAPI  CreateDC(LPCTSTR, LPCTSTR, LPCTSTR, CONST DEVMODE*);
BOOL WINAPI DeleteDC(HDC);
int WINAPI  GetDeviceCaps(HDC, int);

HDC WINAPI  BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
BOOL WINAPI EndPaint(HWND hWnd, CONST PAINTSTRUCT *lpPaint);

BOOL WINAPI GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase);
BOOL WINAPI InvalidateRect(HWND hWnd, CONST RECT *lpRect, BOOL bErase);

BOOL WINAPI ValidateRect(HWND hWnd, CONST RECT *lpRect);

DWORD GetFullPathName(LPCTSTR path, int cbfn, LPTSTR fullname, LPTSTR* pfp);

#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L

#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONWARNING              MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

int WINAPI MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

#define COLOR_SCROLLBAR         0
#define COLOR_BACKGROUND        1
#define COLOR_ACTIVECAPTION     2
#define COLOR_INACTIVECAPTION   3
#define COLOR_MENU              4
#define COLOR_WINDOW            5
#define COLOR_WINDOWFRAME       6
#define COLOR_MENUTEXT          7
#define COLOR_WINDOWTEXT        8
#define COLOR_CAPTIONTEXT       9
#define COLOR_ACTIVEBORDER      10
#define COLOR_INACTIVEBORDER    11
#define COLOR_APPWORKSPACE      12
#define COLOR_HIGHLIGHT         13
#define COLOR_HIGHLIGHTTEXT     14
#define COLOR_BTNFACE           15
#define COLOR_BTNSHADOW         16
#define COLOR_GRAYTEXT          17
#define COLOR_BTNTEXT           18
#define COLOR_INACTIVECAPTIONTEXT 19
#define COLOR_BTNHIGHLIGHT      20

#define COLOR_3DDKSHADOW        21
#define COLOR_3DLIGHT           22
#define COLOR_INFOTEXT          23
#define COLOR_INFOBK            24

#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT

// Stock Objects
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH
#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16

// Pen Styles
#define PS_SOLID            0
#define PS_DASH             1       /* -------  */
#define PS_DOT              2       /* .......  */
#define PS_DASHDOT          3       /* _._._._  */
#define PS_DASHDOTDOT       4       /* _.._.._  */
#define PS_NULL             5
#define PS_INSIDEFRAME      6
#define PS_USERSTYLE        7
#define PS_ALTERNATE        8
#define PS_STYLE_MASK       0x0000000F

#define PS_ENDCAP_ROUND     0x00000000
#define PS_ENDCAP_SQUARE    0x00000100
#define PS_ENDCAP_FLAT      0x00000200
#define PS_ENDCAP_MASK      0x00000F00

#define PS_JOIN_ROUND       0x00000000
#define PS_JOIN_BEVEL       0x00001000
#define PS_JOIN_MITER       0x00002000
#define PS_JOIN_MASK        0x0000F000

#define PS_COSMETIC         0x00000000
#define PS_GEOMETRIC        0x00010000
#define PS_TYPE_MASK        0x000F0000

#define MF_INSERT           0x00000000L
#define MF_CHANGE           0x00000080L
#define MF_APPEND           0x00000100L
#define MF_DELETE           0x00000200L
#define MF_REMOVE           0x00001000L

#define MF_BYCOMMAND        0x00000000L
#define MF_BYPOSITION       0x00000400L

#define MF_SEPARATOR        0x00000800L

#define MF_ENABLED          0x00000000L
#define MF_GRAYED           0x00000001L
#define MF_DISABLED         0x00000002L

#define MF_UNCHECKED        0x00000000L
#define MF_CHECKED          0x00000008L
#define MF_USECHECKBITMAPS  0x00000200L

#define MF_STRING           0x00000000L
#define MF_BITMAP           0x00000004L
#define MF_OWNERDRAW        0x00000100L

#define MF_POPUP            0x00000010L
#define MF_MENUBARBREAK     0x00000020L
#define MF_MENUBREAK        0x00000040L

#define MF_UNHILITE         0x00000000L
#define MF_HILITE           0x00000080L

typedef void MENUTEMPLATE, *LPMENUTEMPLATE;

HMENU WINAPI LoadMenuIndirect(CONST LPMENUTEMPLATE lpMenuTemplate);
HMENU WINAPI LoadMenu(HINSTANCE, LPCTSTR);
HMENU WINAPI CreateMenu(void);
HMENU WINAPI CreatePopupMenu(void);
BOOL  WINAPI DestroyMenu(HMENU hMenu);

HMENU WINAPI GetMenu(HWND hWnd);
DWORD WINAPI CheckMenuItem(
							HMENU	hMenu,
							UINT	uIDCheckItem,
							UINT	uCheck);

BOOL WINAPI EnableMenuItem(
							HMENU	hMenu,
							UINT	uIDEnableItem,
							UINT	uEnable);

HMENU WINAPI GetSubMenu(HMENU hMenu, int nPos);

UINT WINAPI GetMenuItemID(
							HMENU hMenu,
							int nPos);

UINT WINAPI GetMenuState(
							HMENU hMenu,
							UINT  uId,
							UINT  uFlags
						);

int WINAPI GetMenuItemCount(HMENU hMenu);

BOOL WINAPI InsertMenu(
							HMENU	hMenu,
							UINT	uPosition,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							);

BOOL WINAPI AppendMenu(
							HMENU	hMenu,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							);


BOOL WINAPI ModifyMenu(
							HMENU	hMenu,
						    UINT  	uPosition,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							);

BOOL WINAPI RemoveMenu(
						    HMENU 	hMenu,
						    UINT 	uPosition,
						    UINT 	uFlags);

BOOL WINAPI SetMenu(
							HWND	hWnd,
							HMENU	hMenu);

BOOL WINAPI DeleteMenu(
							HMENU	hMenu,
							UINT	uPosition,
							UINT	uFlags);
/*
BOOL
WINAPI
SetMenuItemBitmaps(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    HBITMAP hBitmapUnchecked,
    HBITMAP hBitmapChecked);
*/

/* Windows calls this a BOOL but in its own documentation
 * says that the return can be the menu item id, and on
 * some systems (Mac OSX) BOOL is a char, so that can't work
 */
/*BOOL*/int WINAPI TrackPopupMenu(
							HMENU	hMenu,
							UINT	uFlags,
							int		x,
							int		y,
							int		nReserved,
							HWND	hWnd,
							CONST RECT *prcRect);

#define TPM_RETURNCMD		0x100

#define SBM_SETPOS          0x00E0
#define SBM_GETPOS          0x00E1
#define SBM_SETRANGE        0x00E2
#define SBM_SETRANGEREDRAW  0x00E6
#define SBM_GETRANGE        0x00E3
#define SBM_ENABLE_ARROWS   0x00E4
#define SBM_SETSCROLLINFO   0x00E9
#define SBM_GETSCROLLINFO   0x00EA

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}
SCROLLINFO, FAR *LPSCROLLINFO;
typedef SCROLLINFO CONST FAR *LPCSCROLLINFO;

int  WINAPI   SetScrollInfo(HWND, int, LPCSCROLLINFO, BOOL);
BOOL WINAPI   GetScrollInfo(HWND, int, LPSCROLLINFO);


int  WINAPI   SetScrollPos(
						    HWND hWnd,
						    int nBar,
						    int nPos,
						    BOOL bRedraw);

int WINAPI  GetScrollPos(HWND hWnd, int nBar);
BOOL WINAPI SetScrollRange(
						    HWND hWnd,
						    int nBar,
						    int nMinPos,
						    int nMaxPos,
						    BOOL bRedraw);

BOOL WINAPI GetScrollRange(
						    HWND hWnd,
						    int nBar,
						    LPINT lpMinPos,
						    LPINT lpMaxPos);

typedef DWORD COLORREF, *LPCOLORREF;

#define RGB(r,g,b) (((((DWORD)(b))&0xff)<<16)|((((DWORD)(g))&0xff)<<8)|(((DWORD)(r))&0xff))

typedef struct tagRGBTRIPLE
{
	BYTE    rgbtBlue;
	BYTE    rgbtGreen;
	BYTE    rgbtRed;
}
RGBTRIPLE, FAR* LPRGBTRIPLE;

typedef struct tagRGBQUAD
{
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
}
RGBQUAD, FAR* LPRGBQUAD;

#define SRCCOPY             (DWORD)0x00CC0020 /* dest = source                   */
#define SRCPAINT            (DWORD)0x00EE0086 /* dest = source OR dest           */
#define SRCAND              (DWORD)0x008800C6 /* dest = source AND dest          */
#define SRCINVERT           (DWORD)0x00660046 /* dest = source XOR dest          */
#define SRCERASE            (DWORD)0x00440328 /* dest = source AND (NOT dest )   */

#define TA_NOUPDATECP        0
#define TA_UPDATECP          1

#define TA_LEFT              0
#define TA_RIGHT             2
#define TA_CENTER            6

#define TA_TOP               0
#define TA_BOTTOM            8
#define TA_BASELINE          24

#define VTA_BASELINE		TA_BASELINE
#define VTA_LEFT			TA_BOTTOM
#define VTA_RIGHT			TA_TOP
#define VTA_CENTER			TA_CENTER
#define VTA_BOTTOM			TA_RIGHT
#define VTA_TOP				TA_LEFT

#define MM_TEXT             1
#define MM_LOMETRIC         2
#define MM_HIMETRIC         3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS            6
/*#define MM_ISOTROPIC        7
  #define MM_ANISOTROPIC      8*/

DWORD	WINAPI GetSysColor(int nIndex);
HGDIOBJ WINAPI GetStockObject(int);
HBRUSH	WINAPI GetSysColorBrush(int nIndex);
HBRUSH	WINAPI CreateSolidBrush(COLORREF color);
HPEN	WINAPI CreatePen(int style, int width, COLORREF color);

HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
BOOL 	WINAPI DeleteObject(HGDIOBJ);
int		WINAPI GetObject(HGDIOBJ, int, void*);


int		WINAPI SetMapMode(HDC, int);
int		WINAPI GetMapMode(HDC);

int WINAPI FillRect(
				    HDC hDC,
				    CONST RECT *lprc,
				    HBRUSH hbr
				    );

BOOL WINAPI TextOut(HDC, int, int, LPCTSTR, int);
BOOL WINAPI MoveToEx(HDC, int, int, LPPOINT);
BOOL WINAPI LineTo(HDC, int, int);
BOOL WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);

COLORREF WINAPI SetTextColor(HDC hdc, COLORREF color);
COLORREF WINAPI GetTextColor(HDC hdc);
COLORREF WINAPI SetBkColor(HDC hdc, COLORREF color);
COLORREF WINAPI GetBkColor(HDC hdc);

#define GWL_WNDPROC		-36
#define GWL_HWNDPARENT	-32
#define GWL_HINSTANCE	-28
#define GWL_STYLE		-24
#define GWL_EXSTYLE		-20
#define GWL_ID			-16

#define DWL_DLGPROC		-12
#define DWL_MSGRESULT	-8

#define GWL_USERDATA	-4
#define GWL_MIN			-36
#define GWL_MAX			4*4

#define DWL_USER		-sizeof(LONG)

#define GCL_MENUNAME        -8
#define GCL_HBRBACKGROUND   -10
#define GCL_HCURSOR         -12
#define GCL_HICON           -14
#define GCL_HMODULE         -16
#define GCL_CBWNDEXTRA      -18
#define GCL_CBCLSEXTRA      -20
#define GCL_WNDPROC         -24
#define GCL_STYLE           -26
#define GCW_ATOM            -32
#define GCL_HICONSM         -34

LONG WINAPI GetWindowLong(HWND hWnd, int nIndex);
LONG WINAPI SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
LONG WINAPI GetClassLong(HWND hWnd, int nIndex);
LONG WINAPI SetClassLong(HWND hWnd, int nIndex, LONG dwNewLong);

HWND WINAPI GetParent(HWND hWnd);
HWND WINAPI SetParent(HWND hWndChild, HWND hWndNewParent);

typedef struct tagBITMAPCOREHEADER
{
	DWORD   bcSize;
	WORD    bcWidth;
	WORD    bcHeight;
	WORD    bcPlanes;
	WORD    bcBitCount;
}
BITMAPCOREHEADER, FAR *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER
{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
}
BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAP
{
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    void*      bmBits;
}
BITMAP, *PBITMAP, FAR *LPBITMAP;

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#define DIB_RGB_COLORS      0
#define DIB_PAL_COLORS      1

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
}
BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;

typedef struct tagBITMAPCOREINFO
{
    BITMAPCOREHEADER    bmciHeader;
    RGBTRIPLE           bmciColors[1];
}
BITMAPCOREINFO, FAR *LPBITMAPCOREINFO, *PBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
}
BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

#define CBM_INIT 1

HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCTSTR lpBitmapName);
HBITMAP WINAPI CreateBitmap(int w, int h, UINT pl, UINT bpp, CONST void *lpvBits);
LONG	WINAPI GetBitmapBits(HBITMAP hbm, LONG nBytes, void *lpvBits);
HBITMAP WINAPI CreateCompatibleBitmap(HDC hdc, int w, int h);
HBITMAP WINAPI CreateDIBitmap(HDC hdc, CONST BITMAPINFOHEADER *lpbmih, DWORD fdwInit, CONST void *lpbInit, CONST BITMAPINFO *lpbmi, UINT fuUsage);

HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCTSTR pCursorName);
HCURSOR WINAPI CreateCursor(HINSTANCE hInstance, int xHot, int yHot, int w, int h, const void* pvAnd, const void* pvXOR);

#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)
#define IDC_WAIT            MAKEINTRESOURCE(32514)
#define IDC_CROSS           MAKEINTRESOURCE(32515)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)

typedef struct _ICONINFO {
    BOOL    fIcon;
    DWORD   xHotspot;
    DWORD   yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
} ICONINFO;
typedef ICONINFO *PICONINFO;

HICON	WINAPI 	LoadIcon(HINSTANCE hInstance, LPCTSTR lpIconName);
HANDLE	WINAPI 	LoadImage(HINSTANCE, LPCTSTR, UINT, int, int, UINT);
BOOL	WINAPI	DrawIcon(HDC hdc, int x, int y, HICON hIcon);
BOOL	WINAPI 	GetIconInfo(HICON hIcon, PICONINFO pii);
BOOL	WINAPI	DestroyIcon(HICON hIcon);

#define IMAGE_BITMAP		0
#define IMAGE_ICON			1
#define IMAGE_CURSOR		2

#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_HAND            MAKEINTRESOURCE(32513)
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION     MAKEINTRESOURCE(32515)
#define IDI_ASTERISK        MAKEINTRESOURCE(32516)



#define CLR_NONE                0xFFFFFFFFL
#define CLR_DEFAULT             0xFF000000L

typedef void* HIMAGELIST;

typedef struct _IMAGELISTDRAWPARAMS
{
    DWORD       cbSize;
    HIMAGELIST  himl;
    int         i;
    HDC         hdcDst;
    int         x;
    int         y;
    int         cx;
    int         cy;
    int         xBitmap;        // x offest from the upperleft of bitmap
    int         yBitmap;        // y offset from the upperleft of bitmap
    COLORREF    rgbBk;
    COLORREF    rgbFg;
    UINT        fStyle;
    DWORD       dwRop;
}
IMAGELISTDRAWPARAMS, FAR * LPIMAGELISTDRAWPARAMS;

#define ILC_MASK                0x0001
#define ILC_COLOR               0x0000
#define ILC_COLORDDB            0x00FE
#define ILC_COLOR4              0x0004
#define ILC_COLOR8              0x0008
#define ILC_COLOR16             0x0010
#define ILC_COLOR24             0x0018
#define ILC_COLOR32             0x0020
#define ILC_PALETTE             0x0800

HIMAGELIST  WINAPI ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
BOOL        WINAPI ImageList_Destroy(HIMAGELIST himl);
int         WINAPI ImageList_GetImageCount(HIMAGELIST himl);
BOOL        WINAPI ImageList_SetImageCount(HIMAGELIST himl, UINT uNewCount);
int         WINAPI ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
int         WINAPI ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
COLORREF    WINAPI ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
COLORREF    WINAPI ImageList_GetBkColor(HIMAGELIST himl);
BOOL        WINAPI ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);

#define     ImageList_AddIcon(himl, hicon) ImageList_ReplaceIcon(himl, -1, hicon)

#define ILD_NORMAL              0x0000
#define ILD_TRANSPARENT         0x0001
#define ILD_MASK                0x0010
#define ILD_IMAGE               0x0020
#define ILD_ROP                 0x0040
#define ILD_BLEND25             0x0002
#define ILD_BLEND50             0x0004
#define ILD_OVERLAYMASK         0x0F00
#define INDEXTOOVERLAYMASK(i)   ((i) << 8)

#define ILD_SELECTED            ILD_BLEND50
#define ILD_FOCUS               ILD_BLEND25
#define ILD_BLEND               ILD_BLEND50
#define CLR_HILIGHT             CLR_DEFAULT

BOOL WINAPI ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle);

#if 0 // --- not implemented

BOOL        WINAPI ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);
int         WINAPI ImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
BOOL        WINAPI ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
BOOL        WINAPI ImageList_Remove(HIMAGELIST himl, int i);
HICON       WINAPI ImageList_GetIcon(HIMAGELIST himl, int i, UINT flags);
HIMAGELIST  WINAPI ImageList_LoadImage(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
BOOL        WINAPI ImageList_BeginDrag(HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot);
void        WINAPI ImageList_EndDrag();
BOOL        WINAPI ImageList_DragEnter(HWND hwndLock, int x, int y);
BOOL        WINAPI ImageList_DragLeave(HWND hwndLock);
BOOL        WINAPI ImageList_DragMove(int x, int y);
BOOL        WINAPI ImageList_SetDragCursorImage(HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot);
BOOL        WINAPI ImageList_DragShowNolock(BOOL fShow);
HIMAGELIST  WINAPI ImageList_GetDragImage(POINT FAR* ppt,POINT FAR* pptHotspot);
#endif

#define     ImageList_RemoveAll(himl) ImageList_Remove(himl, -1)
#define     ImageList_ExtractIcon(hi, himl, i) ImageList_GetIcon(himl, i, 0)
#define     ImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask) ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)


typedef struct _IMAGEINFO
{
    HBITMAP hbmImage;
    HBITMAP hbmMask;
    int     Unused1;
    int     Unused2;
    RECT    rcImage;
} IMAGEINFO, FAR *LPIMAGEINFO;

BOOL        WINAPI ImageList_GetIconSize(HIMAGELIST himl, int FAR *cx, int FAR *cy);
BOOL        WINAPI ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy);
BOOL        WINAPI ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo);
#if 0 // -- not implemented
HIMAGELIST  WINAPI ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);
#endif

int WINAPI LoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);

#define GMEM_MOVEABLE	0x01
#define GMEM_DDESHARE	0x02
#define GMEM_ZEROINIT	0x40

HANDLE WINAPI GlobalAlloc(int flags, int size);
void*  WINAPI GlobalLock(HANDLE hMem);
void   WINAPI GlobalUnlock(HANDLE hMem);
void   WINAPI GlobalFree(HANDLE hMem);

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

/* edit box styles
*/
#define ES_LEFT             0x0000L
#define ES_CENTER           0x0001L
#define ES_RIGHT            0x0002L
#define ES_MULTILINE        0x0004L
#define ES_PASSWORD         0x0020L
#define ES_AUTOVSCROLL      0x0040L
#define ES_AUTOHSCROLL      0x0080L
#define ES_READONLY         0x0800L
#define ES_NUMBER           0x2000L

/* edit messages
*/
#define EM_GETSEL               0x00B0
#define EM_SETSEL               0x00B1
#define EM_GETRECT              0x00B2
#define EM_SETRECT              0x00B3
#define EM_SETRECTNP            0x00B4
#define EM_SCROLL               0x00B5
#define EM_LINESCROLL           0x00B6
#define EM_SCROLLCARET          0x00B7
#define EM_GETMODIFY            0x00B8
#define EM_SETMODIFY            0x00B9
#define EM_GETLINECOUNT         0x00BA
#define EM_LINEINDEX            0x00BB
#define EM_SETHANDLE            0x00BC
#define EM_GETHANDLE            0x00BD
#define EM_GETTHUMB             0x00BE
#define EM_LINELENGTH           0x00C1
#define EM_REPLACESEL           0x00C2
#define EM_GETLINE              0x00C4
#define EM_LIMITTEXT            0x00C5
#define EM_CANUNDO              0x00C6
#define EM_UNDO                 0x00C7
#define EM_FMTLINES             0x00C8
#define EM_LINEFROMCHAR         0x00C9
#define EM_SETTABSTOPS          0x00CB
#define EM_SETPASSWORDCHAR      0x00CC
#define EM_EMPTYUNDOBUFFER      0x00CD
#define EM_GETFIRSTVISIBLELINE  0x00CE
#define EM_SETREADONLY          0x00CF

/* button styles
 */
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_OWNERDRAW        0x0000000BL
#define BS_LEFTTEXT         0x00000020L
#define BS_TEXT             0x00000000L
#define BS_ICON             0x00000040L
#define BS_BITMAP           0x00000080L
#define BS_LEFT             0x00000100L
#define BS_RIGHT            0x00000200L
#define BS_CENTER           0x00000300L
#define BS_TOP              0x00000400L
#define BS_BOTTOM           0x00000800L
#define BS_VCENTER          0x00000C00L
#define BS_PUSHLIKE         0x00001000L
#define BS_MULTILINE        0x00002000L
#define BS_NOTIFY           0x00004000L
#define BS_FLAT             0x00008000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT

/* button messages
*/
#define BM_GETCHECK			0x00F0
#define BM_SETCHECK			0x00F1
#define BM_GETSTATE			0x00F2
#define BM_SETSTATE			0x00F3
#define BM_SETSTYLE			0x00F4
#define BM_CLICK			0x00F5
#define BM_GETIMAGE			0x00F6
#define BM_SETIMAGE			0x00F7

#define BST_UNCHECKED		0x0000
#define BST_CHECKED			0x0001
#define BST_INDETERMINATE	0x0002
#define BST_PUSHED			0x0004
#define BST_FOCUS			0x0008


void  WINAPI SetLastError(DWORD dwErrCode);
#ifndef GetLastError
DWORD WINAPI GetLastError();
#endif

// error codes
//
#define ERROR_SUCCESS					0L
#define NO_ERROR						0L
#define ERROR_INVALID_FUNCTION			1L
#define ERROR_FILE_NOT_FOUND			2L
#define ERROR_PATH_NOT_FOUND			3L
#define ERROR_TOO_MANY_OPEN_FILES		4L
#define ERROR_ACCESS_DENIED 			5L
#define ERROR_INVALID_HANDLE			6L
#define ERROR_ARENA_TRASHED 			7L
#define ERROR_NOT_ENOUGH_MEMORY 		8L
#define ERROR_INVALID_BLOCK 			9L
#define ERROR_BAD_ENVIRONMENT			10L
#define ERROR_BAD_FORMAT				11L
#define ERROR_INVALID_ACCESS			12L
#define ERROR_INVALID_DATA				13L
#define ERROR_OUTOFMEMORY				14L
#define ERROR_NO_MORE_FILES 			18L
#define ERROR_WRITE_PROTECT 			19L
#define ERROR_BAD_LENGTH				24L
#define ERROR_WRITE_FAULT				29L
#define ERROR_READ_FAULT				30L
#define ERROR_GEN_FAILURE				31L
#define ERROR_HANDLE_EOF				38L
#define ERROR_FILE_EXISTS				80L
#define ERROR_CANNOT_MAKE				82L
#define ERROR_OUT_OF_STRUCTURES 		84L
#define ERROR_INVALID_PASSWORD			86L
#define ERROR_INVALID_PARAMETER 		87L
#define ERROR_OPEN_FAILED				110L
#define ERROR_BUFFER_OVERFLOW			111L
#define ERROR_DISK_FULL 				112L
#define ERROR_CALL_NOT_IMPLEMENTED		120L
#define ERROR_INSUFFICIENT_BUFFER		122L
#define ERROR_INVALID_NAME				123L
#define ERROR_INVALID_LEVEL 			124L
#define ERROR_MOD_NOT_FOUND 			126L
#define ERROR_PROC_NOT_FOUND			127L
#define ERROR_BAD_PATHNAME				161L
#define ERROR_BUSY						170L
#define ERROR_ALREADY_EXISTS			183L
#define ERROR_INVALID_FLAG_NUMBER		186L
#define ERROR_STACK_OVERFLOW			1001L
#define ERROR_INVALID_MESSAGE			1002L
#define ERROR_CAN_NOT_COMPLETE			1003L
#define ERROR_INVALID_FLAGS 			1004L
#define ERROR_FILE_INVALID				1006L
#define ERROR_CLASS_ALREADY_EXISTS		1410L
#define ERROR_RESOURCE_DATA_NOT_FOUND	1812L
#define ERROR_RESOURCE_TYPE_NOT_FOUND	1813L
#define ERROR_RESOURCE_NAME_NOT_FOUND	1814L
#define ERROR_RESOURCE_LANG_NOT_FOUND	1815L

typedef struct tagLOGBRUSH
{
    UINT        lbStyle;
    COLORREF    lbColor;
    LONG        lbHatch;
}
LOGBRUSH, *PLOGBRUSH, FAR *LPLOGBRUSH;

typedef LOGBRUSH            PATTERN;
typedef PATTERN             *PPATTERN;
typedef PATTERN FAR         *LPPATTERN;

/* Logical Pen */
typedef struct tagLOGPEN
{
    UINT        lopnStyle;
    POINT       lopnWidth;
    COLORREF    lopnColor;
}
LOGPEN, *PLOGPEN, FAR *LPLOGPEN;

// font stuff

#define BSA_MAX_FONTFACE_LEN 64

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagLOGFONT
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    TCHAR     lfFaceName[BSA_MAX_FONTFACE_LEN];
}
LOGFONT, *PLOGFONT, FAR *LPLOGFONT;

#define OUT_DEFAULT_PRECIS          0
#define OUT_STRING_PRECIS           1
#define OUT_CHARACTER_PRECIS        2
#define OUT_STROKE_PRECIS           3
#define OUT_TT_PRECIS               4
#define OUT_DEVICE_PRECIS           5
#define OUT_RASTER_PRECIS           6
#define OUT_TT_ONLY_PRECIS          7
#define OUT_OUTLINE_PRECIS          8
#define OUT_SCREEN_OUTLINE_PRECIS   9

#define CLIP_DEFAULT_PRECIS     0
#define CLIP_CHARACTER_PRECIS   1
#define CLIP_STROKE_PRECIS      2
#define CLIP_MASK               0xf
#define CLIP_LH_ANGLES          (1<<4)
#define CLIP_TT_ALWAYS          (2<<4)
#define CLIP_EMBEDDED           (8<<4)

#define DEFAULT_QUALITY         0
#define DRAFT_QUALITY           1
#define PROOF_QUALITY           2
#define NONANTIALIASED_QUALITY  3
#define ANTIALIASED_QUALITY     4

#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2
#define MONO_FONT               8

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#define FS_LATIN1               0x00000001L
#define FS_LATIN2               0x00000002L
#define FS_CYRILLIC             0x00000004L
#define FS_GREEK                0x00000008L
#define FS_TURKISH              0x00000010L
#define FS_HEBREW               0x00000020L
#define FS_ARABIC               0x00000040L
#define FS_BALTIC               0x00000080L
#define FS_VIETNAMESE           0x00000100L
#define FS_THAI                 0x00010000L
#define FS_JISJAPAN             0x00020000L
#define FS_CHINESESIMP          0x00040000L
#define FS_WANSUNG              0x00080000L
#define FS_CHINESETRAD          0x00100000L
#define FS_JOHAB                0x00200000L
#define FS_SYMBOL               0x80000000L

// Font Families
#define FF_DONTCARE         (0<<4)  /* Don't care or don't know. */
#define FF_ROMAN            (1<<4)  /* Variable stroke width, serifed. */
                                    /* Times Roman, Century Schoolbook, etc. */
#define FF_SWISS            (2<<4)  /* Variable stroke width, sans-serifed. */
                                    /* Helvetica, Swiss, etc. */
#define FF_MODERN           (3<<4)  /* Constant stroke width, serifed or sans-serifed. */
                                    /* Pica, Elite, Courier, etc. */
#define FF_SCRIPT           (4<<4)  /* Cursive, etc. */
#define FF_DECORATIVE       (5<<4)  /* Old English, etc. */

// Font Weights
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
}
TEXTMETRICA, *PTEXTMETRICA, FAR *LPTEXTMETRICA;

typedef struct tagTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
}
TEXTMETRICW, *PTEXTMETRICW, FAR *LPTEXTMETRICW;

#ifdef UNICODE
	typedef TEXTMETRICW TEXTMETRIC;
	typedef PTEXTMETRICW PTEXTMETRIC;
	typedef LPTEXTMETRICW LPTEXTMETRIC;
#else
	typedef TEXTMETRICA TEXTMETRIC;
	typedef PTEXTMETRICA PTEXTMETRIC;
	typedef LPTEXTMETRICA LPTEXTMETRIC;
#endif // UNICODE


BOOL WINAPI GetTextMetrics(HDC, LPTEXTMETRIC);
BOOL WINAPI GetTextExtentPoint32(
								HDC,
								LPCTSTR,
								int,
								LPSIZE
								);
#define GetTextExtentPoint(a,b,c,d) GetTextExtentPoint32(a,b,c,d)

HFONT WINAPI CreateFont(int, int, int, int, int, DWORD,
                             DWORD, DWORD, DWORD, DWORD, DWORD,
                             DWORD, DWORD, LPCTSTR);

typedef int CALLBACK (*FONTENUMPROC)(CONST LOGFONT* lplf, CONST TEXTMETRIC* lptm, DWORD dwType, LPARAM lpData);

#define RASTER_FONTTYPE     0x001
#define DEVICE_FONTTYPE     0x002
#define TRUETYPE_FONTTYPE   0x004

int EnumFonts(HDC hdc, LPCTSTR lpFaceName, FONTENUMPROC lpFontFunc, LPARAM lParam);


// dialog stuff
//
#define DS_ABSALIGN         0x01L
#define DS_SYSMODAL         0x02L
#define DS_LOCALEDIT        0x20L
#define DS_SETFONT          0x40L
#define DS_MODALFRAME       0x80L
#define DS_NOIDLEMSG        0x100L
#define DS_SETFOREGROUND    0x200L
#define DS_3DLOOK           0x0004L
#define DS_FIXEDSYS         0x0008L
#define DS_NOFAILCREATE     0x0010L
#define DS_CONTROL          0x0400L
#define DS_CENTER           0x0800L
#define DS_CENTERMOUSE      0x1000L
#define DS_CONTEXTHELP      0x2000L


#define DLGC_WANTARROWS     0x0001
#define DLGC_WANTTAB        0x0002
#define DLGC_WANTALLKEYS    0x0004
#define DLGC_WANTMESSAGE    0x0004
#define DLGC_HASSETSEL      0x0008
#define DLGC_DEFPUSHBUTTON  0x0010
#define DLGC_UNDEFPUSHBUTTON 0x0020
#define DLGC_RADIOBUTTON    0x0040
#define DLGC_WANTCHARS      0x0080
#define DLGC_STATIC         0x0100
#define DLGC_BUTTON         0x2000


#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04

#define VK_BACK           0x08
#define VK_TAB            0x09

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_PROCESSKEY     0xE5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

// Common Controls --------------------------------------------------------------------
// ------------------------------------------------------------------------------------

#ifndef SNDMSG
#define SNDMSG SendMessage
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;
#endif // _HRESULT_DEFINED

typedef struct tagINITCOMMONCONTROLSEX
{
    DWORD dwSize;             // size of this structure
    DWORD dwICC;              // flags indicating which classes to be initialized
}
INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;


void WINAPI InitCommonControls(void);
BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX);

#define ICC_LISTVIEW_CLASSES 0x00000001 // listview, header
#define ICC_TREEVIEW_CLASSES 0x00000002 // treeview, tooltips
#define ICC_BAR_CLASSES      0x00000004 // toolbar, statusbar, trackbar, tooltips
#define ICC_TAB_CLASSES      0x00000008 // tab, tooltips
#define ICC_UPDOWN_CLASS     0x00000010 // updown
#define ICC_PROGRESS_CLASS   0x00000020 // progress
#define ICC_HOTKEY_CLASS     0x00000040 // hotkey
#define ICC_ANIMATE_CLASS    0x00000080 // animate
#define ICC_WIN95_CLASSES    0x000000FF
#define ICC_DATE_CLASSES     0x00000100 // month picker, date picker, time picker, updown
#define ICC_USEREX_CLASSES   0x00000200 // comboex
#define ICC_COOL_CLASSES     0x00000400 // rebar (coolbar) control
#define ICC_INTERNET_CLASSES 0x00000800
#define ICC_PAGESCROLLER_CLASS 0x00001000   // page scroller
#define ICC_NATIVEFNTCTL_CLASS 0x00002000   // native font control

#define ODT_HEADER              100
#define ODT_TAB                 101
#define ODT_LISTVIEW            102


//Ranges for control messages

#define LVM_FIRST               0x1000
#define TV_FIRST                0x1100
#define TCM_FIRST               0x1300      // Tab control messages

#define INFOTIPSIZE 1024

// WM_NOTIFY Macros

#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(wParam), (NMHDR FAR*)(lParam))
#define FORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
    (LRESULT)(fn)((hwnd), WM_NOTIFY, (WPARAM)(int)(idFrom), (LPARAM)(NMHDR FAR*)(pnmhdr))


// Generic WM_NOTIFY notification codes

#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)    // uses NMCLICK struct
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)    // uses NMCLICK struct
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_CUSTOMDRAW           (NM_FIRST-12)
#define NM_HOVER                (NM_FIRST-13)
#define NM_NCHITTEST            (NM_FIRST-14)   // uses NMMOUSE struct
#define NM_KEYDOWN              (NM_FIRST-15)   // uses NMKEY struct
#define NM_RELEASEDCAPTURE      (NM_FIRST-16)
#define NM_SETCURSOR            (NM_FIRST-17)   // uses NMMOUSE struct
#define NM_CHAR                 (NM_FIRST-18)   // uses NMCHAR struct


#ifndef CCSIZEOF_STRUCT
#define CCSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif


typedef struct tagNMMOUSE
{
    NMHDR   hdr;
    DWORD   dwItemSpec;
    DWORD   dwItemData;
    POINT   pt;
    DWORD   dwHitInfo;
}
NMMOUSE, FAR* LPNMMOUSE;

typedef NMMOUSE		NMCLICK;
typedef LPNMMOUSE	LPNMCLICK;

// Generic structure for a key

typedef struct tagNMKEY
{
    NMHDR hdr;
    UINT  nVKey;
    UINT  uFlags;
}
NMKEY, FAR *LPNMKEY;

// Generic structure for a character

typedef struct tagNMCHAR
{
    NMHDR   hdr;
    UINT    ch;
    DWORD   dwItemPrev;     // Item previously selected
    DWORD   dwItemNext;     // Item to be selected
}
NMCHAR, FAR* LPNMCHAR;


// WM_NOTIFY codes

#define NM_FIRST                (0U-  0U)       // generic to all controls
#define NM_LAST                 (0U- 99U)

#define LVN_FIRST               (0U-100U)       // listview
#define LVN_LAST                (0U-199U)

#define TVN_FIRST               (0U-400U)       // treeview
#define TVN_LAST                (0U-499U)

#define TCN_FIRST               (0U-550U)       // tab control
#define TCN_LAST                (0U-580U)

// Shell reserved               (0U-580U) -  (0U-589U)


#define MSGF_COMMCTRL_BEGINDRAG     0x4200
#define MSGF_COMMCTRL_SIZEHEADER    0x4201
#define MSGF_COMMCTRL_DRAGSELECT    0x4202
#define MSGF_COMMCTRL_TOOLBARCUST   0x4203

// LISTVIEW CONTROL --------------------------------------

#define WC_LISTVIEW             _T("SysListView32")


#define LVS_ICON                0x0000
#define LVS_REPORT              0x0001
#define LVS_SMALLICON           0x0002
#define LVS_LIST                0x0003
#define LVS_TYPEMASK            0x0003
#define LVS_SINGLESEL           0x0004
#define LVS_SHOWSELALWAYS       0x0008
#define LVS_SORTASCENDING       0x0010
#define LVS_SORTDESCENDING      0x0020
#define LVS_SHAREIMAGELISTS     0x0040
#define LVS_NOLABELWRAP         0x0080
#define LVS_AUTOARRANGE         0x0100
#define LVS_EDITLABELS          0x0200
#define LVS_OWNERDATA           0x1000
#define LVS_NOSCROLL            0x2000

#define LVS_TYPESTYLEMASK       0xfc00

#define LVS_ALIGNTOP            0x0000
#define LVS_ALIGNLEFT           0x0800
#define LVS_ALIGNMASK           0x0c00

#define LVS_OWNERDRAWFIXED      0x0400
#define LVS_NOCOLUMNHEADER      0x4000
#define LVS_NOSORTHEADER        0x8000

#define LVM_GETBKCOLOR          (LVM_FIRST + 0)
#define ListView_GetBkColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETBKCOLOR, 0, 0L)

#define LVM_SETBKCOLOR          (LVM_FIRST + 1)
#define ListView_SetBkColor(hwnd, clrBk) \
    (BOOL)SNDMSG((hwnd), LVM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))

#define LVM_GETIMAGELIST        (LVM_FIRST + 2)
#define ListView_GetImageList(hwnd, iImageList) \
    (HIMAGELIST)SNDMSG((hwnd), LVM_GETIMAGELIST, (WPARAM)(INT)(iImageList), 0L)

#define LVSIL_NORMAL            0
#define LVSIL_SMALL             1
#define LVSIL_STATE             2

#define LVM_SETIMAGELIST        (LVM_FIRST + 3)
#define ListView_SetImageList(hwnd, himl, iImageList) \
    (HIMAGELIST)(UINT)SNDMSG((hwnd), LVM_SETIMAGELIST, (WPARAM)(iImageList), (LPARAM)(HIMAGELIST)(himl))

#define LVM_GETITEMCOUNT        (LVM_FIRST + 4)
#define ListView_GetItemCount(hwnd) \
    (int)SNDMSG((hwnd), LVM_GETITEMCOUNT, 0, 0L)


#define LVIF_TEXT               0x0001
#define LVIF_IMAGE              0x0002
#define LVIF_PARAM              0x0004
#define LVIF_STATE              0x0008
#define LVIF_INDENT             0x0010
#define LVIF_NORECOMPUTE        0x0800

#define LVIS_FOCUSED            0x0001
#define LVIS_SELECTED           0x0002
#define LVIS_CUT                0x0004
#define LVIS_DROPHILITED        0x0008
#define LVIS_ACTIVATING         0x0020

#define LVIS_OVERLAYMASK        0x0F00
#define LVIS_STATEIMAGEMASK     0xF000

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)

#define I_INDENTCALLBACK        (-1)

#define LV_ITEM LVITEM

#define LVITEMA_V1_SIZE CCSIZEOF_STRUCT(LVITEMA, lParam)
#define LVITEMW_V1_SIZE CCSIZEOF_STRUCT(LVITEMW, lParam)

typedef struct tagLVITEMA
{
    UINT		mask;
    int			iItem;
    int			iSubItem;
    UINT		state;
    UINT		stateMask;
    LPTSTR		pszText;
    int			cchTextMax;
    int			iImage;
    LPARAM		lParam;
    int			iIndent;
}
LVITEM, FAR* LPLVITEM;


#define LVM_GETITEM             (LVM_FIRST + 75)

#define ListView_GetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_GETITEM, 0, (LPARAM)(LV_ITEM FAR*)(pitem))


#define LVM_SETITEM             (LVM_FIRST + 76)

#define ListView_SetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_SETITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))


#define LVM_INSERTITEM          (LVM_FIRST + 77)

#define ListView_InsertItem(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))


#define LVM_DELETEITEM          (LVM_FIRST + 8)
#define ListView_DeleteItem(hwnd, i) \
    (BOOL)SNDMSG((hwnd), LVM_DELETEITEM, (WPARAM)(int)(i), 0L)


#define LVM_DELETEALLITEMS      (LVM_FIRST + 9)
#define ListView_DeleteAllItems(hwnd) \
    (BOOL)SNDMSG((hwnd), LVM_DELETEALLITEMS, 0, 0L)


#define LVNI_ALL                0x0000
#define LVNI_FOCUSED            0x0001
#define LVNI_SELECTED           0x0002
#define LVNI_CUT                0x0004
#define LVNI_DROPHILITED        0x0008

#define LVNI_ABOVE              0x0100
#define LVNI_BELOW              0x0200
#define LVNI_TOLEFT             0x0400
#define LVNI_TORIGHT            0x0800


#define LVM_GETNEXTITEM         (LVM_FIRST + 12)
#define ListView_GetNextItem(hwnd, i, flags) \
    (int)SNDMSG((hwnd), LVM_GETNEXTITEM, (WPARAM)(int)(i), MAKELPARAM((flags), 0))


#define LVFI_PARAM              0x0001
#define LVFI_STRING             0x0002
#define LVFI_PARTIAL            0x0008
#define LVFI_WRAP               0x0020
#define LVFI_NEARESTXY          0x0040

#define LVHT_NOWHERE            0x0001
#define LVHT_ONITEMICON         0x0002
#define LVHT_ONITEMLABEL        0x0004
#define LVHT_ONITEMSTATEICON    0x0008
#define LVHT_ONITEM             (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)

#define LVHT_ABOVE              0x0008
#define LVHT_BELOW              0x0010
#define LVHT_TORIGHT            0x0020
#define LVHT_TOLEFT             0x0040


#define LVM_ENSUREVISIBLE       (LVM_FIRST + 19)
#define ListView_EnsureVisible(hwndLV, i, fPartialOK) \
    (BOOL)SNDMSG((hwndLV), LVM_ENSUREVISIBLE, (WPARAM)(int)(i), MAKELPARAM((fPartialOK), 0))


#define LVM_SCROLL              (LVM_FIRST + 20)
#define ListView_Scroll(hwndLV, dx, dy) \
    (BOOL)SNDMSG((hwndLV), LVM_SCROLL, (WPARAM)(int)dx, (LPARAM)(int)dy)


#define LVM_REDRAWITEMS         (LVM_FIRST + 21)
#define ListView_RedrawItems(hwndLV, iFirst, iLast) \
    (BOOL)SNDMSG((hwndLV), LVM_REDRAWITEMS, (WPARAM)(int)iFirst, (LPARAM)(int)iLast)


#define LVA_DEFAULT             0x0000
#define LVA_ALIGNLEFT           0x0001
#define LVA_ALIGNTOP            0x0002
#define LVA_SNAPTOGRID          0x0005

#define LVM_ARRANGE             (LVM_FIRST + 22)
#define ListView_Arrange(hwndLV, code) \
    (BOOL)SNDMSG((hwndLV), LVM_ARRANGE, (WPARAM)(UINT)(code), 0L)

#define LVM_GETVIEWRECT         (LVM_FIRST + 34)
#define ListView_GetViewRect(hwnd, prc) \
    (BOOL)SNDMSG((hwnd), LVM_GETVIEWRECT, 0, (LPARAM)(RECT FAR*)(prc))


#define LVM_GETTEXTCOLOR        (LVM_FIRST + 35)
#define ListView_GetTextColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETTEXTCOLOR, 0, 0L)


#define LVM_SETTEXTCOLOR        (LVM_FIRST + 36)
#define ListView_SetTextColor(hwnd, clrText) \
    (BOOL)SNDMSG((hwnd), LVM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText))


#define LVM_GETTEXTBKCOLOR      (LVM_FIRST + 37)
#define ListView_GetTextBkColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETTEXTBKCOLOR, 0, 0L)


#define LVM_SETTEXTBKCOLOR      (LVM_FIRST + 38)
#define ListView_SetTextBkColor(hwnd, clrTextBk) \
    (BOOL)SNDMSG((hwnd), LVM_SETTEXTBKCOLOR, 0, (LPARAM)(COLORREF)(clrTextBk))


#define LVM_GETTOPINDEX         (LVM_FIRST + 39)
#define ListView_GetTopIndex(hwndLV) \
    (int)SNDMSG((hwndLV), LVM_GETTOPINDEX, 0, 0)


#define LVM_GETCOUNTPERPAGE     (LVM_FIRST + 40)
#define ListView_GetCountPerPage(hwndLV) \
    (int)SNDMSG((hwndLV), LVM_GETCOUNTPERPAGE, 0, 0)


#define LVM_GETORIGIN           (LVM_FIRST + 41)
#define ListView_GetOrigin(hwndLV, ppt) \
    (BOOL)SNDMSG((hwndLV), LVM_GETORIGIN, (WPARAM)0, (LPARAM)(POINT FAR*)(ppt))


#define LVM_UPDATE              (LVM_FIRST + 42)
#define ListView_Update(hwndLV, i) \
    (BOOL)SNDMSG((hwndLV), LVM_UPDATE, (WPARAM)i, 0L)


#define LVM_SETITEMSTATE        (LVM_FIRST + 43)
#define ListView_SetItemState(hwndLV, i, data, mask) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.stateMask = mask;\
  _ms_lvi.state = data;\
  SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}


#define LVM_GETITEMSTATE        (LVM_FIRST + 44)
#define ListView_GetItemState(hwndLV, i, mask) \
   (UINT)SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)mask)

#define ListView_GetCheckState(hwndLV, i) \
   ((((UINT)(SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)i, LVIS_STATEIMAGEMASK))) >> 12) -1)

#define LVM_GETITEMTEXT         (LVM_FIRST + 115)

#define ListView_GetItemText(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.cchTextMax = cchTextMax_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}


#define LVM_SETITEMTEXT         (LVM_FIRST + 116)

#define ListView_SetItemText(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_SETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}

#define LVSICF_NOINVALIDATEALL  0x00000001
#define LVSICF_NOSCROLL         0x00000002

#define LVM_SETITEMCOUNT        (LVM_FIRST + 47)
#define ListView_SetItemCount(hwndLV, cItems) \
  SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)cItems, 0)

#define ListView_SetItemCountEx(hwndLV, cItems, dwFlags) \
  SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)cItems, (LPARAM)dwFlags)

typedef int (CALLBACK *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);

#define LVM_SORTITEMS           (LVM_FIRST + 48)
#define ListView_SortItems(hwndLV, _pfnCompare, _lPrm) \
  (BOOL)SNDMSG((hwndLV), LVM_SORTITEMS, (WPARAM)(LPARAM)_lPrm, \
  (LPARAM)(PFNLVCOMPARE)_pfnCompare)


#define LVM_SETITEMPOSITION32   (LVM_FIRST + 49)
#define ListView_SetItemPosition32(hwndLV, i, x, y) \
{ POINT ptNewPos = {x,y}; \
    SNDMSG((hwndLV), LVM_SETITEMPOSITION32, (WPARAM)(int)(i), (LPARAM)&ptNewPos); \
}


#define LVM_GETSELECTEDCOUNT    (LVM_FIRST + 50)
#define ListView_GetSelectedCount(hwndLV) \
    (UINT)SNDMSG((hwndLV), LVM_GETSELECTEDCOUNT, 0, 0L)


#define LVM_GETITEMSPACING      (LVM_FIRST + 51)
#define ListView_GetItemSpacing(hwndLV, fSmall) \
        (DWORD)SNDMSG((hwndLV), LVM_GETITEMSPACING, fSmall, 0L)


#define LVM_GETISEARCHSTRING    (LVM_FIRST + 117)

#define ListView_GetISearchString(hwndLV, lpsz) \
        (BOOL)SNDMSG((hwndLV), LVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)lpsz)

#define LVM_SETICONSPACING      (LVM_FIRST + 53)
// -1 for cx and cy means we'll use the default (system settings)
// 0 for cx or cy means use the current setting (allows you to change just one param)
#define ListView_SetIconSpacing(hwndLV, cx, cy) \
        (DWORD)SNDMSG((hwndLV), LVM_SETICONSPACING, 0, MAKELONG(cx,cy))


#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define ListView_SetExtendedListViewStyle(hwndLV, dw)\
        (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)
#define ListView_SetExtendedListViewStyleEx(hwndLV, dwMask, dw)\
        (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dw)

#define LVM_GETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 55)
#define ListView_GetExtendedListViewStyle(hwndLV)\
        (DWORD)SNDMSG((hwndLV), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0)

#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_SUBITEMIMAGES    0x00000002
#define LVS_EX_CHECKBOXES       0x00000004
#define LVS_EX_TRACKSELECT      0x00000008
#define LVS_EX_HEADERDRAGDROP   0x00000010
#define LVS_EX_FULLROWSELECT    0x00000020
#define LVS_EX_ONECLICKACTIVATE 0x00000040
#define LVS_EX_TWOCLICKACTIVATE 0x00000080


#define LPNM_LISTVIEW   LPNMLISTVIEW
#define NM_LISTVIEW     NMLISTVIEW

typedef struct tagNMLISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;
    UINT    uOldState;
    UINT    uChanged;
    POINT   ptAction;
    LPARAM  lParam;
} NMLISTVIEW, FAR *LPNMLISTVIEW;


#define LVN_ITEMCHANGING        (LVN_FIRST-0)
#define LVN_ITEMCHANGED         (LVN_FIRST-1)
#define LVN_INSERTITEM          (LVN_FIRST-2)
#define LVN_DELETEITEM          (LVN_FIRST-3)
#define LVN_DELETEALLITEMS      (LVN_FIRST-4)
#define LVN_BEGINLABELEDITA     (LVN_FIRST-5)
#define LVN_BEGINLABELEDITW     (LVN_FIRST-75)
#define LVN_ENDLABELEDITA       (LVN_FIRST-6)
#define LVN_ENDLABELEDITW       (LVN_FIRST-76)
#define LVN_COLUMNCLICK         (LVN_FIRST-8)
#define LVN_BEGINDRAG           (LVN_FIRST-9)
#define LVN_BEGINRDRAG          (LVN_FIRST-11)

#define LVN_GETDISPINFOA        (LVN_FIRST-50)
#define LVN_GETDISPINFOW        (LVN_FIRST-77)
#define LVN_SETDISPINFOA        (LVN_FIRST-51)
#define LVN_SETDISPINFOW        (LVN_FIRST-78)

#define LVN_KEYDOWN             (LVN_FIRST-55)

#define LV_KEYDOWN              NMLVKEYDOWN

typedef struct tagLVKEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
}
NMLVKEYDOWN, FAR *LPNMLVKEYDOWN;

#define LVN_MARQUEEBEGIN        (LVN_FIRST-56)

typedef struct tagNMLVGETINFOTIPA
{
    NMHDR		hdr;
    DWORD		dwFlags;
    LPTSTR		pszText;
    int			cchTextMax;
    int			iItem;
    int			iSubItem;
    LPARAM		lParam;
} NMLVGETINFOTIP, *LPNMLVGETINFOTIP;

#define LVGIT_UNFOLDED  0x0001

#define LVN_GETINFOTIP          (LVN_FIRST-58)


// TREEVIEW CONTROL -------------------------------------------------------------

#define WC_TREEVIEW             _T("SysTreeView32")

#define TVS_HASBUTTONS          0x0001
#define TVS_HASLINES            0x0002
#define TVS_LINESATROOT         0x0004
#define TVS_EDITLABELS          0x0008
#define TVS_DISABLEDRAGDROP     0x0010
#define TVS_SHOWSELALWAYS       0x0020
#define TVS_RTLREADING          0x0040

#define TVS_NOTOOLTIPS          0x0080
#define TVS_CHECKBOXES          0x0100
#define TVS_TRACKSELECT         0x0200
#define TVS_SINGLEEXPAND        0x0400
#define TVS_INFOTIP             0x0800
#define TVS_FULLROWSELECT       0x1000
#define TVS_NOSCROLL            0x2000
#define TVS_NONEVENHEIGHT       0x4000

typedef struct _TREEITEM FAR* HTREEITEM;

#define TVIF_TEXT               0x0001
#define TVIF_IMAGE              0x0002
#define TVIF_PARAM              0x0004
#define TVIF_STATE              0x0008
#define TVIF_HANDLE             0x0010
#define TVIF_SELECTEDIMAGE      0x0020
#define TVIF_CHILDREN           0x0040
#define TVIF_INTEGRAL           0x0080
#define TVIS_SELECTED           0x0002
#define TVIS_CUT                0x0004
#define TVIS_DROPHILITED        0x0008
#define TVIS_BOLD               0x0010
#define TVIS_EXPANDED           0x0020
#define TVIS_EXPANDEDONCE       0x0040
#define TVIS_EXPANDPARTIAL      0x0080

#define TVIS_OVERLAYMASK        0x0F00
#define TVIS_STATEIMAGEMASK     0xF000
#define TVIS_USERMASK           0xF000

#define I_CHILDRENCALLBACK  (-1)

#define LPTV_ITEM               LPTVITEM
#define TV_ITEM                 TVITEM

typedef struct tagTVITEM {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPTSTR    pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
} TVITEM, FAR *LPTVITEM;

#define TVI_ROOT                ((HTREEITEM)0xFFFF0000)
#define TVI_FIRST               ((HTREEITEM)0xFFFF0001)
#define TVI_LAST                ((HTREEITEM)0xFFFF0002)
#define TVI_SORT                ((HTREEITEM)0xFFFF0003)

#define LPTV_INSERTSTRUCT       LPTVINSERTSTRUCT
#define TV_INSERTSTRUCT         TVINSERTSTRUCT

#define TVINSERTSTRUCTA_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTA, item)
#define TVINSERTSTRUCTW_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTW, item)

typedef struct tagTVINSERTSTRUCT
{
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TV_ITEM   item;
}
TVINSERTSTRUCT, FAR *LPTVINSERTSTRUCT;


#define TVM_INSERTITEM          (TV_FIRST + 50)

#define TreeView_InsertItem(hwnd, lpis) \
    (HTREEITEM)SNDMSG((hwnd), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(lpis))


#define TVM_DELETEITEM          (TV_FIRST + 1)
#define TreeView_DeleteItem(hwnd, hitem) \
    (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)(hitem))


#define TreeView_DeleteAllItems(hwnd) \
    (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT)


#define TVM_EXPAND              (TV_FIRST + 2)
#define TreeView_Expand(hwnd, hitem, code) \
    (BOOL)SNDMSG((hwnd), TVM_EXPAND, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TVE_COLLAPSE            0x0001
#define TVE_EXPAND              0x0002
#define TVE_TOGGLE              0x0003
#define TVE_EXPANDPARTIAL       0x4000
#define TVE_COLLAPSERESET       0x8000


#define TVM_GETITEMRECT         (TV_FIRST + 4)
#define TreeView_GetItemRect(hwnd, hitem, prc, code) \
    (*(HTREEITEM FAR *)prc = (hitem), (BOOL)SNDMSG((hwnd), TVM_GETITEMRECT, (WPARAM)(code), (LPARAM)(RECT FAR*)(prc)))


#define TVM_GETCOUNT            (TV_FIRST + 5)
#define TreeView_GetCount(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETCOUNT, 0, 0)


#define TVM_GETINDENT           (TV_FIRST + 6)
#define TreeView_GetIndent(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETINDENT, 0, 0)


#define TVM_SETINDENT           (TV_FIRST + 7)
#define TreeView_SetIndent(hwnd, indent) \
    (BOOL)SNDMSG((hwnd), TVM_SETINDENT, (WPARAM)indent, 0)


#define TVM_GETIMAGELIST        (TV_FIRST + 8)
#define TreeView_GetImageList(hwnd, iImage) \
    (HIMAGELIST)SNDMSG((hwnd), TVM_GETIMAGELIST, iImage, 0)


#define TVSIL_NORMAL            0
#define TVSIL_STATE             2


#define TVM_SETIMAGELIST        (TV_FIRST + 9)
#define TreeView_SetImageList(hwnd, himl, iImage) \
	(HIMAGELIST)SNDMSG((hwnd), TVM_SETIMAGELIST, iImage, (LPARAM)(HIMAGELIST)(himl))


#define TVM_GETNEXTITEM         (TV_FIRST + 10)
#define TreeView_GetNextItem(hwnd, hitem, code) \
    (HTREEITEM)SNDMSG((hwnd), TVM_GETNEXTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TVGN_ROOT               0x0000
#define TVGN_NEXT               0x0001
#define TVGN_PREVIOUS           0x0002
#define TVGN_PARENT             0x0003
#define TVGN_CHILD              0x0004
#define TVGN_FIRSTVISIBLE       0x0005
#define TVGN_NEXTVISIBLE        0x0006
#define TVGN_PREVIOUSVISIBLE    0x0007
#define TVGN_DROPHILITE         0x0008
#define TVGN_CARET              0x0009
#define TVGN_LASTVISIBLE        0x000A

#define TreeView_GetChild(hwnd, hitem)          TreeView_GetNextItem(hwnd, hitem, TVGN_CHILD)
#define TreeView_GetNextSibling(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)
#define TreeView_GetPrevSibling(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUS)
#define TreeView_GetParent(hwnd, hitem)         TreeView_GetNextItem(hwnd, hitem, TVGN_PARENT)
#define TreeView_GetFirstVisible(hwnd)          TreeView_GetNextItem(hwnd, NULL,  TVGN_FIRSTVISIBLE)
#define TreeView_GetNextVisible(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
#define TreeView_GetPrevVisible(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE)
#define TreeView_GetSelection(hwnd)             TreeView_GetNextItem(hwnd, NULL,  TVGN_CARET)
#define TreeView_GetDropHilight(hwnd)           TreeView_GetNextItem(hwnd, NULL,  TVGN_DROPHILITE)
#define TreeView_GetRoot(hwnd)                  TreeView_GetNextItem(hwnd, NULL,  TVGN_ROOT)
#define TreeView_GetLastVisible(hwnd)           TreeView_GetNextItem(hwnd, NULL,  TVGN_LASTVISIBLE)


#define TVM_SELECTITEM          (TV_FIRST + 11)
#define TreeView_Select(hwnd, hitem, code) \
    (BOOL)SNDMSG((hwnd), TVM_SELECTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TreeView_SelectItem(hwnd, hitem)            TreeView_Select(hwnd, hitem, TVGN_CARET)
#define TreeView_SelectDropTarget(hwnd, hitem)      TreeView_Select(hwnd, hitem, TVGN_DROPHILITE)
#define TreeView_SelectSetFirstVisible(hwnd, hitem) TreeView_Select(hwnd, hitem, TVGN_FIRSTVISIBLE)

#define TVM_GETITEM             (TV_FIRST + 62)

#define TreeView_GetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), TVM_GETITEM, 0, (LPARAM)(TV_ITEM FAR*)(pitem))


#define TVM_SETITEM             (TV_FIRST + 63)

#define TreeView_SetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), TVM_SETITEM, 0, (LPARAM)(const TV_ITEM FAR*)(pitem))


#define TVM_EDITLABEL           (TV_FIRST + 65)

#define TreeView_EditLabel(hwnd, hitem) \
    (HWND)SNDMSG((hwnd), TVM_EDITLABEL, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_GETEDITCONTROL      (TV_FIRST + 15)
#define TreeView_GetEditControl(hwnd) \
    (HWND)SNDMSG((hwnd), TVM_GETEDITCONTROL, 0, 0)


#define TVM_GETVISIBLECOUNT     (TV_FIRST + 16)
#define TreeView_GetVisibleCount(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETVISIBLECOUNT, 0, 0)


#define TVM_CREATEDRAGIMAGE     (TV_FIRST + 18)
#define TreeView_CreateDragImage(hwnd, hitem) \
    (HIMAGELIST)SNDMSG((hwnd), TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_SORTCHILDREN        (TV_FIRST + 19)
#define TreeView_SortChildren(hwnd, hitem, recurse) \
    (BOOL)SNDMSG((hwnd), TVM_SORTCHILDREN, (WPARAM)recurse, (LPARAM)(HTREEITEM)(hitem))


#define TVM_ENSUREVISIBLE       (TV_FIRST + 20)
#define TreeView_EnsureVisible(hwnd, hitem) \
    (BOOL)SNDMSG((hwnd), TVM_ENSUREVISIBLE, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_SORTCHILDRENCB      (TV_FIRST + 21)
#define TreeView_SortChildrenCB(hwnd, psort, recurse) \
    (BOOL)SNDMSG((hwnd), TVM_SORTCHILDRENCB, (WPARAM)recurse, \
    (LPARAM)(LPTV_SORTCB)(psort))


#define TVM_ENDEDITLABELNOW     (TV_FIRST + 22)
#define TreeView_EndEditLabelNow(hwnd, fCancel) \
    (BOOL)SNDMSG((hwnd), TVM_ENDEDITLABELNOW, (WPARAM)fCancel, 0)


#define TVM_SETTOOLTIPS         (TV_FIRST + 24)
#define TreeView_SetToolTips(hwnd,  hwndTT) \
    (HWND)SNDMSG((hwnd), TVM_SETTOOLTIPS, (WPARAM)hwndTT, 0)

#define TVM_GETTOOLTIPS         (TV_FIRST + 25)
#define TreeView_GetToolTips(hwnd) \
    (HWND)SNDMSG((hwnd), TVM_GETTOOLTIPS, 0, 0)

#define TVM_SETITEMHEIGHT         (TV_FIRST + 27)
#define TreeView_SetItemHeight(hwnd,  iHeight) \
    (int)SNDMSG((hwnd), TVM_SETITEMHEIGHT, (WPARAM)iHeight, 0)

#define TVM_GETITEMHEIGHT         (TV_FIRST + 28)
#define TreeView_GetItemHeight(hwnd) \
    (int)SNDMSG((hwnd), TVM_GETITEMHEIGHT, 0, 0)

#define TVM_SETBKCOLOR              (TV_FIRST + 29)
#define TreeView_SetBkColor(hwnd, clr) \
    (COLORREF)SNDMSG((hwnd), TVM_SETBKCOLOR, 0, (LPARAM)clr)

#define TVM_SETTEXTCOLOR              (TV_FIRST + 30)
#define TreeView_SetTextColor(hwnd, clr) \
    (COLORREF)SNDMSG((hwnd), TVM_SETTEXTCOLOR, 0, (LPARAM)clr)

#define TVM_GETBKCOLOR              (TV_FIRST + 31)
#define TreeView_GetBkColor(hwnd) \
    (COLORREF)SNDMSG((hwnd), TVM_GETBKCOLOR, 0, 0)

#define TVM_GETTEXTCOLOR              (TV_FIRST + 32)
#define TreeView_GetTextColor(hwnd) \
    (COLORREF)SNDMSG((hwnd), TVM_GETTEXTCOLOR, 0, 0)

typedef int (CALLBACK *PFNTVCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#define LPTV_SORTCB    LPTVSORTCB
#define   TV_SORTCB      TVSORTCB

typedef struct tagTVSORTCB
{
    HTREEITEM       hParent;
    PFNTVCOMPARE    lpfnCompare;
    LPARAM          lParam;
}
TVSORTCB, FAR *LPTVSORTCB;


#define LPNM_TREEVIEW           LPNMTREEVIEW
#define NM_TREEVIEW             NMTREEVIEW

typedef struct tagNMTREEVIEW
{
    NMHDR       hdr;
    UINT        action;
    TVITEM      itemOld;
    TVITEM      itemNew;
    POINT       ptDrag;
}
NMTREEVIEW, FAR *LPNMTREEVIEW;


#define TVN_SELCHANGING         (TVN_FIRST-50)
#define TVN_SELCHANGED          (TVN_FIRST-51)

#define TVC_UNKNOWN             0x0000
#define TVC_BYMOUSE             0x0001
#define TVC_BYKEYBOARD          0x0002


#define TVN_ITEMEXPANDING       (TVN_FIRST-54)
#define TVN_ITEMEXPANDED        (TVN_FIRST-55)
#define TVN_BEGINDRAG           (TVN_FIRST-56)
#define TVN_BEGINRDRAG          (TVN_FIRST-57)
#define TVN_DELETEITEM          (TVN_FIRST-58)
#define TVN_BEGINLABELEDIT      (TVN_FIRST-59)
#define TVN_ENDLABELEDIT        (TVN_FIRST-60)
#define TVN_KEYDOWN             (TVN_FIRST-12)

#define TVN_GETINFOTIP          (TVN_FIRST-14)
#define TVN_SINGLEEXPAND        (TVN_FIRST-15)


#define TV_KEYDOWN      NMTVKEYDOWN

typedef struct tagTVKEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
}
NMTVKEYDOWN, FAR *LPNMTVKEYDOWN;

// for tooltips

typedef struct tagNMTVGETINFOTIP
{
    NMHDR		hdr;
    LPTSTR		pszText;
    int			cchTextMax;
    HTREEITEM	hItem;
    LPARAM		lParam;
}
NMTVGETINFOTIP, *LPNMTVGETINFOTIP;


// treeview's customdraw return meaning don't draw images.  valid on CDRF_NOTIFYITEMPREPAINT
#define TVCDRF_NOIMAGES         0x00010000

// Standard Trackbar CONTROL -------------------------------------------------------------

#define TRACKBAR_CLASS          _T("msctls_trackbar32")

#define TBS_AUTOTICKS           0x0001
#define TBS_VERT                0x0002
#define TBS_HORZ                0x0000
#define TBS_TOP                 0x0004
#define TBS_BOTTOM              0x0000
#define TBS_LEFT                0x0004
#define TBS_RIGHT               0x0000
#define TBS_BOTH                0x0008
#define TBS_NOTICKS             0x0010
#define TBS_ENABLESELRANGE      0x0020
#define TBS_FIXEDLENGTH         0x0040
#define TBS_NOTHUMB             0x0080

#define TBM_GETPOS              (WM_USER)
#define TBM_GETRANGEMIN         (WM_USER+1)
#define TBM_GETRANGEMAX         (WM_USER+2)
#define TBM_GETTIC              (WM_USER+3)
#define TBM_SETTIC              (WM_USER+4)
#define TBM_SETPOS              (WM_USER+5)
#define TBM_SETRANGE            (WM_USER+6)
#define TBM_SETRANGEMIN         (WM_USER+7)
#define TBM_SETRANGEMAX         (WM_USER+8)
#define TBM_CLEARTICS           (WM_USER+9)
#define TBM_SETSEL              (WM_USER+10)
#define TBM_SETSELSTART         (WM_USER+11)
#define TBM_SETSELEND           (WM_USER+12)
#define TBM_GETPTICS            (WM_USER+14)
#define TBM_GETTICPOS           (WM_USER+15)
#define TBM_GETNUMTICS          (WM_USER+16)
#define TBM_GETSELSTART         (WM_USER+17)
#define TBM_GETSELEND           (WM_USER+18)
#define TBM_CLEARSEL            (WM_USER+19)
#define TBM_SETTICFREQ          (WM_USER+20)
#define TBM_SETPAGESIZE         (WM_USER+21)
#define TBM_GETPAGESIZE         (WM_USER+22)
#define TBM_SETLINESIZE         (WM_USER+23)
#define TBM_GETLINESIZE         (WM_USER+24)
#define TBM_GETTHUMBRECT        (WM_USER+25)
#define TBM_GETCHANNELRECT      (WM_USER+26)
#define TBM_SETTHUMBLENGTH      (WM_USER+27)
#define TBM_GETTHUMBLENGTH      (WM_USER+28)


// hacks for layered (alpha blended) windows
//
#define LWA_ALPHA				0x00000002
#define LWA_COLORKEY			0x00000001

BOOL SetLayeredWindowAttributes(HWND hwnd, COLORREF key, BYTE alpha, DWORD flags);

/////////////////////////////////////////////////////////////////////////////
// Standard Commands

// File commands
#define ID_FILE_NEW                     0xE100
#define ID_FILE_OPEN                    0xE101
#define ID_FILE_CLOSE                   0xE102
#define ID_FILE_SAVE                    0xE103
#define ID_FILE_SAVE_AS                 0xE104
#define ID_FILE_PAGE_SETUP              0xE105
#define ID_FILE_PRINT_SETUP             0xE106
#define ID_FILE_PRINT                   0xE107
#define ID_FILE_PRINT_DIRECT            0xE108
#define ID_FILE_PRINT_PREVIEW           0xE109
#define ID_FILE_UPDATE                  0xE10A
#define ID_FILE_SAVE_COPY_AS            0xE10B
#define ID_FILE_SEND_MAIL               0xE10C

#define ID_FILE_MRU_FIRST               0xE110
#define ID_FILE_MRU_FILE1               0xE110
#define ID_FILE_MRU_FILE2               0xE111
#define ID_FILE_MRU_FILE3               0xE112
#define ID_FILE_MRU_FILE4               0xE113
#define ID_FILE_MRU_FILE5               0xE114
#define ID_FILE_MRU_FILE6               0xE115
#define ID_FILE_MRU_FILE7               0xE116
#define ID_FILE_MRU_FILE8               0xE117
#define ID_FILE_MRU_FILE9               0xE118
#define ID_FILE_MRU_FILE10              0xE119
#define ID_FILE_MRU_FILE11              0xE11A
#define ID_FILE_MRU_FILE12              0xE11B
#define ID_FILE_MRU_FILE13              0xE11C
#define ID_FILE_MRU_FILE14              0xE11D
#define ID_FILE_MRU_FILE15              0xE11E
#define ID_FILE_MRU_FILE16              0xE11F
#define ID_FILE_MRU_LAST                0xE11F

// Edit commands
#define ID_EDIT_CLEAR                   0xE120
#define ID_EDIT_CLEAR_ALL               0xE121
#define ID_EDIT_COPY                    0xE122
#define ID_EDIT_CUT                     0xE123
#define ID_EDIT_FIND                    0xE124
#define ID_EDIT_PASTE                   0xE125
#define ID_EDIT_PASTE_LINK              0xE126
#define ID_EDIT_PASTE_SPECIAL           0xE127
#define ID_EDIT_REPEAT                  0xE128
#define ID_EDIT_REPLACE                 0xE129
#define ID_EDIT_SELECT_ALL              0xE12A
#define ID_EDIT_UNDO                    0xE12B
#define ID_EDIT_REDO                    0xE12C

// Window commands
#define ID_WINDOW_NEW                   0xE130
#define ID_WINDOW_ARRANGE               0xE131
#define ID_WINDOW_CASCADE               0xE132
#define ID_WINDOW_TILE_HORZ             0xE133
#define ID_WINDOW_TILE_VERT             0xE134
#define ID_WINDOW_SPLIT                 0xE135

#define ID_APP_ABOUT                    0xE140
#define ID_APP_EXIT                     0xE141
#define ID_HELP_INDEX                   0xE142
#define ID_HELP_FINDER                  0xE143
#define ID_HELP_USING                   0xE144
#define ID_CONTEXT_HELP                 0xE145      // shift-F1

#define ID_HELP                         0xE146      // first attempt for F1
#define ID_DEFAULT_HELP                 0xE147      // last attempt

// Misc
#define ID_NEXT_PANE                    0xE150
#define ID_PREV_PANE                    0xE151

// Format
#define ID_FORMAT_FONT                  0xE160


#define ID_VIEW_TOOLBAR                 0xE800
#define ID_VIEW_STATUS_BAR              0xE801
#define ID_VIEW_REBAR                   0xE804

#define ID_VIEW_SMALLICON               0xE810
#define ID_VIEW_LARGEICON               0xE811
#define ID_VIEW_LIST                    0xE812
#define ID_VIEW_DETAILS                 0xE813
#define ID_VIEW_LINEUP                  0xE814
#define ID_VIEW_BYNAME                  0xE815

#define ID_RECORD_FIRST                 0xE900
#define ID_RECORD_LAST                  0xE901
#define ID_RECORD_NEXT                  0xE902
#define ID_RECORD_PREV                  0xE903

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif
#define IDC_STATIC						(-1)


// Common Dialogs ---------------------------------------------------------------------
// ------------------------------------------------------------------------------------

// printer stuff
//
typedef void (*LPPRINTHOOKPROC)(void);
typedef void (*LPSETUPHOOKPROC)(void);

// print dlg struct
typedef struct
{
   DWORD            lStructSize;
   HWND             hwndOwner;
   HGLOBAL          hDevMode;
   HGLOBAL          hDevNames;
   HDC              hDC;
   DWORD            Flags;
   WORD             nFromPage;
   WORD             nToPage;
   WORD             nMinPage;
   WORD             nMaxPage;
   WORD             nCopies;
   HINSTANCE        hInstance;
   LPARAM           lCustData;
   LPPRINTHOOKPROC  lpfnPrintHook;
   LPSETUPHOOKPROC  lpfnSetupHook;
   LPCTSTR          lpPrintTemplateName;
   LPCTSTR          lpSetupTemplateName;
   HGLOBAL          hPrintTemplate;
   HGLOBAL          hSetupTemplate;
}
PRINTDLG, FAR* LPPRINTDLG;

BOOL WINAPI PrintDlg(LPPRINTDLG);

#define PD_ALLPAGES			0x00000000
#define PD_SELECTION		0x00000001
#define PD_PAGENUMS			0x00000002
#define PD_NOSELECTION		0x00000004
#define PD_NOPAGENUMS		0x00000008
#define PD_COLLATE			0x00000010
#define PD_PRINTTOFILE		0x00000020
#define PD_PRINTSETUP		0x00000040
#define PD_NOWARNING		0x00000080

// misc
//
LPTSTR	GetCommandLine(void);

#ifdef __cplusplus
} // extern C
#endif

#endif // _BSAWINAPI_H_


