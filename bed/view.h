#ifndef _VIEW_H_
#define _VIEW_H_ 1

#define MAX_SEARCH_STR  1024
#define MAX_PROTO_ZOOM  64

#define MAX_TAB_SPACE   128

#define FORCE_LINE_MATCH 1 // use a line to match braces

typedef struct tag_font_init
{
    TCHAR   face[64];
    int     height;
    bool    bold;
    bool    italic;
    bool    antialias;
}
FONTSPEC, FAR *LPFONTSPEC;

class Btcl;

enum BkwType
{
    kwPlain,
    kwQuoted,
    kwComment,
    kwBuiltin,
    kwBuiltinType,
    kwBuiltinFunc,
    kwAddonType,
    kwAddonFunc,
    kwMacro,
    kwOperator,
    kwSpecial,
	/* background color only after here (i.e. not assoc. font) */
    kwSelected,
    kwHighlight,
	kwLinoBackground,
    kwBackground
};

// these must match number of BlwType
#define MAX_VIEW_FONTS  (kwSpecial - kwPlain + 1)
#define MAX_VIEW_COLORS (kwBackground - kwPlain + 1)

enum TokenState
{
    tsBase,
    tsComment,
    tsQuotedString,
    tsQuotedLiteral,
    tsMacro,
    tsSpanningComment,
    tsSpanningElement,
    tsPostText,
    tsSpecial,
};

enum DrawType { dtDraw, dtPrint, dtSetCaret, dtSetPosToMouse };

// binary tree keyword object
//**************************************************************************
typedef struct tagKeyword
{
public:
    tagKeyword                          (LPCTSTR lpWord, BkwType type, bool casesensi);
    ~tagKeyword                         ();

public:
    struct tagKeyword*  AddKeyword      (struct tagKeyword* pKey, struct tagKeyword*& pList);
    struct tagKeyword*  FindKeyword     (LPCTSTR lpWord, int nWord, struct tagKeyword* pList);

public:
    BkwType             m_type;

protected:
    LPTSTR              m_word;
    bool                m_sensi;
    struct tagKeyword*  m_left;
    struct tagKeyword*  m_right;
}
KEYWORD, *LPKEYWORD;

// prefix object list
//**************************************************************************
typedef BkwType* LPTOKENPREFIX;

KEY_ARRYVAL_LIST_TEMPLATE(BprefixList, LPTOKENPREFIX, NO_API);

// comment start/ends object list
//**************************************************************************
typedef TokenState* LPTOKENSTATE;

KEY_ARRYVAL_LIST_TEMPLATE(BcommentList, LPTOKENSTATE, NO_API);

// parm stack
//**************************************************************************
typedef struct tagParm
{
public:
    tagParm(LPCTSTR pParm, int nParm, ParmType parmType);
    tagParm(int parm, ParmType parmType);
    ~tagParm();

public:
    LPTSTR              m_pParm;
    int                 m_nParm;
    ParmType            m_type;
    tagParm*            m_next;
}
PARM, *LPPARM;

// macro record
//**************************************************************************
typedef struct tagMacroEntry
{
public:
    tagMacroEntry(EditCommand command, LPCTSTR lpData, int nData, ParmType partType);
    tagMacroEntry(EditCommand command, int nParm, ParmType partType);
    tagMacroEntry(EditCommand command);
    ~tagMacroEntry();

public:
    LPTSTR              m_pParm;
    int                 m_nParm;
    ParmType            m_type;
    EditCommand         m_command;
    tagMacroEntry*      m_next;
}
MACROREC, *LPMACROREC;

#define ID_VIEW_EVENT       0xface
#define ID_PANE_SELCHANGED  0xbeef

enum TokenRet { trOK, trEOLTOK, trEOLLINE, trFAILURE };

