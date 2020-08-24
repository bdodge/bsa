
#include "bcalcx.h"

//**************************************************************************
BdblNumber::BdblNumber(double n, CODE code)
	: Bnumber(code)
{
	m_val = n;
};


//**************************************************************************
BdblNumber::BdblNumber(LPCSTR pt, int radix, CODE code)
	: Bnumber(code)
{
	int d;
	int p = 0;
	int n = 0;
	int g = 0;

	m_val = 0.0;
	while(*pt)
	{
		if(*pt == '.')
		{
			p = 1;
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
			m_val = 0.0;
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
			{
				d = *pt - '0';
			}
			pt++;
			if(p)
				n++;
			m_val *= (double)radix;
			m_val += (double)d;
		}
	}
	while(n < 0)
	{
		n++;
		m_val *= (double)radix;
	}
	while(n > 0)
	{
		n--;
		m_val /= (double)radix;
	}
	if(g)
		m_val = -m_val;
};

//**************************************************************************
LPCSTR BdblNumber::Format(int nd, int radix, DisplayFormat format)
{
	uberlong ival = (uberlong)m_val;
	CHAR     pf[64];
	double	 dval;
	int		 e;
	int		 prec;

	Bnumber::Format(nd, radix, format);

	prec = 8;

	switch(radix)
	{
	case 2:
		{
			CHAR* pd  = m_rb + nd - 1;
			CHAR* pld = pd;
			uberlong mask = 1;

			*pd-- = '\0';

			while(nd > 0 && mask)
			{
				if(ival & mask)
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
			return pld;
		}
	case 8:
		_snprintf(m_rb, nd, "%llo", ival);
		break;
	case 10:
		switch(format)
		{
		case kbDefault:
			dval = m_val;
			if((fabs(m_val) < 0.00000001 /*|| m_val > 100000000.0*/)&&(m_val != 0.0))
				return Format(nd, radix, kbScientific);
			_snprintf(pf, 64, "%%.%df", prec);
			break;
		case kbScientific:
			for(dval = m_val, e = 0; fabs(dval) >= 10.0; dval /= 10.0)
				e++;
			for(; dval != 0.0 && fabs(dval) < 1.0; dval *= 10.0)
				e--;
			_snprintf(pf, 32, "%%.%dfe%03d", prec, e);
			break;
		case kbEngineer:
			for(dval = m_val, e = 0; (fabs(dval) >= 1000.0) || (e % 3); dval /= 10.0)
				e++;
			for(; (dval != 0.0 && fabs(dval) < 1.0) || (e % 3); dval *= 10.0)
				e--;
			_snprintf(pf, 64, "%%.%dfe%03d", prec, e);
			break;
		}
		_snprintf(m_rb, nd, pf, dval);
		{
			char* pe, *ps;

			for(pe = m_rb; (*pe && *pe != ' ' && *pe != 'e' && *pe != 'E');)
				pe++;
			if(pe > m_rb)
			{
				ps = pe;
				pe--;
				while(pe >= m_rb && *pe == ' ' || *pe == '0')
					pe--;
				pe++;
				if(format != kbDefault && pe < ps)
					pe++;
				while(*ps)
					*pe++ = *ps++;
				*pe = '\0';
			}
		}
		break;
	case 16:
		_snprintf(m_rb, nd, "%llX", ival);
		break;
	}
	return m_rb;
}

//**************************************************************************
bool BdblNumber::IsNegative()
{
	return m_val < 0.0;
}

//**************************************************************************
bool BdblNumber::IsZero()
{
	return m_val == 0.0;
}

//**************************************************************************
Bnumber* BdblNumber::Negate(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	ec = errOK;
	return new BdblNumber(-(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Invert(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	if(pin->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	return new BdblNumber(1.0/(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Add(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BdblNumber* pina = (BdblNumber*)pna;
	BdblNumber* pinb = (BdblNumber*)pnb;

	ec = errOK;
	return new BdblNumber(pina->m_val + pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Sub(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BdblNumber* pina = (BdblNumber*)pna;
	BdblNumber* pinb = (BdblNumber*)pnb;

	ec = errOK;
	return new BdblNumber(pinb->m_val - pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Mul(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BdblNumber* pina = (BdblNumber*)pna;
	BdblNumber* pinb = (BdblNumber*)pnb;

	ec = errOK;
	return new BdblNumber(pina->m_val * pinb->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Div(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BdblNumber* pina = (BdblNumber*)pna;
	BdblNumber* pinb = (BdblNumber*)pnb;

	if(pina->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	ec = errOK;
	return new BdblNumber(pinb->m_val / pina->m_val, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Floor(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(floor(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Frac(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(pin->m_val - floor(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Sqrt(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(sqrt(pin->m_val), ID_OPR_EQUAL);
}

#define RADCVT(a) a

//**************************************************************************
Bnumber* BdblNumber::Sin(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(sin(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Cos(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(cos(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Tan(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(tan(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::aSin(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(asin(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::aCos(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(acos(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::aTan(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(atan(RADCVT(pin->m_val)), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Exp(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(exp(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::nLog(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(log(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::TenX(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(pow(10.0, pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::Log(Bnumber* pn, ERRCODE& ec)
{
	BdblNumber* pin = (BdblNumber*)pn;

	return new BdblNumber(log10(pin->m_val), ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BdblNumber::XtoY(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BdblNumber* pina = (BdblNumber*)pna;
	BdblNumber* pinb = (BdblNumber*)pnb;

	return new BdblNumber(pow(pinb->m_val, pina->m_val), ID_OPR_EQUAL);
}
