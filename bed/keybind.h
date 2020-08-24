#ifndef _KEYBINDH_INCL
#define _KEYBINDH_INCL

//**************************************************************************
//
// Available Commamds
//
typedef enum
{
	UnknownCommand = 0x41,	/* A */

	/*
	 * Cursor Movement
	 */
	MoveToCol,
	MoveToLine,

	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,

	MoveWordLeft,
	MoveWordRight,

	MovePageLeft,
	MovePageRight,
	MovePageUp,
	MovePageDown,

	MoveToBOL,
	MoveToEOL,

	MoveToTOB,
	MoveToEOB,

	/*
	 * Locating
	 */
	FindForward,
	FindReverse,
	FindNonZero,
	FindAgain,
	Replace,
	SetSearch,
	SetReplace,
	ReplaceFoundText,
	SearchAndReplace,
	SetSearchDirection,
	SetSearchMode,
	FindFile,
	FindInFile,
	ReplaceInFile,
	FindDefinitionOf,
	FindReferencesOf,

	/*
	 * Item insertion/deletion
	 */
	SelfInsert,
	Insert,
	Paste,
	Delete,
	DeleteBack,
	DeleteEOL,
	DeleteEolOrLineOrBlock,
	DeleteBlock,
	Cut,
	Copy,
	Type,

	/*
	 * Marking
	 */
	SetMark,
	ClearMark,
	SwapMark,
	ToggleMark,
	SelectChar,
	SelectWord,
	SelectLine,
	SelectAll,

	/*
	 * Buffer
	 */
	NewBuffer,
	OpenBuffer,
	ReadBuffer,
	NextBuffer,
	PreviousBuffer,
	SaveBuffer,
	SaveAs,
	SaveAll,
	KillBuffer,
	EmptyBuffer,
	ListBuffers,
	MergeBuffers,
	MakeWritable,

	/*
	 * Window
	 */
	SplitVertical,
	SplitHorizontal,
	SplitWindow,
	NextWindow,
	PreviousWindow,
	RefreshWindow,
	RecenterWindow,
	KillWindow,
	KillAllWindows,
	SetBinRadix,
	SetBinFormat,
	SetBinColumns,

	/*
	 * Undo
	 */
	Undo,
	Redo,

	/*
	 * Macros
	 */
	DefineMacro,
	PlayMacro,
	KillMacro,
	SaveMacro,
	RecallMacro,
	
	PushString,
	PushNumber,
	PopString,
	PopNumber,

	/*
	 * Misc
	 */
	OverstrikeOn,
	OverstrikeOff,
	ToggleOverstrike,
	SetTabSpace,
	SetTab,
	ClearTab,
	Tabify,
	Untab,
	TabsAsSpaces,
	ShowLineNums,
	HideLineNums,
	ShowWhitespace,
	HideWhitespace,
	TrimWhitespace,
	ToggleCarriageReturns,
	CaseOn,
	CaseOff,
	ToggleCase,
	ToggleTrimOnWrite,
	EnableRefresh,
	DisableRefresh,
	SetDirty,
	ClearDirty,
	ViewHex,
	
	BindKey,
	BindDefaultKey,
	SetKeyBinds,
	ShowKeyBinds,
	BedBind,
	EmacsBind,
	MSVCBind,
	CWBind,
	
	DoMenu,
	Help,
	HelpAbout,
	ShowCommands,
	ShowKeyBindings,

	/*
	 * Commands 
	 */
	DoCommand,
	ReadCommandFile,

	/*
	 * Tags
	 */
	SetTagFile,
	FindTag,

	/*
	 * Projects
	 */
	 NewProject,
	 OpenProject,
	 CloseProject,

	/*
	 * Programs
	 */
	Indent,
	IndentLine,
	IndentMore,
	IndentLess,
	IndentLevel,
	IndentSpace,
	Dismember,
	Match,
	Build,
	Execute,
	Target,
	CancelBuild,
	NextError,
	CheckIn,
	CheckOut,
	Revert,

	Debug,
	StopDebug,
	Start,
	Stop,
	StepIn,
	StepOut,
	Step,
	SetBreak,
	ClearBreak,
	ToggleBreak,
	RunToCursor,
	ReadVar,
	WriteVar,
	SetWatchVar,
	ClearWatchVar,

	/*
	 * Program
	 */
	ShellFinished, /* this is really for internal, use */
	ExecShell,
	ExitBed,

	/*
	 * Inquire
	 */
	LineNum,
	ColumnNum,
	Found,
	BufferName,
	BufferType,
	BufferTypeAsString,
	IsModified,
	IsUntitled,
	IsNew,
	IsEmpty,
	LineLen,
	LineContent,
	CurrentChar,
	CurrentCharAsString,
	SelectionStartLine,
	SelectionStartColumn,
	SelectionEndLine,
	SelectionEndColumn,

	/*
	 * Dialog
	 */
	InquireNumber,
	InquireString,
	InquireFilename,
	InquireYesNo,
	InquireYesNoCancel,
	InquireYesNoSkipAll,
	Message,
	SetStatus,

	/*
	 * Line type
     */
	SetLineType,
	
	/*
	 * Misc
	 */
	SetFrgColor,
	SetBkgColor,
	SetFont,
	SetDefaultFonts,
	
	PageSetup,
	PageRange,
	SetPrinterName,
	PrintScreen,
	PrintSelection,
	PrintDocument,
	AquireScreenImage,
	
	AbortKeySequence, /* internal */

	MaxEditCommand

} EditCommand;


