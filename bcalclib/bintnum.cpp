
#include "bcalcx.h"

//**************************************************************************
BintNumber::BintNumber(uberlong n, CODE code)
	: Bnumber(code)
{
	m_val = n;
};


//**************************************************************************
BintNumber::BintNumber(LPCSTR pt, int radix, CODE code)
	: Bnumber(code)
{
	int d;
	int n = 0;
	int g = 0;

	m_val = 0;
	while(*pt)
	{
		if(*pt == '.')
		{
			n = 1;
			pt++;
		}
		else if(*pt == '-')
		{
			g = 1;
			pt++;
		}
		else if (*pt == 'x' || *pt == 'X')
		{
			radix = 16;
			pt++;
			m_val = 0;
		}
		else
		{
			if(radix > 10 && *pt >= 'A' && *pt <= 'F')
				d = *pt - ('A' - 10);
			else if(radix > 10 && *pt >= 'a' && *pt <= 'f')
				d = *pt - ('a' - 10);
			else if(*pt == 'e')
			{
				n -= strtol(pt+1, (LPSTR*)&pt, 0);
				break;
			}
			else
				d = *pt - '0';
			pt++;
			if(n > 0) n++;
			m_val *= (uberlong)radix;
			m_val += (uberlong)d;
		}
	}
	while(n < 0)
	{
		n++;
		m_val *= (uberlong)radix;
	}
	while(n > 1)
	{
		n--;
		m_val /= (uberlong)radix;
	}
	if(g)
		m_val = -m_val;
}

//**************************************************************************
LPCSTR BintNumber::Format(int nd, int radix, DisplayFormat format)
{
	Bnumber::Format(nd, radix, format);
	switch(radix)
	{
	case 2:
		{
			CHAR* pd   = m_rb + nd - 1;
			CHAR* pld  = pd;
			CHAR* ipld = pd;
			uberlong mask = 1;

			*pd-- = '\0';

			while(nd > 0 && mask)
			{
				if(m_val & mask)
				{
					*pd = '1';
					pld = pd;
				}
				else
				{
					*pd = '0';
				}
				mask <<= 1;
				pd--;
				nd--;
			}
			if(pld == ipld)
				pld--;
			return pld;
		}
#ifdef Windows
	case 8:
		_snprintf(m_rb, nd, "%I64o", m_val);
		break;
	case 10:
		_snprintf(m_rb, nd, "%I64d", m_val);
		break;
	case 16:
		_snprintf(m_rb, nd, "%I64X", m_val);
		break;
#else
	case 8:
		_snprintf(m_rb, nd, "%llo", m_val);
		break;
	case 10:
		_snprintf(m_rb, nd, "%lld", m_val);
		break;
	case 16:
		_snprintf(m_rb, nd, "%llX", m_val);
		break;
#endif
	}
	return m_rb;
}

//**************************************************************************
bool BintNumber::IsNegative()
{
	return m_val < 0;
}

//**************************************************************************
bool BintNumber::IsZero()
{
	return m_val == 0;
}

