#ifndef Windows

#include "winapix.h"


#define MAX_TARGETS 8

#ifdef X11

// owner Window of clip
//
static Window _zg_clipwindow = 0;

// Atoms of the two X selections we're dealing with:
// CLIPBOARD (explicit-copy) and PRIMARY (selection-copy)
//
static Atom clipboard_atom, primary_atom;

// Atom of the TARGETS clipboard target
//
static Atom targets_atom;

// Atom of the TIMESTAMP clipboard target
//
static Atom timestamp_atom;

// Atom _WINAPI_CLIPBOARD_TARGET which is used as the 'property' argument in
//   XConvertSelection calls: This is the property of our window into which
//   XConvertSelection will store the received clipboard data. */
//
static Atom winapi_clipboard_target_atom;

// Atoms _WINAPI_PRIMARY_TIMESTAMP_TARGET and _WINAPI_CLIPBOARD_TIMESTAMP_TARGET
//   are used to store the timestamps for when a window got ownership of the selections.
//   We use these to determine which is more recent and should be used.
//
static Atom winapi_primary_timestamp_target_atom, winapi_clipboard_timestamp_target_atom;

// Storage for timestamps since we get them in two separate notifications.
//
static Time acquire_time;

// Clipboard target for getting a list of native Windows clipboard formats. The
//   presence of this target indicates that the selection owner is another winapi.
//
static Atom winapi_clipboard_formats_atom;

// We need to know when another winapi process gets or loses ownership of a
//   selection. Without XFixes we do this by touching a property on the root window
//   which will generate PropertyNotify notifications.
static Atom winapi_selection_notify_atom;

// Atoms _WINAPI_PRIMARY_OWNER and _WINAPI_CLIPBOARD_OWNER. Used as properties
//   on the root window to indicate which selections that are owned by winapi.
//
static Atom winapi_primary_owner_atom, winapi_clipboard_owner_atom;
static Atom format_string_atom, format_utf8_string_atom, format_unicode_atom;

// Array of offered clipboard targets that will be sent to fellow X
// clients upon a TARGETS request.
//
static Atom targets[MAX_TARGETS];
static int num_targets;

#endif // X11

static BYTE *_zg_clipdata = NULL;
static DWORD _zg_clipdatalen = 0;

static BYTE *formats_data = NULL;
static DWORD formats_data_length = 0;

static int s_gotnot = 0;

//**************************************************************************
int _w_xclip_utf8_encode(BYTE* data, int datalen)
{
	unsigned char  utfbuf[8];
	unsigned char* src = data, *dst;
	unsigned short ca, cb, cc, cd;
	wchar_t  uc, nc;
	int  j, k;
	size_t i, len;

#ifdef UNICODE
	len = wcslen((WCHAR*)src);
#else
	WCHAR* ps = (WCHAR*)src;
	
	for(len = 0; ps && *ps; ps++)
	{
		len++;
	}
#endif
	if((datalen / sizeof(WCHAR)) < len)
	{
		len = datalen / sizeof(WCHAR);
	}
	if(_zg_clipdata)
	{
		free(_zg_clipdata);
		_zg_clipdata = NULL;
	}
	_zg_clipdata = malloc(len * 3 + 6);
	if(! _zg_clipdata)
	{
		return 1;
	}
	dst = _zg_clipdata;

	for(i = 0; i < len; i++)
	{
		/* first get the unicode character
		*/
		ca = (unsigned char)*src++;
		cb = (unsigned char)*src++;
#if BYTE_SWAPPED
		nc = ca | (cb << 8);
#else
		nc = (ca << 8) | cb;
#endif  	  
		if(sizeof(WCHAR) == 4) // it is on Linux, etc.
		{
		 cc = (unsigned char)*src++;
		 cd = (unsigned char)*src++;
#if BYTE_SWAPPED
		 uc = cc | (cd << 8);
		 nc = nc | (uc << 16);
#else
		 uc = (cc << 8) | cd;
		 nc = (nc << 8) | uc;
#endif
		}
		/* next, UTF-8 encode it
		*/
		j = 0;
		
		if (nc < 0x80) {
			utfbuf[j++] = (unsigned char)nc;
		}
		else if (nc < 0x800) {
			utfbuf[j++] = 0xC0 | (nc >> 6);
			utfbuf[j++] = 0x80 | (nc & 0x3F);
		}
		else if (nc < 0x10000) {
			utfbuf[j++] = 0xE0 | (nc >> 12);
			utfbuf[j++] = 0x80 | ((nc >> 6) & 0x3F);
			utfbuf[j++] = 0x80 | (nc  & 0x3F);
		}
		else if (nc < 0x200000) {
			utfbuf[j++] = 0xF0 | (nc >> 18);
			utfbuf[j++] = 0x80 | ((nc >> 12) & 0x3F);
			utfbuf[j++] = 0x80 | ((nc >> 6) & 0x3F);
			utfbuf[j++] = 0x80 | (nc  & 0x3F);
		}
		for(k = 0; k < j; k++)
		{
			*dst++ = (char)utfbuf[k];
		}
	}
	*dst = '\0';
	_zg_clipdatalen = dst - _zg_clipdata; 
	return 0;
}

