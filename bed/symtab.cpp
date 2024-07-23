#include "bccx.h"

Blog* g_log = NULL;

//***********************************************************************
void DescInit(PSYMTYPE pdesc)
{
	pdesc->bsize   = 0;
	pdesc->isptr   = 0;
	pdesc->isdim   = 0;
	
	pdesc->isaggrt = 0;
	pdesc->isauto  = 0;
	pdesc->iscode  = 0;
	pdesc->isconst = 0;
	pdesc->iscopy  = 0;
	pdesc->isdef   = 0;
	pdesc->isenum  = 0;
	pdesc->isext   = 0;
	pdesc->isfunc  = 0;
	pdesc->isinlin = 0;
	pdesc->islit   = 0;
	pdesc->islong  = 0;
	pdesc->ispack  = 0;
	pdesc->isreal  = 0;
	pdesc->isref   = 0;
	pdesc->isreg   = 0;
	pdesc->istatic = 0;
	pdesc->isvoid  = 0;
	pdesc->isvol   = 0;
	pdesc->isuns   = 0;
}

#define SYMHASH_SIZE	16384
#define SYMHASH_MASK	(SYMHASH_SIZE - 1)
#define SYMHASH(v) (((v[1] & 0x7f)<< 7) | (v[0] & 0x7F))

//***********************************************************************
PSYM SYMcreate(const TCHAR* name)
{
	PSYM psym = (PSYM)malloc(sizeof(SYM));
	
	if(! psym)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	psym->name	= (TCHAR*)malloc(_tcslen(name) + 1);
	if(! psym->name)
	{
		free(psym);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	_tcscpy(psym->name, name);

	DescInit(&psym->desc);	
	psym->desc.bsize = 0;
	psym->type		 = NULL;
	psym->members	 = NULL;
	psym->init       = NULL;
	psym->offset	 = 0;
	psym->next 		 = NULL;
	return psym;
}

//***********************************************************************
PSYM SYMcreateCopy(PSYM pbase)
{
	PSYM psym = (PSYM)malloc(sizeof(SYM));
	
	if(! psym)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	psym->name	= (TCHAR*)malloc(_tcslen(pbase->name) + 1);
	if(! psym->name)
	{
		free(psym);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	_tcscpy(psym->name, pbase->name);
	
	// structure deep copy
	psym->desc = pbase->desc;
	psym->desc.iscopy = 1;
	
	psym->type		= pbase->type;
	psym->members	= pbase->members;
	psym->init		= pbase->init;
	psym->offset	= pbase->offset;
	psym->next		= NULL;
	return psym;
}

//***********************************************************************
PSYM SYMrename(PSYM psym, const TCHAR* newname)
{
	if(! psym) return NULL;
	if(! newname) return NULL;
	if(psym->name) free(psym->name);
	psym->name	= (TCHAR*)malloc(_tcslen(newname) + 1);
	if(! psym->name)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	_tcscpy(psym->name, newname);
	return psym;
}

//***********************************************************************
void SYMdestroy(PSYM psym)
{
	if(psym)
	{
		// references to the original sym dont
		// delete the member list
		//
		if(! psym->desc.iscopy)
		{
			SYMdestroy(psym->members);
		}
		if(psym->name)
		{
			free(psym->name);
		}
		free(psym);
	}
}

//***********************************************************************
PSYMREF SYMREFcreate(PSYM pbase)
{

	PSYMREF psymr;


	if(! pbase)
		return NULL;
			
	psymr = (PSYMREF)malloc(sizeof(SYMREF));
	
	if(! psymr)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symref memory\n"));
		return NULL;	
	}
	psymr->name	= (TCHAR*)malloc(_tcslen(pbase->name) + 1);
	if(! psymr->name)
	{
		free(psymr);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symref memory\n"));
		return NULL;	
	}
	_tcscpy(psymr->name, pbase->name);
	// deep copy structure
	psymr->desc = pbase->desc;
	psymr->type = pbase->type;
	psymr->psym = pbase;
	psymr->next = NULL;
	return psymr;
}

//***********************************************************************
void SYMREFdestroy(PSYMREF psymr)
{
	if(psymr)
	{
		// references to the original sym dont
		// delete the member list
		//
		if(psymr->name)
		{
			free(psymr->name);
		}
		free(psymr);
	}
}

//***********************************************************************
PSYMREF SYMREFrename(PSYMREF psymr, const TCHAR* newname)
{
	if(! psymr) return NULL;
	if(! newname) return NULL;
	if(psymr->name) free(psymr->name);
	psymr->name	= (TCHAR*)malloc(_tcslen(newname) + 1);
	if(! psymr->name)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symref memory\n"));
		return NULL;	
	}
	_tcscpy(psymr->name, newname);
	return psymr;
}