// view object
//**************************************************************************
class Bview : public BappPane
{
public:
                    Bview           (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
    virtual         ~Bview          ();

public:
    virtual void    AddTags         (void)  { return; }
    virtual void    AddTags         (LPCTSTR* ptags, BkwType type, bool casesensi);
    virtual LPCTSTR GetTypeName     (void)  { return _T("Text");    }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.*");  }
    virtual HWND    GetEditWindow   (void)  { return GetWindow();   }

    // BappPane
    //
    virtual void    Activate        (void);
    virtual void    Deactivate      (void);

    // command execution
    //
    virtual ERRCODE Dispatch        (EditCommand command);
    virtual ERRCODE CheckReadOnly   ();
    virtual ERRCODE CheckKillBlock  (void);
    virtual ERRCODE ClearMouseSelection(void);
    virtual ERRCODE SetOpRegionFromSelection(void);
    virtual ERRCODE ClearOpRegion   (void);
    virtual ERRCODE ClearSearchHighlight(void);
    virtual bool    InRegion        (int line, int col);
    virtual bool    RegionSet       (void)      { return m_regional; }
    virtual void    ResetFoundLocation(void)    { m_locl = m_locc = 0; }
    virtual int     CountRegionChars(Bbuffer* pBuf, int mla, int mca, int mlb, int mcb);
    virtual ERRCODE Key             (WORD key);
    virtual EditCommand CommandFromName(LPCTSTR pName);
    virtual ERRCODE ResetKey        (WORD key);
    virtual ERRCODE PushParm        (LPCTSTR pParm, int nParm, ParmType type);
    virtual ERRCODE PushParm        (int parm, ParmType type);
    virtual ERRCODE PopParm         (LPCTSTR pPrompt, ParmType type, LPCTSTR& pParm, int& nParm, LPCTSTR pInitial = NULL);
    virtual ERRCODE PopParm         (LPCTSTR pPrompt, ParmType type, int& parm);
    virtual ERRCODE FreeOldParms    (void);
    virtual ERRCODE SearchAndReplace(void);
    virtual ERRCODE MoveAbs         (int line, int col);
    virtual ERRCODE MoveRel         (int lines, int cols);
    virtual bool    GetMarkedRegion (int& mla, int& mca, int& mlb, int& mcb);
    virtual void    SetMarkedRegion (int mla, int mca, int mlb, int mcb);
    virtual void    ClearMark       (void);
    virtual BOOL    CheckModified   (bool cangoon);
    virtual BOOL    CheckExternallyModified(void);
    virtual BOOL    Close           (void);

    // macro
    //
    virtual ERRCODE AddToMacro      (LPMACROREC pRecord);
    virtual ERRCODE PlayMacro       (LPMACROREC pMacro);
    static  ERRCODE KillMacro       (LPMACROREC pRec);

    // token functions
    //
    virtual int      DisplayColumn  (HDC hdc, int line, int col, bool inout, int& x, int& y);
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        BkwType&    kw
                                    );
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        HFONT&      hFont,
                                        COLORREF&   frgColor,
                                        COLORREF&   bkgColor
                                    );
    virtual ERRCODE FindMatchingPhrase(
                                        LPCTSTR phrase,
                                        LPCTSTR match,
                                        int&    line,
                                        int&    col
                                    );
    virtual ERRCODE GetMatchingPhrase(
                                        LPCTSTR lpPhrase,
                                        LPTSTR  lpMatch,
                                        int     nMatch
                                    );
    virtual ERRCODE GetFirstNonBlankColumn
                                    (
                                        int     line,
                                        int&    col,
                                        bool    incComments,
                                        bool    incMacros,
                                        TCHAR&  lastdelim,
                                        TCHAR&  firstdelim,
                                        BkwType& firstdelimkw,
                                        int&    firstdelimcol,
                                        int&    parens,
                                        int&    braces,
                                        int&    colons
                                    );
	virtual ERRCODE GetIsLabel(
										int		line,
										bool	&label
									);
    virtual ERRCODE GetStatement
                                    (
                                        int         line,
                                        int&        col,
                                        bool        last,
                                        BkwType&    kw,
                                        LPTSTR      pToken,
                                        int&        cbToken
                                    );
    virtual ERRCODE GetFirstStatementToken(
                                        int&        line,
                                        int&        col,
                                        LPCTSTR     pVarname,
                                        LPTSTR      pToken,
                                        int&        nToken
                                    );
    virtual bool    IsDelim         (TCHAR ic);
    static  bool    NonWhite(LPCTSTR lpToken, int nToken);
    static  int     KeywordCompare  (LPCTSTR lpA, LPCTSTR lpB, int nA, int nB, bool casesensi);
    virtual BkwType KeyWord         (LPCTSTR pWord, int nWord);
    virtual void    AddKeyword      (LPCTSTR pWord, BkwType type, bool casesensi);
    virtual void    AddPrefix       (LPCTSTR pPref, BkwType type);
    virtual void    AddComment      (LPCTSTR pText, TokenState state);
    virtual bool    StartsComment   (LPCTSTR lpText, int nText, int& nComment, TokenState& state);
    virtual bool    EndsComment     (LPCTSTR lpText, int nText, int& nComment, TokenState& state);
    virtual ERRCODE Indent          (WORD key, int& line, int& col);
    virtual ERRCODE GetVarname      (
                                        int         line,
                                        int&        col,
                                        LPTSTR      pVarname,
                                        int&        nVarname,
                                        TCHAR&      delim,
                                        TCHAR&      leftdelim,
                                        int&        leftdelcol,
                                        bool&       isFunc
                                    );
    virtual ERRCODE GetTypeOf       (
                                        LPCTSTR     pVarname,
                                        int&        line,
                                        int&        col,
                                        LPTSTR      pTypename,
                                        int&        nTypename,
                                        bool&       isPtr,
                                        Bpdb*       pPDB
                                    );
    virtual ERRCODE GetContainingClass(
                                        int&        line,
                                        int&        col,
                                        LPTSTR      pClassname,
                                        int&        nClassname
                                    );
    virtual ERRCODE GetTypeOfTokenAtPoint(
                                        int&        line,
                                        int&        col,
                                        LPBSYM&     pSym,
                                        Bpdb*       pPDB,
                                        bool&       isFunc
                                    );
    virtual ERRCODE GetDefinitionOfTokenAtPoint(
                                        int&        line,
                                        int&        col,
                                        LPBSYM&     pSym,
                                        Bpdb*       pPDB,
                                        bool&       isFunc
									);
    virtual ERRCODE Dismember       (void);
    virtual ERRCODE Match           (LPCTSTR pKey);
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL) { return errOK; }

    // state
    //
    virtual ERRCODE PersistSettings (void);
    virtual ERRCODE DepersistParms  (void);
    virtual Bbuffer*GetBuffer       (void) { return m_buffer; }
    virtual int     GetCurLine      (void) { return m_curline; }
    virtual int     GetCurCol       (void) { return m_curcol;  }
    virtual bool    GetOverstrike   (void) { return m_overstrike;}
    virtual bool    GetCaseSensi    (void) { return m_casesensi; }
    virtual bool    GetTrimOnWrite  (void) { return m_trimonwrite; }
    virtual int     GetIndentSpace  (void) { return m_indentspace; }
    virtual int     GetTabSpace     (void) { return m_tabspace; }
    virtual bool    GetTabsAsSpaces (void) { return m_tabsasspaces; }
    virtual bool    GetAutoTabDetect(void) { return m_autotabs; }
    virtual bool    GetHandMarked   (void) { return m_handmarked; }
    virtual int     GetTopLine      (void) { return m_topline;   }
    virtual int     GetLeftCol      (void) { return m_leftcol;   }
    virtual int     GetViewRadix    (void) { return m_binradix;  }
    virtual int     GetViewFormat   (void) { return m_binformat;  }
    virtual void    SetBuffer       (Bbuffer* pBuf) { m_buffer = pBuf; }
    virtual void    SetOverstrike   (bool o) { m_overstrike = o; UpdateStatus(); }
    virtual void    SetCaseSensi    (bool s) { m_casesensi = s;  UpdateStatus(); }
    virtual void    SetTrimOnWrite  (bool s) { m_trimonwrite = s; }
    virtual void    SetTabsAsSpaces (bool s) { m_tabsasspaces = s; UpdateStatus(); }
    virtual void    SetAutoTabDetect(bool s) { m_autotabs = s; UpdateStatus(); }
    virtual void    SetHandMarked   (bool q) { m_handmarked = q; UpdateStatus(); }
    virtual ERRCODE SetViewRadix    (int x)  { m_binradix = x; UpdateStatus(); return errOK; }
    virtual ERRCODE SetViewFormat   (int x)  { m_binformat = x; UpdateStatus(); return errOK; }
    virtual void    SetLastModifiedCol  (int col);
    virtual int     GetLastModifiedCol  (void);
    virtual LPCTSTR GetSearch       (int& l)    { l = m_searchlen; return m_searchstr;  }
    virtual LPCTSTR GetReplacement  (int& l)    { l = m_replacelen; return m_replacestr; }
    virtual bool    OnFoundText     (void)      { return m_locl > 0 && m_locc > 0 && m_searchlen; }
    virtual BkeyBinds* GetKeyBinds  (void)      { return m_keyBindings; }
    virtual Bed*    GetEditor       (void)      { return m_editor; }

    // redraw
    //
    virtual ERRCODE SetFocus        (bool inout);
    virtual void    SetNext         (Bview* pView);
    virtual Bview*  GetNext         (Bview* pView)  { return m_next;   }
    virtual bool    IsVisible       (void)          { return GetActive();}
    virtual void    Draw            (HDC hdc, HWND hWnd, LPRECT lprcUpdate, DrawType type);
    virtual ERRCODE PrintView       (int startline, int endline, BprintRange range);
    virtual void    UpdateView      (int line1, int col1, int line2, int col2);
    virtual void    UpdateAllViews  (int line1, int col1, int line2, int col2, Bbuffer* pBuf);
    virtual void    UpdatePos       (int line, int col);
	virtual int		GetLeftMargin	(void);
    virtual int     GetCaretWidth   (void)      { return m_caretWidth; }
    virtual int     GetCaretHeight  (void)      { return GetFontHeight(NULL); /*m_caretHeight;*/}
    virtual void    ClearLineInfo   (BlineInfo info);
    virtual void    Event           (LPARAM lParam) { };

    // scrolling and mouse
    //
    virtual void    GetViewClientRect(HWND hWnd, LPRECT rc);
    virtual int     GetViewLines    (HDC hdc = NULL);
    virtual int     GetViewCols     (HDC hdc = NULL);
    virtual int     GetFontHeight   (HDC hdc = NULL);
    virtual int     GetFontWidth    (HDC hdc = NULL);
    virtual void    ClearMetricsCache(void);
    virtual int     GetWheelIncrement(void){ return m_wheelinc;  }
    virtual void    RecenterView    (void);
    virtual void    ScrollView      (int which, int where);
    virtual void    MouseDown       (int x, int y);
    virtual void    MouseUp         (int x, int y);
    virtual void    MouseMove       (int x, int y);
    virtual void    MouseMenu       (int x, int y);
    virtual void    MouseSpecial    (int x, int y);
    virtual void    SpecificMouseMenu(HMENU hMenu) { return; }
    virtual void    SetScrollers    (int which);

    // misc
    //
    static ERRCODE  ParseFontSpec   (LPCTSTR pSpec, LPTSTR pFace, int nFace, int& height, bool& bold, bool& italic, bool& antialias);
    static ERRCODE  ParseColorSpec  (LPCTSTR pSpec, int& r, int& g, int& b, int& a);
    virtual ERRCODE SetKeywordFont  (BkwType kw, LPCTSTR pFontSpec);
    virtual LPCTSTR GetKeywordFont  (BkwType kw, LPTSTR pszSpec, int nSpec);
    virtual ERRCODE SetKeywordColor (BkwType kw, LPCTSTR pColorSpec);
    virtual LPCTSTR GetKeywordColor (BkwType kw, LPTSTR pszSpec, int nSpec);
    virtual COLORREF GetBackground  (BYTE& alpha);
    virtual HWND    GetFuncsCache   (void) { return m_hWndCacheFuncs;   }
    virtual HWND    GetFilesCache   (void) { return m_hWndCacheFiles;   }
    virtual void    SetFuncsCache   (HWND hWnd) { m_hWndCacheFuncs = hWnd; }
    virtual void    SetFilesCache   (HWND hWnd) { m_hWndCacheFiles = hWnd; }
    virtual void    SetShowTabs     (bool s)    { m_showtabs = s; InvalidateRect(m_hwnd, NULL, FALSE); }
    virtual bool    GetShowTabs     (void)      { return m_showtabs; }
    virtual void    SetShowComments (bool s)    { m_showCommentInfo = s; InvalidateRect(m_hwnd, NULL, FALSE); }
    virtual bool    GetShowComments (bool s)    { return m_showCommentInfo; }
    virtual void    SetShowLineNums (bool s)    { m_showLineNums = s; InvalidateRect(m_hwnd, NULL, FALSE); }
    virtual bool    GetShowLineNums (void)      { return m_showLineNums; }
    virtual bool    GetLineInfoChanged(void)    { return m_lineinfoChanged; }
    virtual void    SetLineInfoChanged(bool c)  { m_lineinfoChanged = c; }
    ERRCODE         TabifyText(LPCTSTR pSrc, int nSrc, LPTSTR& pDst, int& nDst);
    ERRCODE         UntabText(LPCTSTR pSrc, int nSrc, LPTSTR& pDst, int& nDst);
    ERRCODE         WhiteTrimText(LPCTSTR pSrc, int nSrc, LPTSTR& pDst, int& nDst);

    virtual void    CheckCloseProtoWindow(void);
    virtual void    SelectProtoItem (void);
    virtual void    ZoomProtoItem   (int key);
    virtual ERRCODE ProtoWindow     (LPBSYM pSym, EnumFuncsType type);

    virtual int     GetClosed       (void) { return m_closed; }
    virtual void    SetClosed       (int c) { m_closed = c; }

    // help
    //
    virtual ERRCODE HelpCommands    (void);
    virtual ERRCODE HelpKeyBinds    (void);

    static  int     UnescapeString  (LPTSTR pRes, LPCTSTR pSrc, bool isPath);
    static  int     EscapeString    (LPTSTR pRes, LPCTSTR pSrc);