//**************************************************************************
int _w_xclip_copy(BYTE* data, int datalen)
{
	if(_zg_clipdata)
	{
		free(_zg_clipdata);
		_zg_clipdata = NULL;
	}
	_zg_clipdata = malloc(datalen + 8);
	if(! _zg_clipdata)
	{
		return 1;
	}
	memcpy(_zg_clipdata, data, datalen);
	_zg_clipdata[_zg_clipdatalen] = 0;
	_zg_clipdata[_zg_clipdatalen + 1] = 0;
	_zg_clipdata[_zg_clipdatalen + 2] = 0;
	_zg_clipdata[_zg_clipdatalen + 3] = 0;
	_zg_clipdatalen = datalen; 
	return 0;
}

//**************************************************************************
int _w_xclip_utf8_decode(BYTE* src, int datalen)
{
	int len;
	unsigned long b, c;
	WCHAR* dst;

	if(_zg_clipdata)
	{
		free(_zg_clipdata);
		_zg_clipdata = NULL;
	}
	len = strlen((char*)src);
	if(datalen < len)
	{
		len = datalen;
	}
	_zg_clipdata = malloc((len + 4) * sizeof(WCHAR));
	if(! _zg_clipdata)
	{
		return 1;
	}
	dst = (WCHAR*)_zg_clipdata;
	
	while(*src && (len > 0))
	{
		b = *src++;
		len--;
		
		if(b & 0x80)
		{
			if(b & 0x20)
			{
				if(b & 0x10)
				{
					b &= 0x7;
					b <<= 18;
					c = *src++; len--;
					b |= (c & 0x3f) << 12;
					c = *src++; len--;
					b |= (c & 0x3f) << 6;
					c = *src++; len--;
					b |= (c & 0x3f);
				}
				else
				{
					b &= 0xf;
					b <<= 12;
					c = *src++; len--;
					b |= (c & 0x3f) << 6;
					c = *src++; len--;
					b |= (c & 0x3f);
				}
			}
			else
			{
				b &= 0x1f;
				b <<= 6;
				c = *src++; len--;
				b |= c & 0x3f;
			}
		}
		*dst++ = (WCHAR)b;
	}
	*dst++ = 0;
	_zg_clipdatalen = (char*)dst - (char*)_zg_clipdata;
	return 0;
}

//**************************************************************************
BYTE *_w_xclip_data(DWORD *len)
{
	if(len)	*len = _zg_clipdatalen;
	return _zg_clipdata;
}

#ifdef X11
//**************************************************************************
static void xclip_notify_change()
{
	XChangeProperty(_zg_display, DefaultRootWindow(_zg_display),
			winapi_selection_notify_atom, XA_INTEGER, 32, PropModeReplace, NULL, 0);
}

//**************************************************************************
static void xclip_provide_selection(
			XSelectionRequestEvent *req,
			Atom type,
			unsigned int format,
			BYTE *data,
			DWORD length
		)
{
	XEvent xev;
	/*
	printf("xclip_provide_selection: requestor=0x%08x, target=%s, property=%s, length=%u\n",
			(unsigned) req->requestor, XGetAtomName(_zg_display, req->target), XGetAtomName(_zg_display, req->property), (unsigned) length);
	*/
	XChangeProperty(_zg_display, req->requestor, req->property, type, format, PropModeReplace, data, length);

	xev.xselection.type 		= SelectionNotify;
	xev.xselection.serial 		= 0;
	xev.xselection.send_event 	= True;
	xev.xselection.requestor	= req->requestor;
	xev.xselection.selection	= req->selection;
	xev.xselection.target 		= req->target;
	xev.xselection.property		= req->property;
	xev.xselection.time 		= req->time;

	XSendEvent(_zg_display, req->requestor, False, NoEventMask, &xev);
	XFlush(_zg_display);
}