//**************************************************************************
Bnumber* BintNumber::Negate(Bnumber* pn, ERRCODE& ec)
{
	BintNumber* pin = (BintNumber*)pn;

	ec = errOK;
	return new BintNumber(-(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::BitInvert(Bnumber* pn, ERRCODE& ec)
{
	BintNumber* pin = (BintNumber*)pn;

	ec = errOK;
	return new BintNumber(~(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Invert(Bnumber* pn, ERRCODE& ec)
{
	BintNumber* pin = (BintNumber*)pn;

	if(pin->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	return new BintNumber(1/(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::ShiftLeft(Bnumber* pn, ERRCODE& ec)
{
	BintNumber* pin = (BintNumber*)pn;

	ec = errOK;
	return new BintNumber(pin->m_val << 1, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::ShiftRight(Bnumber* pn, ERRCODE& ec)
{
	BintNumber* pin = (BintNumber*)pn;

	ec = errOK;
	return new BintNumber(pin->m_val >> 1, ID_OPR_EQUAL);
}


//**************************************************************************
Bnumber* BintNumber::Add(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pina->m_val + pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Sub(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pinb->m_val - pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Mul(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pina->m_val * pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Div(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	if(pina->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	ec = errOK;
	return new BintNumber(pinb->m_val / pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Mod(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pinb->m_val % pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::And(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pina->m_val & pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Or(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pina->m_val | pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::Xor(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	ec = errOK;
	return new BintNumber(pina->m_val ^ pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BintNumber::XtoY(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BintNumber* pina = (BintNumber*)pna;
	BintNumber* pinb = (BintNumber*)pnb;

	uberlong y = pina->m_val;

	if(y < 0) return new BintNumber("0", 10, ID_OPR_EQUAL);

	BintNumber* acc = new BintNumber("1", 10, ID_OPR_EQUAL);
	BintNumber* nacc;

	while(y-- > 0)
	{
		nacc = (BintNumber*)acc->Mul(acc, pinb, ec);
		delete acc;
		acc = nacc;
	}
	return acc;
}


//**************************************************************************
BuintNumber::BuintNumber(uberlong n, CODE code)
	: Bnumber(code)
{
	m_val = n;
};


//**************************************************************************
BuintNumber::BuintNumber(LPCSTR pt, int radix, CODE code)
	: Bnumber(code)
{
	unsigneduberlong d;

	m_val = 0;
	while(*pt)
	{
		if(*pt == '.') 
			break;
		if(*pt >= 'A' && *pt <= 'F')
			d = *pt - ('A' - 10);
		else if(*pt >= 'a' && *pt <= 'f')
			d = *pt - ('a' - 10);
		else
			d = *pt - '0';
		pt++;
		switch(radix)
		{
		case 2:
			m_val <<= 1;
			m_val |= (d ? 1 : 0);
			break;
		case 8:
			m_val <<= 3;
			m_val |= (d & 7);
			break;
		case 10:
			m_val *= 10;
			m_val += d;
			break;
		case 16:
			m_val <<= 4;
			m_val |= (d & 15);
			break;
		}
	}
}

//**************************************************************************
LPCSTR BuintNumber::Format(int nd, int radix, DisplayFormat format)
{
	Bnumber::Format(nd, radix, format);
	switch(radix)
	{
	case 2:
		{
			CHAR* pd   = m_rb + nd - 1;
			CHAR* pld  = pd;
			CHAR* ipld = pd;
			uberlong mask = 1;

			*pd-- = '\0';

			while(nd > 0 && mask)
			{
				if(m_val & mask)
				{
					*pd = '1';
					pld = pd;
				}
				else
				{
					*pd = '0';
				}
				mask <<= 1;
				pd--;
				nd--;
			}
			if(pld == ipld)
				pld--;
			return pld;
		}
	case 8:
#ifdef Windows
		_snprintf(m_rb, nd, "%I64o", m_val);
#else
		_snprintf(m_rb, nd, "%llo", m_val);
#endif
		break;
	case 10:
#ifdef Windows
		_snprintf(m_rb, nd, "%I64u", m_val);
#else
		_snprintf(m_rb, nd, "%llu", m_val);
#endif
		break;
	case 16:
#ifdef Windows
		_snprintf(m_rb, nd, "%I64X", m_val);
#else
		_snprintf(m_rb, nd, "%llX", m_val);
#endif
		break;
	}
	return m_rb;
}

//**************************************************************************
bool BuintNumber::IsNegative()
{
	return false;
}

//**************************************************************************
bool BuintNumber::IsZero()
{
	return m_val == 0;
}

//**************************************************************************
Bnumber* BuintNumber::BitInvert(Bnumber* pn, ERRCODE& ec)
{
	BuintNumber* pin = (BuintNumber*)pn;

	ec = errOK;
	return new BuintNumber(~(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Invert(Bnumber* pn, ERRCODE& ec)
{
	BuintNumber* pin = (BuintNumber*)pn;

	if(pin->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	return new BuintNumber(1/(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::ShiftLeft(Bnumber* pn, ERRCODE& ec)
{
	BuintNumber* pin = (BuintNumber*)pn;

	ec = errOK;
	return new BuintNumber(pin->m_val << 1, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::ShiftRight(Bnumber* pn, ERRCODE& ec)
{
	BuintNumber* pin = (BuintNumber*)pn;

	ec = errOK;
	return new BuintNumber(pin->m_val >> 1, ID_OPR_EQUAL);
}


//**************************************************************************
Bnumber* BuintNumber::Add(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pina->m_val + pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Sub(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pinb->m_val - pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Mul(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pina->m_val * pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Div(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	if(pina->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	ec = errOK;
	return new BuintNumber(pinb->m_val / pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Mod(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pinb->m_val % pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::And(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pina->m_val & pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Or(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pina->m_val | pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::Xor(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	ec = errOK;
	return new BuintNumber(pina->m_val ^ pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BuintNumber::XtoY(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BuintNumber* pina = (BuintNumber*)pna;
	BuintNumber* pinb = (BuintNumber*)pnb;

	uberlong y = pina->m_val;

	if(y < 0) return new BuintNumber("0", 10, ID_OPR_EQUAL);

	BuintNumber* acc = new BuintNumber("1", 10, ID_OPR_EQUAL);
	BuintNumber* nacc;

	while(y-- > 0)
	{
		nacc = (BuintNumber*)acc->Mul(acc, pinb, ec);
		delete acc;
		acc = nacc;
	}
	return acc;
}