//***********************************************************************
PSYMREF SYMREFcreateCopy(PSYMREF pbase)
{
	PSYMREF psymr = (PSYMREF)malloc(sizeof(SYMREF));
	
	if(! psymr)
	{
		if(g_log) g_log->Log(logError, 0, _T("Internal: symref memory\n"));
		return NULL;	
	}
	psymr->name	= (TCHAR*)malloc(_tcslen(pbase->name) + 1);
	if(! psymr->name)
	{
		free(psymr);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symref memory\n"));
		return NULL;	
	}
	_tcscpy(psymr->name, pbase->name);
	
	// structure deep copy
	psymr->desc = pbase->desc;
	psymr->desc.iscopy = 1;

	psymr->type		= pbase->type;
	psymr->psym		= pbase->psym;
	psymr->next		= NULL;
	return psymr;
}

//***********************************************************************
void SYMaddMember(PSYM psym, PSYM pmember)
{
	PSYM pmem;
	
	if(! psym || ! pmember) return;
	if(! psym->members)
	{
		psym->members = pmember;
		return;
	}
	for(pmem = psym->members; pmem->members;)
		pmem = pmem->members;
	pmem->members = pmember;
}

//***********************************************************************
PSYM SYMfindMember(TCHAR* name, PSYM psym)
{
	PSYM pmem;
	
	if(! psym || ! name) return NULL;
	for(pmem = psym->members; pmem;	pmem = pmem->members)
	{
		if(! _tcscmp(pmem->name, name))
		{
			return pmem;
		}
	}
	return NULL;
}

//***********************************************************************
void SYMsetType(PSYM psym, PSYM ptype, PSYMTYPE pdesc)
{
	SYMTYPE mdesc;

	psym->type = ptype;
	if(ptype)
	{
		// merge the desc from the type with the as-specified type
		// basically pdesc = (ptype->desc) | pdesc, but I only merge
		// the attributes 
		//
		memcpy(&mdesc, &ptype->desc, sizeof(mdesc));

		mdesc.isaggrt	|= pdesc->isaggrt;
		mdesc.isauto	|= pdesc->isauto;
		mdesc.isconst	|= pdesc->isconst;
		mdesc.isdim		+= pdesc->isdim;
		mdesc.isenum	|= pdesc->isenum;
		mdesc.isext		|= pdesc->isext;
		mdesc.isfunc	|= pdesc->isfunc;
		mdesc.isinlin	|= pdesc->isinlin;
		mdesc.islit		|= pdesc->islit;
		mdesc.islong	|= pdesc->islong;
		mdesc.ispack	|= pdesc->ispack;
		mdesc.isptr		+= pdesc->isptr;
		mdesc.istatic	|= pdesc->istatic;
		mdesc.isuns		|= pdesc->isuns;
		mdesc.isvol		|= pdesc->isvol;
		pdesc = &mdesc;
	}
	if(! pdesc->bsize)
	{
		if(! pdesc->isaggrt)
		{
			if(g_log) g_log->Log(logError, 0, _T("Internal: no size for basic type\n"));
			pdesc->bsize = sizeof(int); //CPU_INT_SIZE;
		}
	}
	// structure deep copy
	psym->desc = *pdesc;
}