//**************************************************************************
//
// Command and name, with help
//
typedef struct tag_cmd_names
{
	EditCommand cmd;
	LPCTSTR		name;
#ifdef CONFIG_HELP
	DWORD		cmdType;
	LPCTSTR		description;
	LPCTSTR		syntax;
	LPCTSTR		helpinfo;
#endif
} BcmdName;

typedef BcmdName *LPBCMDNAME;

#define MAX_KEYSEQUENCE 4

//**************************************************************************
//
// Array of keys in sequence which is bound to a command
// the upper byte of the word is used to hold a key state
// mask (ctrl, alt, shift), the lower byte, a key code
// native to this application
//
#define SHIFT_MASK	0x4000
#define ALT_MASK	0x2000
#define CTRL_MASK	0x1000

typedef struct
{
	WORD		seq[MAX_KEYSEQUENCE];
	EditCommand code;
}
BkeyBindItem;

typedef BkeyBindItem FAR *LPBKEYBINDITEM;


/*
 * Command Types
 */
#define cmdMOVE		0x00000001
#define cmdTEXT		0x00000002
#define cmdMARK		0x00000004
#define cmdWIND		0x00000008
#define cmdBUFF		0x00000010
#define cmdLOOK		0x00000020
#define cmdMACR		0x00000040
#define cmdFIND		0x00000080
#define cmdVIEW		0x00000100
#define cmdIMAGE	0x00000200
#define cmdHELP		0x00000400
#define cmdMISC		0x00000800
#define cmdPRGM		0x00001000
#define cmdKEYS		0x00002000
#define cmdCMDS		0x00004000
#define cmdSTCK		0x00008000
#define cmdINQU		0x00010000
#define cmdTCLX		0x00020000
#define cmdRESERVED	0x10000000

#define cmdMaxCmdTypeIndex 18

extern BcmdName		commandNames[];

extern BkeyBindItem nativeBindings[];
extern BkeyBindItem emacsBindings[];
extern BkeyBindItem msvcBindings[];
extern BkeyBindItem codewarriorBindings[];

extern WORD BkeyTranslate[];


// These are names for common keys, to avoid having
// to use 'A' type notation, and some are easier
// to see as "keyCtrlA" instead of "\01"
//

// Virtual keys are mapped into "extended" keys through
// a mapping table, so we can distinguish between
// "VK_DELETE" (= 0x2E) and "." (also = 0x2E)
//