//**************************************************************************
static void xclip_refuse_selection(XSelectionRequestEvent * req)
{
	XEvent xev;
	/*
	printf("xclip_refuse_selection: requestor=0x%08x, target=%s, property=%s\n",
			 (unsigned) req->requestor, XGetAtomName(_zg_display, req->target),
			 XGetAtomName(_zg_display, req->property));
	*/
	xev.xselection.type 		= SelectionNotify;
	xev.xselection.serial 		= 0;
	xev.xselection.send_event 	= True;
	xev.xselection.requestor 	= req->requestor;
	xev.xselection.selection	= req->selection;
	xev.xselection.target 		= req->target;
	xev.xselection.property		= None;
	xev.xselection.time 		= req->time;

	XSendEvent(_zg_display, req->requestor, False, NoEventMask, &xev);
	XFlush(_zg_display);
}

//**************************************************************************
void _w_clipSelectionRequest(XEvent event)
{
	XSelectionRequestEvent *req;
	char* data = NULL;
	int datalen = 0;
		
	req = &event.xselectionrequest;
	/*
	printf("handle_SelectionRequest: selection=%s, target=%s, property=%s\n",
			 XGetAtomName(_zg_display, req->selection),
			 XGetAtomName(_zg_display, req->target),
			 XGetAtomName(_zg_display, req->property));
	*/
	if (req->target == targets_atom)
	{
		xclip_provide_selection(req, XA_ATOM, 32, (BYTE *)&targets, num_targets);
	}
	else if(req->target == timestamp_atom)
	{
		xclip_provide_selection(req, XA_INTEGER, 32, (BYTE *)&acquire_time, 1);
	}
	else if(req->target == winapi_clipboard_formats_atom)
	{
		xclip_provide_selection(req, XA_STRING, 8, formats_data, formats_data_length);
	}
	else if(req->target == format_string_atom || req->target == XA_STRING)
	{
		if(_zg_clipboard != NULL)
		{
			data = XFetchBytes(_zg_display, &datalen);
		}
		if(data == NULL || datalen <= 0)
		{
			xclip_refuse_selection(req);
		}
		else
		{
			if(_zg_clipformat == CF_TEXT)
			{			
				xclip_provide_selection(req, req->target, 8, (BYTE*)data, datalen);
			}
			else
			{
				xclip_refuse_selection(req);
			}
		}
	}
	else if (req->target == format_utf8_string_atom)
	{
		if(_zg_clipboard != NULL)
		{
			data = XFetchBytes(_zg_display, &datalen);
		}
		if(data == NULL || datalen <= 0)
		{
			xclip_refuse_selection(req);
		}
		else
		{
			if(_zg_clipformat == CF_UNICODETEXT)
			{
				_w_xclip_utf8_encode((BYTE*)data, datalen);
				xclip_provide_selection(req, req->target, 8, _zg_clipdata, _zg_clipdatalen);
			}
			else if(_zg_clipformat == CF_TEXT)
			{
				xclip_provide_selection(req, req->target, 8, (BYTE*)data, datalen);
			}
		}
	}
	else if (req->target == format_unicode_atom)
	{
		if(_zg_clipboard != NULL)
		{
			data = XFetchBytes(_zg_display, &datalen);
		}
		if(data == NULL || datalen <= 0)
		{
			xclip_refuse_selection(req);
		}
		else
		{
			if(_zg_clipformat == CF_UNICODETEXT)
			{
				xclip_provide_selection(req, req->target, 8, (BYTE*)data, datalen);
			}
			else
			{
				xclip_refuse_selection(req);
			}
		}
	}
	else
	{
		xclip_refuse_selection(req);
	}
}

//**************************************************************************
void _w_clipSelectionNotify(XEvent event)
{
	XSelectionEvent* sn;
	
	sn = &event.xselection;
	if(sn->property == None)
	{
		s_gotnot = -1;
		//printf("XConvert failed\n");
	}
	else
	{
		s_gotnot = 1;
		/*
		printf("SelectionNotify: selection=%s, target=%s, property=%s\n",
			 XGetAtomName(_zg_display, sn->selection),
			 XGetAtomName(_zg_display, sn->target),
			 XGetAtomName(_zg_display, sn->property));
	    */
	}
}

//**************************************************************************
void _w_clipSelectionClear(XEvent event)
{
	// I don't really care about this, it just means some other window
	// took ownership of the clipboard.
	;
}

//**************************************************************************
void _w_clipPropertyNotify(XEvent event)
{
	XPropertyEvent *pev;

	pev = &event.xproperty;

	if(
			(pev->atom == winapi_selection_notify_atom)
		&&	(pev->window == _zg_clipwindow)
	)
	{
		//printf("propnot\n");
	}
}

