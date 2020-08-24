
#include "openfilex.h"

//***********************************************************************
Bflentry::Bflentry(LPCTSTR name, int type)
{
	m_name			 = new TCHAR [ _tcslen(name)  + 2 ];
	_tcscpy(m_name, name);
	m_type			 = type;
	m_left = m_right = NULL;
}

//***********************************************************************
Bflentry::~Bflentry()
{
	delete [] m_name;
}

//***********************************************************************
Bflentry* Bflentry::AddFileEntry(Bflentry* sym, Bflentry*& tree)
{
	if(! tree) return tree = sym;

	int cmp = _tcscmp(sym->m_name, tree->m_name);

	if(cmp < 0)  
		return AddFileEntry(sym, tree->m_left);
	if(cmp > 0)
		return AddFileEntry(sym, tree->m_right);

	delete sym;
	return tree;
}

//***********************************************************************
void Bflentry::DumpFileEntry(Bflentry* tree)
{
	if(! tree) return;
	if(tree->m_left)
		DumpFileEntry(tree->m_left);
	DumpFileEntry(tree->m_right);
}

//***********************************************************************
void Bflentry::DeleteFileEntry(Bflentry* sym)
{
	if(! sym) return;
	DeleteFileEntry(sym->m_left);
	sym->m_left = NULL;
	DeleteFileEntry(sym->m_right);
	sym->m_right = NULL;
	delete sym;
}

//***********************************************************************
Bflentry* Bflentry::FindFileEntry(LPCTSTR name, Bflentry* tree)
{
	if(! tree || ! name) return NULL;

	int cmp = _tcscmp(name, tree->m_name);

	if(cmp < 0)  
		return FindFileEntry(name, tree->m_left);
	if(cmp > 0)
		return FindFileEntry(name, tree->m_right);
	return tree;
}

//***********************************************************************
Bflentry* Bflentry::GetFileEntryNumber(Bflentry* tree, int num)
{
	if(! tree || num < 0) return NULL;

	if(tree->m_left)
		return GetFileEntryNumber(tree->m_left, num);
	
	if(num == 0) return tree;

	if(tree->m_right)
		return GetFileEntryNumber(tree->m_right, num - 1);

	return tree;
}

//***********************************************************************
void Bflentry::PruneFileEntry(Bflentry* tree, LPCTSTR pattern, Bflentry*& subtree, bool dironly)
{
	if(! tree || ! pattern) return;

	//if((dironly && (tree->m_type & 8)) || (!dironly && ! (tree->m_type & 8)))
	if(! dironly || (tree->m_type & 8))
	{
		if(BUtil::SimplePatternMatch(tree->m_name, pattern))
		{
			Bflentry* ne = new Bflentry(tree->m_name, tree->m_type);

			AddFileEntry(ne, subtree);
		}
	}
	PruneFileEntry(tree->m_left, pattern, subtree, dironly);
	PruneFileEntry(tree->m_right, pattern, subtree, dironly);
}


//***********************************************************************
void Bflentry::InsertFileEntry(Bflentry* tree, HWND hWnd)
{
	TVINSERTSTRUCT tvItem;

	if(! tree) return;

	InsertFileEntry(tree->m_left, hWnd);

	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_LAST;
	
	tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM;
	tvItem.item.pszText = tree->m_name;
	tvItem.item.lParam	= 0;
	
	TreeView_InsertItem(hWnd, &tvItem);

	InsertFileEntry(tree->m_right, hWnd);
}

