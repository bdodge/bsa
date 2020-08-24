#ifndef SYMTAB_H_
#define SYMTAB_H_ 1

typedef struct tag_symtype
{
	unsigned int	bsize;			// byte size if atomic type. or array size
	unsigned int	isptr;			// pointer level (0,*,**,***,etc.)
	unsigned int	isdim;			// dimension level (0,1,...)
	unsigned int	isaggrt : 2;	// is object (not atomic)
	unsigned int	isauto  : 1;	// is auto (local)
	unsigned int	iscode  : 1;	// is allocated on stack already
	unsigned int	isconst : 1;	// is constant
	unsigned int	iscopy  : 1;	// is a reference to an original symbol
	unsigned int	isdef	: 1;	// has a body/definition/been stored to
	unsigned int	isenum	: 1;	// is an enumerated type
	unsigned int	isext	: 1;	// is external
	unsigned int	isreal  : 2;	// is a real (float/double)
	unsigned int	isfunc  : 1;	// is a function
	unsigned int	isinlin : 1;	// is inline
	unsigned int	islit   : 1;	// is literal
	unsigned int	islong  : 2;	// is long
	unsigned int	ispack	: 1;	// is packed struct
	unsigned int	isref	: 1;	// has been loaded (used)
	unsigned int	isreg   : 1;	// is register
	unsigned int	istatic : 1;	// is static
	unsigned int	isuns   : 1;	// is unsigned
	unsigned int	isvoid  : 1;	// is static
	unsigned int	isvol   : 1;	// is volatile
}
SYMTYPE, *PSYMTYPE;

#define AGGRT_STRUCT	0x1
#define AGGRT_UNION		0x2
#define AGGRT_CLASS		0x3

typedef struct tag_sym
{
	TCHAR*			name;
	
	// type description
	SYMTYPE			desc;

	// member object chain
	struct tag_sym*	members;

	// type chain
	struct tag_sym* type;

	// initializer chain
	struct tag_sym* init;
	
	// location offset (on stack)
	long			offset;

	// linkup chain
	struct tag_sym* next;
}
SYM, *PSYM;

typedef struct tag_symref
{
	TCHAR*			name;
	
	// type description
	SYMTYPE			desc;

	// type chain
	struct tag_sym* type;

	// actual symbol 
	PSYM			psym;
	
	// alloc chain
	struct tag_sym* next;
}
SYMREF, *PSYMREF;

typedef struct tag_symlist
{
	PSYM*				symbols;
	PSYM*				types;
	struct tag_symlist* prev;
	struct tag_symlist* next;
}
SYMTAB, *PSYMTAB;

typedef struct tag_symstack
{
	PSYM				 psym;
	struct tag_symstack* prev;
}
SYMSTACK, *PSYMSTACK;

typedef int (*SYMWALKFUNC)(PSYM psym, void* cookie);

//***********************************************************************
// symtab
void		DescInit(PSYMTYPE pdesc);

PSYM		SYMcreate			(const TCHAR* name);
PSYM		SYMcreateCopy		(PSYM pbase);
PSYM		SYMrename			(PSYM psym, const TCHAR* newname);
void		SYMdestroy			(PSYM psym);
void		SYMaddMember		(PSYM psym, PSYM pmember);
PSYM		SYMfindMember		(TCHAR* name, PSYM psym);
void		SYMsetType			(PSYM psym, PSYM ptype, PSYMTYPE pdesc);
PSYM		SYMbaseType			(PSYM psym);

PSYMREF		SYMREFcreate		(PSYM pbase);
PSYMREF		SYMREFcreateCopy	(PSYMREF pbase);
void		SYMREFdestroy		(PSYMREF psymr);
PSYMREF		SYMREFrename		(PSYMREF psymr, const TCHAR* newname);
unsigned long SYMREFgetSize		(PSYMREF psymr);
unsigned long SYMREFgetSizeBytes(PSYMREF psymr);

PSYM		SYMTABaddSym		(PSYM psym, PSYM* table);
PSYMTAB		SYMTABcreate		(void);
void		SYMTABdestroySym	(PSYM* table);
void		SYMTABdestroy		(PSYMTAB ptab);
int			SYMTABtypeString	(PSYM sym, TCHAR* buf, int nbuf);
unsigned long SYMgetSize		(PSYM psym);
unsigned long SYMgetSizeBytes	(PSYM psym);
void		SYMTABdumpSym		(PSYM* table, int level);
void		SYMTABdump			(PSYMTAB ptab, int level);
void 		SYMTABmove			(PSYMTAB pdst, PSYMTAB psrc);
PSYM		SYMTABfindSym		(const TCHAR* name, PSYM* table);
void		SYMTABremoveSym		(PSYM psym, PSYM* table);
PSYM		SYMTABfindVar		(const TCHAR* name, PSYMTAB ptab);
PSYM		SYMTABfindType		(const TCHAR* name, PSYMTAB ptab);
int			SYMTABwalk			(PSYM* table, SYMWALKFUNC pfunc, void* cookie);

#endif