protected:
    static void     InitializeViews (Bpersist* pPersist, HWND hwndParent);
    static void     DeInitializeViews(void);
    virtual void    UpdateStatus(void);
    virtual ERRCODE SetupCommentInfo(int firstline, int lastline = -1);

protected:
    int             m_closed;
    Bbuffer*        m_buffer;
    Bed*            m_editor;
    Bview*          m_next;
    Btcl*           m_tcl;
    HWND            m_hWndProto;
    HWND            m_hWndCacheFuncs;
    HWND            m_hWndCacheFiles;
    HIMAGELIST      m_protimgs;
    int             m_protzoomx;
    TCHAR           m_protzoomb[MAX_PROTO_ZOOM];
    LPPARM          m_parmstack;
    LPPARM          m_oldparms;
    bool            m_first_token;
    BkeyBinds*      m_keyBindings;
    LPKEYWORD       m_keywords;
    BcommentList*   m_comments;
    BprefixList*    m_prefixes;
    TCHAR           m_statement_term;
    TCHAR           m_macro_prefix;
    int             m_curline;
    int             m_curcol;
    int             m_startline;
    int             m_startcol;
    bool            m_handmarked;
    bool            m_lineinfoChanged;

    // selection
    int             m_qla, m_qlb;
    int             m_qca, m_qcb;
    // dispatch region
    int             m_ola, m_olb;
    int             m_oca, m_ocb;
    // saved cur pos before regional op
    int             m_orsvl, m_orsvc;

    // found text highlight
    int             m_rla, m_rlb;
    int             m_rca, m_rcb;
    bool            m_regional;
    // location of start of found text
    int             m_locl, m_locc;

    // view top
    int             m_topline;
    int             m_leftcol;
    bool            m_colscrollok;
    int             m_lastmodcol;
    int             m_lastcaretcol;
    int             m_caretWidth;
    int             m_caretHeight;
    bool            m_recursedraw;
    bool            m_autotabs;
    int             m_tabspace;
    int             m_indentspace;
    bool            m_tabsasspaces;
    int             m_wheelinc;
    bool            m_showtabs;
    bool            m_showLineNums;
    bool            m_showCommentInfo;
    int             m_mousex, m_mousey;
    int             m_pmx, m_pmy;
    int             m_mouseclicks;
    bool            m_suppressrefresh;
    int             m_bmla, m_bmca, m_bmlb, m_bmcb;

    int             m_cacheCols;
    int             m_cacheLines;
    int             m_cacheFontHeight;
    int             m_cacheFontWidth;
    RECT            m_cacheRect;

    HFONT           m_view_fonts[MAX_VIEW_FONTS];
    FONTSPEC        m_view_fspec[MAX_VIEW_FONTS];
    COLORREF        m_view_colors[MAX_VIEW_COLORS];

    static LPCTSTR    view_keyword_names[];
    static FONTSPEC   view_init_fonts[];
    static COLORREF   view_init_colors[];
    static bool       m_overstrike;
    static bool       m_casesensi;
    static bool       m_trimonwrite;
    static bool       m_iniautotabs;
    static bool       m_initabsasspaces;
    static int        m_initabspace;
    static int        m_iniindentspace;
    static FONTSPEC   m_def_fonts[MAX_VIEW_FONTS];
    static COLORREF   m_def_colors[MAX_VIEW_COLORS];
    static int        m_binradix;
    static int        m_binformat;
    static bool       m_recording;
    static bool       m_playing;
    static bool       m_suppressmacro;
    static bool       m_runTCL;
    static bool       m_runPerl;
    static LPMACROREC m_macro;
    static LPMACROREC m_macroend;
    static bool       m_searchrev;
    static TCHAR      m_searchstr[MAX_SEARCH_STR];
    static int        m_searchlen;
    static TCHAR      m_replacestr[MAX_SEARCH_STR];
    static int        m_replacelen;
    static TCHAR      m_tabBuf[MAX_TAB_SPACE * 4];
    static Bprinter*  m_printer;
    static long       m_init;
};