//**************************************************************************
char* _w_clipGetClipboardData(int* lenret, UINT format)
{
	Window so, me;
	BYTE *data;

	*lenret = 0;

	so = XGetSelectionOwner(_zg_display, clipboard_atom);	
	me = _zg_clipwindow;

	if(so == None)
	{
		// use our local clip buffer in case some other window
		// stole the clip board but isn't using it now, but
		// don't bother re-assering X ownership of it.
		//
		if(_zg_clipowner != NULL && _zg_clipboard != NULL)
		{
			so = me;
		}
	}
	if(so == me)
	{
		data = (BYTE*)XFetchBytes(_zg_display, lenret);
		
		if(format == CF_TEXT)
		{
			if(_zg_clipformat == CF_UNICODETEXT)
			{
				// utf8 encode it
				_w_xclip_utf8_encode((BYTE*)data, *lenret);
				*lenret = _zg_clipdatalen;
				//_tprintf(_T("my unidata to ansi=%s\n"), _zg_clipdata);
				return (char*)_zg_clipdata;
			}
			else
			{
				//_tprintf(_T("my ansidata to ansi=%s\n"), data);
				return (char*)data;
			}
		}
		else if(format == CF_UNICODETEXT)
		{
			if(_zg_clipformat == CF_TEXT)
			{
				// utf8 decode it
				_w_xclip_utf8_decode((BYTE*)data, *lenret);
				*lenret = _zg_clipdatalen;
				//_tprintf(_T("my ansidata to uni=%ls\n"), _zg_clipdata);
				return (char*)_zg_clipdata;
			}
			else
			{
				//_tprintf(_T("my unidata to uni=%s\n"), data);
				return (char*)data;
			}
		}		
	}
	else if(so != None)
	{
		int retformat;
		unsigned long items, bytesrem;
		Atom aFormat, type;
		
		if(format == CF_TEXT)
		{
			aFormat = format_string_atom;
		}
		else if(format == CF_UNICODETEXT)
		{
		//	aFormat = format_unicode_atom;
		// more apps support utf8, so get that and decode it
			aFormat = format_utf8_string_atom;
		}
		else
		{
			printf("unsupported clip format %d\n", format);
			return NULL;
		}
		// convert to string type we want
		//
		XConvertSelection(
				_zg_display,
				clipboard_atom,
				aFormat,
				winapi_clipboard_target_atom,
				me,
				CurrentTime
		);
		XFlush(_zg_display);
		
		// pump a few messages to allow conversion to complete
		// in foreign window (a success in property change)
		//
		{
			MSG msg;
			int cnt = 0;
			
			s_gotnot = 0;
			while(GetMessage(&msg, NULL, 0, 0)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if(++cnt > 100 || s_gotnot) break;
			}
			if(s_gotnot < 0 || cnt > 100)
			{
				// couldn't convert
				//
				return NULL;
			}
		}
		s_gotnot = 0;
		
		// see how big the data is
		//
		XGetWindowProperty(
				_zg_display,
				me,
				winapi_clipboard_target_atom,
				0,
				0,
				False,
				AnyPropertyType,
				&type,
				&retformat,
				&items,
				&bytesrem,
				&data
		);
		if(bytesrem > 0)
		{
			int result;
			unsigned long dummy;

			result = XGetWindowProperty(
				_zg_display,
				me,
				winapi_clipboard_target_atom,
				0,
				bytesrem,
				False,
				AnyPropertyType,
				&type,
				&retformat,
				&items,
				&dummy,
				&data
			);
			if(result == Success)
			{
				if(_zg_clipdata)
				{
					free(_zg_clipdata);
					_zg_clipdata = NULL;
				}
				// if asked for utf8, and wanted unicode
				// then decode utf8 into raw unicode
				//
				if(
						aFormat == format_utf8_string_atom
					||	(aFormat == format_string_atom && format == CF_UNICODETEXT)
				)
				{
					_w_xclip_utf8_decode(data, bytesrem);
					XFree(data);
					*lenret = _zg_clipdatalen;
					//_tprintf(_T("extern utf8data to uni=%ls\n"), _zg_clipdata);
					return (char*)_zg_clipdata;
				}
				else
				{
					_zg_clipdata = malloc(bytesrem + 32);
					if(_zg_clipdata)
					{
						_zg_clipdatalen = bytesrem;
						memcpy(_zg_clipdata, data, _zg_clipdatalen);
						XFree(data);
						*lenret = _zg_clipdatalen;
						//_tprintf(_T("extern ansi to ansi=%s\n"), _zg_clipdata);
						return (char*)_zg_clipdata;
					}
				}
				XFree(data);
			}
		}
		return NULL;
	}
	return NULL;
}

