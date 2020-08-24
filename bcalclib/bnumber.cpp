
#include "bcalcx.h"

LPSTR Bnumber::m_rb = NULL;
DWORD Bnumber::m_nrb = 0;


//**************************************************************************
LPCSTR Bnumber::Format(int nd, int radix, DisplayFormat format)
{
	if((DWORD)nd > m_nrb)
	{
		if(m_rb) delete [] m_rb;
		m_rb = new CHAR [ nd + 32 ];
		m_nrb = nd + 16;
	}
	return m_rb;
}


//**************************************************************************
Bnumber* Bstack::Push(Bnumber* pn)
{
	if(pn)
	{
		pn->m_next = m_top;
		m_top = pn;
	}
	return pn;
}

//**************************************************************************
Bnumber* Bstack::Pop(CODE& code)
{
	Bnumber* pn = m_top;

	if(m_top)
	{
		code  = m_top->m_code;
		m_top = m_top->m_next;
	}
	return pn;
}

//**************************************************************************
void Bstack::Clear(void)
{
	Bnumber* pn;
	CODE	 code;

	while(GetTop())
	{
		pn = Pop(code);
		delete pn;
	}
}

//**************************************************************************
ERRCODE Bstack::Execute(execRange to)
{
	Bnumber* pn  = NULL;
	Bnumber* pnb = NULL;
	ERRCODE  ec = errOK;
	CODE	 code;

	//Dump(_T("Execute"));

	do
	{
		pn = Pop(code);
		if(! pn) 
			return errUNDERFLOW;
	
		switch(code)
		{
		case ID_OPR_EQUAL:
		case ID_OPR_RPAREN:
			if(! m_top)
			{
				Push(pn);
				return errOK;
			}
			pnb = Pop(code);
			if(! pnb)
				return errUNDERFLOW;
			if(pnb->m_code == ID_OPR_LPAREN)
			{
				delete pnb;
				Push(pn);
				if(to == toParen)
					return errOK;
				else
					return errFAILURE;
			}
		}
		switch(code)
		{
		case ID_OPR_NEG:
			Push(pn->Negate(pn, ec));
			break;
		case ID_OPR_NOT:
			Push(pn->BitInvert(pn, ec));
			break;
		case ID_OPR_INV:
			Push(pn->Invert(pn, ec));
			break;
		case ID_OPR_ADD:
			Push(pn->Add(pn, pnb, ec));
			break;
		case ID_OPR_SUB:
			Push(pn->Sub(pn, pnb, ec));
			break;
		case ID_OPR_MUL:
			Push(pn->Mul(pn, pnb, ec));
			break;
		case ID_OPR_DIV:
			Push(pn->Div(pn, pnb, ec));
			break;
		case ID_OPR_MOD:
			Push(pn->Mod(pn, pnb, ec));
			break;
		case ID_OPR_AND:
			Push(pn->And(pn, pnb, ec));
			break;
		case ID_OPR_OR:
			Push(pn->Or(pn, pnb, ec));
			break;
		case ID_OPR_XOR:
			Push(pn->Xor(pn, pnb, ec));
			break;
		case ID_OPR_SHFTL:
			Push(pn->ShiftLeft(pn, ec));
			break;
		case ID_OPR_SHFTR:
			Push(pn->ShiftRight(pn, ec));
			break;
		case ID_OPR_INT:
			Push(pn->Floor(pn, ec));
			break;
		case ID_OPR_FRAC:
			Push(pn->Frac(pn, ec));
			break;
		case ID_OPR_SQ:
			Push(pn->Mul(pn, pn, ec));
			break;
		case ID_OPR_SQRT:
			Push(pn->Sqrt(pn, ec));
			break;
		case ID_OPR_SIN:
			Push(pn->Sin(pn, ec));
			break;
		case ID_OPR_COS:
			Push(pn->Cos(pn, ec));
			break;
		case ID_OPR_TAN:
			Push(pn->Tan(pn, ec));
			break;
		case ID_OPR_ASIN:
			Push(pn->aSin(pn, ec));
			break;
		case ID_OPR_ACOS:
			Push(pn->aCos(pn, ec));
			break;
		case ID_OPR_ATAN:
			Push(pn->aTan(pn, ec));
			break;
		case ID_OPR_EXPO:
			Push(pn->Exp(pn, ec));
			break;
		case ID_OPR_NLOG:
			Push(pn->nLog(pn, ec));
			break;
		case ID_OPR_XEXPY:
			Push(pn->XtoY(pn, pnb, ec));
			break;
		case ID_OPR_10EXP:
			Push(pn->TenX(pn, ec));
			break;
		case ID_OPR_LOG:
			Push(pn->Log(pn, ec));
			break;
		default:
			ec = errBAD_PARAMETER;
			break;
		}
		if(pn)  delete pn;
		if(pnb) delete pnb;

		switch(to)
		{
		case toTop:
			return ec;
		case toBottom:
			if(! GetTop() || (! (GetTop()->m_next) && (GetTop()->m_code == ID_OPR_EQUAL)))
				return ec;
		}
	}
	while(ec == errOK);
	return ec;
}

//**************************************************************************
void Bstack::Dump(LPCTSTR msg)
{
	Bnumber* pn;
	TCHAR    ts[512];
	LPCSTR   nf;

	OutputDebugString(msg);
	OutputDebugString(_T("\n"));

	for(pn = m_top; pn;)
	{
		nf = pn->Format(32, 10, kbDefault);
		ts[0] = (TCHAR)pn->m_code;
		ts[1] = _T(' ');
		ts[2] = 0;
#ifdef UNICODE
		BUtil::Utf8Decode(ts + 2, nf);
#else
		strcpy(ts + 2, nf);
#endif
		OutputDebugString(ts);
		OutputDebugString(_T("\n"));
		pn = pn->m_next;
	}
	OutputDebugString(_T("\n"));
}