#define MAX_TYPEBUFF 512


typedef enum
{
    psBase, psInclude, psInclToken, psTypedef, psInFuncDecl, psInMemberFuncDecl,
    psInFunc, psInMemberFunc, psStructSpec, psClassSpec, psInClass, psPostClass,
    psInDerivation
}
SourceParseState;

//**************************************************************************
struct BefState
{
public:
    BefState(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog);
    ~BefState();

public:
    SourceParseState  m_parseState;
    EnumFuncsCallback m_pCallback;
    Blog*             m_log;
    LPVOID            m_cookie;
    LPTSTR          m_pToken;
    int             m_nTokText;
    TokenState      m_tokState;
    int             m_tokLine;
    int             m_tokInCol;
    int             m_tokOutCol;
    int             m_scope;
    bool            m_ininterface;
    bool            m_typedefPending;
	bool			m_nestedclassok;
};

//**************************************************************************
class BviewC : public Bview
{
public:
    BviewC                          (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
    virtual ~BviewC                 ();

public:
    virtual void    SpecificMouseMenu(HMENU hMenu);
    virtual ERRCODE Key             (WORD key);
    virtual void    AddTags         (void);
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);
    virtual LPCTSTR GetTypeName     (void)  { return _T("C");   }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.c;*.h;*.cpp;*.cc;*.cs");  }
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        BkwType&    kw
                                    );
    virtual ERRCODE Indent          (WORD key, int& line, int& col);