//**************************************************************************
void _w_clipOwnClipboard(UINT format)
{
	if(XGetSelectionOwner(_zg_display, clipboard_atom) != _zg_clipwindow)
	{
		XSetSelectionOwner(_zg_display, clipboard_atom, _zg_clipwindow, CurrentTime);
	}
}

//**************************************************************************
void _w_clipDisownClipboard(void)
{
	if(XGetSelectionOwner(_zg_display, clipboard_atom) == _zg_clipwindow)
	{
		XSetSelectionOwner(_zg_display, clipboard_atom, None, acquire_time);
	}
	if(_zg_clipdata)
	{
		free(_zg_clipdata);
		_zg_clipdata = NULL;
	}
}

//**************************************************************************
void _w_clipInit(void)
{
	// create a bunch of atoms to hang stuff off of
	//
	primary_atom 	= XInternAtom(_zg_display, "PRIMARY", False);
	clipboard_atom	= XInternAtom(_zg_display, "CLIPBOARD", False);
	targets_atom	= XInternAtom(_zg_display, "TARGETS", False);
	timestamp_atom	= XInternAtom(_zg_display, "TIMESTAMP", False);

	winapi_clipboard_target_atom 		 	= XInternAtom(_zg_display, "_WINAPI_CLIPBOARD_TARGET", False);
	winapi_primary_timestamp_target_atom	= XInternAtom(_zg_display, "_WINAPI_PRIMARY_TIMESTAMP_TARGET", False);
	winapi_clipboard_timestamp_target_atom	= XInternAtom(_zg_display, "_WINAPI_CLIPBOARD_TIMESTAMP_TARGET", False);

	format_string_atom 		= XInternAtom(_zg_display, "STRING", False);
	format_utf8_string_atom = XInternAtom(_zg_display, "UTF8_STRING", False);
	format_unicode_atom 	= XInternAtom(_zg_display, "text/unicode", False);

	// winapi sets _WINAPI_SELECTION_NOTIFY on the root window when acquiring the clipboard.
	//
	winapi_selection_notify_atom = XInternAtom(_zg_display, "_WINAPI_SELECTION_NOTIFY", False);
	
	winapi_clipboard_formats_atom	= XInternAtom(_zg_display, "_WINAPI_CLIPBOARD_FORMATS", False);
	winapi_primary_owner_atom 		= XInternAtom(_zg_display, "_WINAPI_PRIMARY_OWNER", False);
	winapi_clipboard_owner_atom		= XInternAtom(_zg_display, "_WINAPI_CLIPBOARD_OWNER", False);

	// fill out targets we can do
	num_targets = 0;
	targets[num_targets++] = targets_atom;
	targets[num_targets++] = timestamp_atom;
	targets[num_targets++] = winapi_clipboard_formats_atom;
#ifdef UNICODE
	targets[num_targets++] = format_utf8_string_atom;
	targets[num_targets++] = format_unicode_atom;
#endif
	targets[num_targets++] = format_string_atom;
	targets[num_targets++] = XA_STRING;
	
	// create a hidden unlinked window to own the clip contents
	//
	_zg_clipwindow = XCreateSimpleWindow(_zg_display, DefaultRootWindow(_zg_display),
		0, 0, 100, 100, 0, 0, 0);
	#if 0
	if(_zg_clipwindow)
	{
		XMapWindow(_zg_display, _zg_clipwindow);
	}
	#endif
}

//**************************************************************************
void _w_clipDeinit(void)
{
	// if we own the selection, disown it
	//
	if(XGetSelectionOwner(_zg_display, primary_atom) == DefaultRootWindow(_zg_display))
		XSetSelectionOwner(_zg_display, primary_atom, None, acquire_time);
	if(XGetSelectionOwner(_zg_display, clipboard_atom) == DefaultRootWindow(_zg_display))
		XSetSelectionOwner(_zg_display, clipboard_atom, None, acquire_time);
	if(_zg_clipdata)
	{
		free(_zg_clipdata);
		_zg_clipdata = NULL;
	}
	xclip_notify_change();
	
	if(_zg_clipwindow)
	{
		XDestroyWindow(_zg_display, _zg_clipwindow);		
		_zg_clipwindow = 0;
	}
}
#endif

#endif // Windows