//***********************************************************************
static unsigned long SYMdescSize(PSYMTYPE desc, PSYM ptype, const TCHAR* name)
{
	int bitsize;
	PSYM pmem;
	
	if(desc->isfunc)
	{
		if(ptype->type)
			return SYMgetSize(ptype->type);
		bitsize = sizeof(TCHAR*); //CPU_ADDRESS_SIZE;
	}
	else if(desc->isptr)
	{
		bitsize = sizeof(TCHAR*); //CPU_ADDRESS_SIZE;
	}
	else if(desc->isdim)
	{
		bitsize = desc->bsize * SYMgetSize(ptype->type ? ptype->type : ptype);
	}
	else if(desc->isaggrt)
	{
		bitsize = 0;
		ptype = SYMbaseType(ptype);
		if(ptype)
		{
			for(pmem = ptype->members; pmem; pmem = pmem->members)
			{
				bitsize += SYMgetSize(pmem);
			}
		}
		else
		{
			bitsize = 0;
		}
	}
	else
	{
		bitsize = desc->bsize;
	}
	if(g_log) g_log->Log(logDebug, 8, _T("sizeof %s is %d (%d bytes)\n"),
			name, bitsize, (bitsize + 7) / 8);

	return bitsize;
}

//***********************************************************************
unsigned long SYMgetSize(PSYM psym)
{
	return SYMdescSize(&psym->desc, psym, psym->name);
}

//***********************************************************************
PSYM SYMbaseType(PSYM psym)
{
	while(psym && psym->type)
		psym = psym->type;
	return psym;
}

//***********************************************************************
unsigned long SYMREFgetSize(PSYMREF psymr)
{
	return SYMdescSize(&psymr->desc, psymr->type, psymr->name);
}

//***********************************************************************
unsigned long SYMgetSizeBytes(PSYM psym)
{
	return (7 + SYMdescSize(&psym->desc, psym, psym->name)) / 8;
}

//***********************************************************************
unsigned long SYMREFgetSizeBytes(PSYMREF psymr)
{
	return (7 + SYMdescSize(&psymr->desc, psymr->psym, psymr->name)) / 8;
}

//***********************************************************************
PSYM SYMTABaddSym(PSYM psym, PSYM* table)
{
	PSYM ptsym, pvsym;
	int  hash, cmp;

	if(! table)
		return NULL;
	
	hash = SYMHASH(psym->name);

	ptsym = table[hash];
	pvsym = NULL;
	if(! ptsym)
	{
		table[hash] = psym;
		psym->next  = NULL;
	}
	else while(ptsym)
	{
		cmp = _tcscmp(psym->name, ptsym->name);
		if(cmp < 0)  
		{
			psym->next = ptsym;
			if(pvsym)
				pvsym->next = psym;
			else
				table[hash] = psym;
			break;
		}
		else if (cmp > 0)
		{
			pvsym = ptsym;
			ptsym = ptsym->next;
			if(! ptsym)
			{
				pvsym->next = psym;
				psym->next  = NULL;
				break;
			}
		}
		else
		{
			// duplicate, whine and return
			//
			SYMdestroy(psym);
			psym = ptsym;
			break;
		}
	}
	return psym;
}