protected:
    virtual ERRCODE NextToken       (BefState* state, LPTSTR pToken, int ccToken, BkwType& kwType);
    virtual ERRCODE ParseStatement  (BefState* state, LPCTSTR pFirst, bool& popscope);
    virtual ERRCODE ParsePreProc    (BefState* state, LPCTSTR pDirective);
    virtual ERRCODE ParseInitializer(BefState* state, int inbrace, LPTSTR pEndChar);
    virtual ERRCODE ParseDimension  (BefState* state);
    virtual ERRCODE ParseVarDecl    (BefState* state, LPCTSTR pType, bool allowFuncDecls, bool& popscope);
    virtual ERRCODE ParseFunctionArgs(BefState* state);
    virtual ERRCODE ParseParentClass(BefState* state);
    virtual ERRCODE ParseAccess     (BefState* state, LPCTSTR pAccess);
    virtual ERRCODE ParseClassDecl  (BefState* state, LPCTSTR pType, bool& popscope);
    virtual ERRCODE ParseFunctionDecl(BefState* state, LPCTSTR pType, int ptrTo, LPCTSTR name);
    virtual ERRCODE ParseScope      (BefState* state, bool allowFuncDecls);

protected:
    bool             m_iscpp;
    static bool      m_haveckeywords;
    static LPKEYWORD m_ckeywords;

    static bool      m_havecppkeywords;
    static LPKEYWORD m_cppkeywords;
    static bool      m_havecskeywords;
    static LPKEYWORD m_cskeywords;
    static bool      m_haveobjckeywords;
    static LPKEYWORD m_objckeywords;

};