typedef enum
 {
	/*
	 * ASCII chars
	 */
	keyNULL = 0,				/* 0x0 */

	keyCtrlA = 1,				/* 0x01 - 0x1A */
	keyCtrlB, keyCtrlC, keyCtrlD, keyCtrlE,
	keyCtrlF, keyCtrlG,	keyCtrlH, keyCtrlI,
	keyCtrlJ, keyCtrlK, keyCtrlL, keyCtrlM,
	keyCtrlN, keyCtrlO, keyCtrlP, keyCtrlQ,
	keyCtrlR, keyCtrlS, keyCtrlT, keyCtrlU,
	keyCtrlV, keyCtrlW, keyCtrlX, keyCtrlY,
	keyCtrlZ,

	keyEscape = 0x1B,			/* 0x1B */

	keyFS = 0x1C,
	keyGS,
	keyRS,
	keyUS,

	keySpace = 0x20,			/* 0x20 */
	keyBang,
	keyDoubleQuote,
	keyPound,
	keyDollar,
	keyPercent,
	keyAmpersand,
	keyRightQuote,
	keyOpenParen,
	keyCloseParen,
	keyStar,
	keyPlus,
	keyComma,
	keyMinus,
	keyDot,
	keySlash,

	key0,						/* 0x30 */
	key1,
	key2,
	key3,
	key4,
	key5,
	key6,
	key7,
	key8,
	key9,

	keyColon,
	keySemiColon,
	keyLessThan,
	keyEquals,
	keyMoreThan,
	keyHook,
	keyAt,

	keyA, keyB, keyC, keyD, keyE, keyF,	keyG,
	keyH, keyI, keyJ, keyK, keyL, keyM,	keyN,
	keyO, keyP, keyQ, keyR, keyS, keyT, keyU,
	keyV, keyW, keyX, keyY, keyZ,

	keyOpenBracket,
	keyBackSlash,
	keyCloseBracket,
	keyHat,
	keyUnderscore,
	keyLeftQuote,

	keya, keyb, keyc, keyd, keye, keyf, keyg,
	keyh, keyi, keyj, keyk, keyl, keym, keyn,
	keyo, keyp, keyq, keyr, keys, keyt, keyu,
	keyv, keyw, keyx, keyy, keyz,

	keyOpenBrace,
	keyVertBar,
	keyCloseBrace,
	keyTilde,
}
BregularKeys;

/*** End of 7 Bit ASCII codes, Begin Meta (virtual key) codes ***/

typedef enum
{
	keyDelete = 0x7F,			/* 0x7F */
	/*
	 * Cursor Keys
	 */
	keyCursorLeft,				/* 0x80 */
	keyCursorRight,
	keyCursorUp,
	keyCursorDown,
	keyCursorHome,
	keyCursorPrev,
	keyCursorNext,
	keyCursorEnd,
	keyCursorPageUp,
	keyCursorUpLeft,
	keyCursorDownLeft,
	keyCursorPageDown,			/* 0x8C */

	keySelect,					/* 0x8D */
	keyPrint,
	keyExec,
	keyInsert,
	keyUndo,
	keyRedo,
	keyMenu,
	keySearch,
	keyStop,
	keyHelp,
	keyBreak,
	keyModeSwitch,
	keyNumLock,					/* 0x99 */
	
	keyShift,
	keyCtrl,
	keyAlt,
	keyCapsLock,
	keyPause,					/* 0x9E */
	/*
	 * Functions Keys
	 */
	keyF1,  keyF2,  keyF3,  keyF4,  keyF5,
	keyF6,  keyF7,  keyF8,  keyF9,  keyF10,
	keyF11, keyF12, keyF13, keyF14, keyF15,
	keyF16, keyF17,	keyF18, keyF19, keyF20,	
	keyF21, keyF22, keyF23, keyF24,	/* 0xB6 */
	/*
	 * Sequence key identifies start of sequence of keys (ie Esc or 0)
	 */
	keyBeginSequence,			/* 0xB7 */
	keyNOOP,					/* 0xB8 */
	keyIllegal,					/* 0xB9  keep this last */

}
BextendedKeys;

typedef enum { kbNative, kbEmacs, kbMSVC, kbCW } aKeyBind;

//**************************************************************************

class BkeyBinds
{
public:
	BkeyBinds(aKeyBind bind);
	~BkeyBinds();

public:
	ERRCODE			SetBindings(aKeyBind bind);
	aKeyBind		GetBindings(void)		{ return m_bind; };
	BkeyBindItem*	GetKeySequences(void)	{ return m_keySequences; };
	EditCommand		Translate(WORD key);
	EditCommand		CommandFromName(LPCTSTR pName);
	void			Reset(WORD key) { m_keyState = 0; };

protected:
	int				KeySequenceCompare		(BkeyBindItem* pSeq, LPWORD pSoFar, int nSoFar);
	EditCommand		MatchSequence			(WORD key);

protected:
	int				m_keyState;
	aKeyBind		m_bind;
	BkeyBindItem*	m_keySequences;
	WORD			m_keyseq[MAX_KEYSEQUENCE+1];
};

//**************************************************************************
int PickKeyBindingDialog(LPPARMPASS pParm, HWND hwndParent);


#endif /* KEYBINDH_INCL */