//***********************************************************************
PSYMTAB SYMTABcreate()
{
	PSYMTAB ptab;
	
	ptab = (PSYMTAB)malloc(sizeof(SYMTAB));
	if(! ptab)
	{
		free(ptab);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	
	ptab->symbols = (PSYM*)malloc(SYMHASH_SIZE * sizeof(PSYM));
	if(! ptab->symbols)
	{
		free(ptab);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;	
	}
	memset(ptab->symbols, 0, SYMHASH_SIZE * sizeof(PSYM));
	
	ptab->types = (PSYM*)malloc(SYMHASH_SIZE * sizeof(PSYM));
	if(! ptab->types)
	{
		free(ptab->symbols);
		free(ptab);
		if(g_log) g_log->Log(logError, 0, _T("Internal: symtab memory\n"));
		return NULL;
	}
	memset(ptab->types, 0, SYMHASH_SIZE * sizeof(PSYM));
	
	ptab->prev = ptab->next = NULL;	
	return ptab;
}

//***********************************************************************
void SYMTABdestroyTable(PSYM* table)
{
	PSYM psym, pnsym;
	int  hash;

	if(! table)
		return;
	
	for(hash = 0; hash < SYMHASH_SIZE; hash++)
	{
		psym = table[hash];
		while(psym)
		{
			pnsym = psym->next;
			SYMdestroy(psym);
			psym = pnsym;
		}
		table[hash] = NULL;
	}
}

//***********************************************************************
void SYMTABdestroy(PSYMTAB ptab)
{
	if(! ptab) return;
	SYMTABdestroyTable(ptab->symbols);
	SYMTABdestroyTable(ptab->types);
	free(ptab->symbols);
	free(ptab->types);
	free(ptab);
}

//***********************************************************************
void SYMTABprintSym(PSYM sym, int level)
{
	int  i;
	
	for(i = 0; i <= level; i++)
	{
		_tprintf(_T("    "));
	}
	_tprintf(_T("" _Pfs_ " "), sym->name);
	if(sym->desc.isptr)
	{
		for(i = 0; i < (int)sym->desc.isptr; i++)
		{
			_tprintf(_T("->"));
		}
	}
	else
	{
		_tprintf(_T("  "));
	}
	if(sym->desc.isext)   _tprintf(_T("extern "));
	if(sym->desc.istatic) _tprintf(_T("static "));
	if(sym->desc.isconst) _tprintf(_T("const "));
	if(sym->desc.isvol)   _tprintf(_T("volatile "));
	if(sym->desc.isuns)   _tprintf(_T("unsigned "));
	if(sym->desc.isreal)  _tprintf(_T("real "));
	if(sym->desc.isreg)   _tprintf(_T("register "));
	if(sym->desc.islong)  _tprintf(_T("long "));
	if(sym->desc.isvoid)  _tprintf(_T("void "));
	if(sym->desc.islit)   _tprintf(_T("literal "));
	if(sym->desc.isenum)  _tprintf(_T("enum "));
	if(sym->desc.ispack)  _tprintf(_T("packed "));
	if(sym->desc.isfunc)  _tprintf(_T("()"));
	for(i = 0; i < (int)sym->desc.isdim; i++)
	{
		_tprintf(_T("[]"));
	}
}

//***********************************************************************
int SYMTABtypeString(PSYM sym, TCHAR* buf, int nbuf)
{
	int tlen = 0, i;
	PSYM types;
	
	if(sym->desc.isext)   tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("extern "));
	if(sym->desc.istatic) tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("static "));
	if(sym->desc.isconst) tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("const "));
	if(sym->desc.isvol)   tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("volatile "));
	if(sym->desc.isuns)   tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("unsigned "));
	if(sym->desc.isreg)   tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("register "));
	if(sym->desc.islong)  tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("long "));
	if(sym->desc.isvoid)  tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("void "));

	types = sym->type;
	while(types)
	{
		tlen += SYMTABtypeString(types, buf + tlen, nbuf - tlen);
		types = types->type;
	}
	if(sym->desc.isptr)
	{
		for(i = 0; i < (int)sym->desc.isptr; i++)
		{
			tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("*"));
		}
	}
	tlen += _sntprintf(buf + tlen, nbuf - tlen, _T(" " _Pfs_ ""), sym->name);

	for(i = 0; i < (int)sym->desc.isdim; i++)
	{
		tlen += _sntprintf(buf + tlen, nbuf - tlen, _T("[]"));
	}
	return tlen;
}

//***********************************************************************
void SYMTABdumpSym(PSYM* table, int level)
{
	PSYM psym;
	PSYM types;
	int  hash, i;

	if(! table)
		return;
	
	for(hash = 0; hash < SYMHASH_SIZE; hash++)
	{
		psym = table[hash];
		while(psym)
		{	
			SYMTABprintSym(psym, level);
			types = psym->type;
			while(types)
			{
				SYMTABprintSym(types, level);
				types = types->type;
			}
			_tprintf(_T("\n"));
			if(psym->members)
			{
				PSYM member = psym->members;
						
				for(i = 0; i <= level; i++)
				{
					_tprintf(_T("    "));
				}
				_tprintf(_T("%c\n"), psym->desc.isfunc ? '(' : '{');
				while(member)
				{
					SYMTABprintSym(member, level + 1);
					types = member->type;
					while(types)
					{
						SYMTABprintSym(types, level);
						types = types->type;
					}
					_tprintf(_T("\n"));
					member = member->members;
				}	
				for(i = 0; i <= level; i++)
				{
					_tprintf(_T("    "));
				}
				_tprintf(_T("%c\n"), psym->desc.isfunc ? ')' : '}');
			}
			psym = psym->next;
		}
	}
}