#define MAX_HEX_COLS    256

//**************************************************************************
class BviewHex : public Bview
{
public:
    BviewHex                        (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void)  { return; }
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        HFONT&      hFont,
                                        COLORREF&   frgColor,
                                        COLORREF&   bkgColor
                                    );

public:
    virtual void    SpecificMouseMenu(HMENU hMenu);
    virtual int     GetViewCols     (void)  { return m_viewcols;    }
    virtual LPCTSTR GetTypeName     (void)  { return _T("Binary");  }

protected:
    int             m_viewcols;
    int             m_address;
    int             m_nDumpBuf;
    bool            m_dispaddr;
    TCHAR           m_dumpBuf[MAX_HEX_COLS * 3 + 2];
};


//**************************************************************************
class BviewCPP : public BviewC
{
public:
    BviewCPP                        (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("C++"); }

protected:
};

//**************************************************************************
class BviewCS : public BviewC
{
public:
    BviewCS                         (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
	virtual LPCTSTR GetTypeName     (void)  { return _T("C#"); }
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);

protected:
};

//**************************************************************************
class BviewObjectiveC : public BviewC
{
public:
    BviewObjectiveC                 (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("OBJC"); }

protected:
};

//**************************************************************************
class BviewSwift : public BviewC
{
public:
    BviewSwift                      (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
    virtual ~BviewSwift             ();

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("Swift");   }
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);

