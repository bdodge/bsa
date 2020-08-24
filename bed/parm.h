#ifndef _PARM_H_
#define _PARM_H_ 1

// types of buffers

enum BufType
{
	btAny,
	btRaw,
	btText,
	btC,
	btCPP,
	btCS,
	btJava,
	btJavaScript,
	btOBJC,
	btPHP,
	btPython,
	btTCL,
	btVerilog,
	btASM,
	btPerl,
	btHTML,
	btXML,
	btShell,
	btSwift,
	btTelnet,
	btTerm
};

// types of things to ask user for

enum ParmType {
				ptNone,

				ptString,
				ptOpenFilename,
				ptOpenExistingFilename,
				ptSaveFilename,
				ptDirectory,
				ptFontSpec,

				ptNumber,
				ptColor,
				ptTextEncoding,
				ptTextLineTerm,
				ptBufferType,
				ptKeyBindings,
				ptBinFormat
			};

class Bview;

typedef struct tagParmPass
{
	Bview*		pView;
	LPCTSTR		lpTitle;
	LPCTSTR		lpPrompt;
	LPCTSTR		lpInitVal;
	LPTSTR		lpString;
	int			nString;
	ParmType	type;
}
PARMPASS, *LPPARMPASS;


// dialogs for open file
//**************************************************************************

int PickFileDialog(
				   LPCTSTR	lpTitle,
				   bool		openExisting,
				   bool		openorsave,
				   LPTSTR	lpName,
				   int		nName,
				   TEXTENCODING&	encoding,
				   LINETERMINATION& lineterm,
				   BufType&			bufType,
				   HWND hwndParent
				  );

extern int PickDirectoryDialog(LPPARMPASS pParm, HWND hwndParent);

// dialog for font pick
//**************************************************************************
int PickFontDialog(LPPARMPASS pParm, HWND hwndParent);

// dialog of color pick
//**************************************************************************
int PickColorDialog(LPPARMPASS pParm, HWND hwndParent);

#endif