//***********************************************************************
void SYMTABdump(PSYMTAB ptab, int level)
{
	if(! ptab) return;
	if(g_log->GetLevel() < 7) return;
	if(g_log) g_log->Log(logDebug, 7, _T("SYMBOLS level %d\n"), level);
	SYMTABdumpSym(ptab->symbols, 0);
	if(g_log) g_log->Log(logDebug, 7, _T("TYPES level %d\n"), level);
	SYMTABdumpSym(ptab->types, 0);
}

//***********************************************************************
void SYMTABmove(PSYMTAB pdst, PSYMTAB psrc)
{
	PSYM psym, pnsym;
	int  hash;

	if(! pdst || ! psrc)
		return;
	
	for(hash = 0; hash < SYMHASH_SIZE; hash++)
	{
		psym = psrc->symbols[hash];
		psrc->symbols[hash] = NULL;
		
		while(psym)
		{
			pnsym = psym->next;
			psym->next = NULL;
			SYMTABaddSym(psym, pdst->symbols);
			psym = pnsym;
		}
		
		psym = psrc->types[hash];
		psrc->types[hash] = NULL;
		
		while(psym)
		{
			pnsym = psym->next;
			psym->next = NULL;
			SYMTABaddSym(psym, pdst->types);
			psym = pnsym;
		}
	}
}

//***********************************************************************
PSYM SYMTABfindSym(const TCHAR* name, PSYM* table)
{
	PSYM psym;
	int  hash;
	int  cmp;
	
	if(! table || ! name)
		return NULL;

	hash = SYMHASH(name);
	
	for(psym = table[hash]; psym; psym = psym->next)
	{
		cmp = _tcscmp(name, psym->name);

		if(cmp == 0)
			return psym;
		if(cmp < 0)  
			return NULL;
	}
	return NULL;
}

//***********************************************************************
void SYMTABremoveSym(PSYM psym, PSYM* table)
{
	PSYM ptsym;
	int  hash;
	
	if(! table || ! psym)
		return;

	hash  = SYMHASH(psym->name);
	ptsym = table[hash];
	
	if(psym == ptsym)
	{
		table[hash] = psym->next;
	}
	else while(ptsym->next)
	{
		if(ptsym->next == psym)
		{
			ptsym->next = psym->next;
			break;
		}
	}
	psym->next = NULL;
}

//***********************************************************************
PSYM SYMTABfindVar(const TCHAR* name, PSYMTAB ptab)
{
	PSYM psym;
	
	while(ptab)
	{
		psym = SYMTABfindSym(name, ptab->symbols);
		if(psym) return psym;
		ptab = ptab->prev;
	}
	return NULL;
}

//***********************************************************************
PSYM SYMTABfindType(const TCHAR* name, PSYMTAB ptab)
{
	PSYM psym;
	
	while(ptab)
	{
		psym = SYMTABfindSym(name, ptab->types);
		if(psym) return psym;
		ptab = ptab->prev;
	}
	return NULL;
}

//***********************************************************************
int SYMTABwalk(PSYM *psymtab, SYMWALKFUNC callback, void* cookie)
{
	PSYM psym;
	int  hash;
	int  rv;
	
	if(! psymtab || ! callback)
		return 1;
	
	for(hash = 0; hash < SYMHASH_SIZE; hash++)
	{
		psym = psymtab[hash];
		
		while(psym)
		{
			rv = callback(psym, cookie);
			if(rv) return rv;
			psym = psym->next;
		}
	}

	return 0;
}