protected:
};

//**************************************************************************
class BviewF : public BviewC
{
public:
    BviewF                          (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("F");   }

protected:
};

//**************************************************************************
class BviewJava : public BviewCPP
{
public:
    BviewJava                       (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
    virtual ~BviewJava              ();

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("Java");    }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.java;*.js");  }
	virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);

protected:
    static bool      m_havejkeywords;
    static LPKEYWORD m_jkeywords;
};

//**************************************************************************
class BviewPython : public BviewCPP
{
public:
    BviewPython                     (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
    virtual ~BviewPython            ();

public:
    virtual void    AddTags         (void);
    virtual LPCTSTR GetTypeName     (void)  { return _T("Python");  }
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);
    virtual ERRCODE StartsBlock     (int line, int& colons);
    virtual ERRCODE Indent          (WORD key, int& line, int& col);
};

//**************************************************************************
class BviewASM : public Bview
{
public:
    BviewASM                        (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual ERRCODE Key             (WORD key);
    virtual void    AddTags         (void)  { return; }
    virtual bool    IsDelim         (TCHAR ic);
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        BkwType&    kw
                                    );
    virtual ERRCODE HasLabel        (int line, int& colons);
    virtual ERRCODE Indent          (WORD key, int& line, int& col);
public:
    virtual LPCTSTR GetTypeName     (void)  { return _T("ASM"); }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.asm;*.s;*.S");  }
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);

};

//**************************************************************************
class BviewTCL : public Bview
{
public:
    BviewTCL                        (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        BkwType&    kw
                                    );
public:
    virtual LPCTSTR GetTypeName     (void)  { return _T("TCL"); }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.tcl;*.tk");  }
    virtual ERRCODE EnumFunctions   (EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL);
};

//**************************************************************************
class BviewHTML : public Bview
{
public:
    BviewHTML                       (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         (void);
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        HFONT&      hFont,
                                        COLORREF&   frgColor,
                                        COLORREF&   bkgColor
                                    );
public:
    virtual LPCTSTR GetTypeName     (void)  { return _T("HTML");    }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.html;*.htm");  }

public:
    int             m_attrib;
    static  LPCTSTR m_keywords[];

};

//**************************************************************************
class BviewXML : public Bview
{
public:
    BviewXML                        (Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);

public:
    virtual void    AddTags         ()  { return; }
    virtual TokenRet GetToken       (
                                        LPTSTR&     lpText,
                                        int&        nText,
                                        int&        line,
                                        int&        incol,
                                        int&        outcol,
                                        TokenState& state,
                                        LPTSTR&     lpToken,
                                        int&        nToken,
                                        HFONT&      hFont,
                                        COLORREF&   frgColor,
                                        COLORREF&   bkgColor
                                    );
public:
    virtual LPCTSTR GetTypeName     (void)  { return _T("XML"); }
    virtual LPCTSTR GetSearchExtensions(void)  { return _T("*.xml");  }
};

#endif

