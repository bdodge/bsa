
#include "bcalcx.h"

//**************************************************************************
int XtoI(CHAR x)
{
	if(x >= 'a' && x <= 'f')
		return x - ('a' - 10);
	else if(x >= 'A' && x <= 'F')
		return x - ('A' - 10);
	else
		return x - '0';
}

//**************************************************************************
int ItoX(int i)
{
	return	i >= 10 ? i + 'A' - 10 : i + '0';
}

//**************************************************************************
int BarbNumber::Compare(BarbNumber* pna, BarbNumber* pnb)
{
	if(pna->m_len > pnb->m_len)
	{
		return 1;
	}
	else if(pna->m_len == pnb->m_len)
	{
		int i;

		for(i = 0; i < pna->m_len; i++)
		{
			if(pna->m_val[i] > pnb->m_val[i])
			{
				return 1;
			}
			else if(pna->m_val[i] < pnb->m_val[i])
			{
				return -1;
			}
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

//**************************************************************************
BarbNumber::BarbNumber(BarbNumber* n, CODE code)
	: Bnumber(code)
{
	m_val = new CHAR [ n->m_alloc ];
	memcpy(m_val, n->m_val, n->m_alloc);
	m_alloc = n->m_alloc;
	m_len   = n->m_len;
	m_neg   = n->m_neg;
	m_radix	= n->m_radix;
};


//**************************************************************************
BarbNumber::BarbNumber(LPCSTR pt, int radix, CODE code)
	: Bnumber(code)
{
	long len;
	
	m_neg = false;
	if(*pt == '-')
	{
		m_neg = true;
		pt++;
	}	
	if (! radix)
	{
		if (*pt == 'x' || *pt == 'X')
		{
			radix = 16;
			pt++;
		}
		else if (*pt == '0' && (pt[1] == 'x' || pt[1] == 'X'))
		{
			radix = 16;
			pt += 2;
		}
		else
		{
			radix = 10;
		}
	}
	len = strlen(pt);
	m_alloc = len + 32;
	m_val = new CHAR [ m_alloc ];
	memcpy(m_val, pt, len + 1);
	pt = strstr(m_val, ".");
	if(pt)
	{
		char* xt = (char*)pt;

		*xt = '\0';
		len = strlen(m_val);
	}
	m_len = len;
	m_radix = radix;
};

//**************************************************************************
LPCSTR BarbNumber::Format(int nd, int radix, DisplayFormat format)
{
	LPSTR	pb;
	ERRCODE ec;
	int		i, j, k, d;

	Bnumber::Format(m_len * 4 + 4, radix, format);

	pb = m_rb;
	if(m_neg && m_len)
		*pb++ = '-';
	if(m_len)
	{
		switch(m_radix)
		{
		case 2:
			switch(radix)
			{
			case 2:
				memcpy(pb, m_val, m_len);
				pb[m_len] = 0;
				break;
			case 8:
			case 16:
				k = (radix == 8) ? 3 : 4;
				pb += (m_len + (k - 1)) / k;
				*pb-- = 0;
				for(i = m_len - 1, j = d = 0; i >= 0; i--)
				{
					d += (XtoI(m_val[i]) << j);
					if(++j == k)
					{
						*pb-- = ItoX(d);
						j = d = 0;
					}
				}
				if(j > 0)
					*pb-- = ItoX(d);
				break;
			case 10:
				{
					BarbNumber* acc = new BarbNumber("0", 10, ID_OPR_EQUAL);
					BarbNumber* mul = new BarbNumber("1", 10, ID_OPR_EQUAL);
					BarbNumber* two = new BarbNumber("2", 10, ID_OPR_EQUAL);
					BarbNumber* nacc, *nmul;

					for(i = m_len - 1; i >= 0; i--)
					{
						if(XtoI(m_val[i]) != 0)
						{
							nacc = (BarbNumber*)acc->Add(acc, mul, ec);
							delete acc;
							acc = nacc;
						}
						nmul = (BarbNumber*)mul->Mul(mul, two, ec);
						delete mul;
						mul = nmul;
					}
					memcpy(pb, acc->m_val, acc->m_len);
					pb[acc->m_len] = '\0';
					delete acc;
					delete mul;
					delete two;
				}
				break;
			}
			break;
		case 8:
			switch(radix)
			{
			case 2:
				pb += m_len * 3;
				*pb-- = 0;
				for(i = m_len - 1, j = d = 0; i >= 0; i--)
				{
					d = XtoI(m_val[i]);
					*pb-- = ItoX((d & 1) ? 1 : 0);
					*pb-- = ItoX((d & 2) ? 1 : 0);
					*pb-- = ItoX((d & 4) ? 1 : 0);
				}
				while(pb[1] == '0')
					pb++;
				if(m_neg)
					*pb = '-';
				else
					pb++;
				return pb;
				break;
			case 8:
				memcpy(pb, m_val, m_len);
				pb[m_len] = 0;
				break;
			case 10:
			case 16:
				{
					BarbNumber* acc   = new BarbNumber("0", radix, ID_OPR_EQUAL);
					BarbNumber* mul   = new BarbNumber("1", radix, ID_OPR_EQUAL);
					BarbNumber* eight = new BarbNumber("8", radix, ID_OPR_EQUAL);
					BarbNumber* nacc, *nmul;

					for(i = m_len - 1; i >= 0; i--)
					{
						if(XtoI(m_val[i]) != 0)
						{
							char        vb[2] = { m_val[i], '\0' };
							BarbNumber* val   = new BarbNumber(vb, radix, ID_OPR_EQUAL);

							nmul = (BarbNumber*)val->Mul(val, mul, ec);
							delete val;
							nacc = (BarbNumber*)acc->Add(acc, nmul, ec);
							delete acc;
							delete nmul;
							acc = nacc;
						}
						nmul = (BarbNumber*)mul->Mul(mul, eight, ec);
						delete mul;
						mul = nmul;
					}
					memcpy(pb, acc->m_val, acc->m_len);
					pb[acc->m_len] = '\0';
					delete acc;
					delete mul;
					delete eight;
				}
				break;
			}
			break;
		case 10:
			switch(radix)
			{
			case 10:
				memcpy(pb, m_val, m_len);
				pb[m_len] = 0;
				break;
			case 2:
			case 8:
			case 16:
				{
					BarbNumber* acc = new BarbNumber("0", radix, ID_OPR_EQUAL);
					BarbNumber* mul = new BarbNumber("1", radix, ID_OPR_EQUAL);
					BarbNumber* ten = new BarbNumber(((radix == 2) ? "1010" : ((radix == 8) ? "12" : "A")), radix, ID_OPR_EQUAL);
					BarbNumber* nacc, *nmul;

					for(i = m_len - 1; i >= 0; i--)
					{
						if((j = XtoI(m_val[i])) != 0)
						{
							const char* vs;
							BarbNumber* val;
							
							switch(radix)
							{
							case 2:
								switch(j)
								{
								case 1: vs = "1";  break;
								case 2: vs = "10"; break;
								case 3: vs = "11"; break;
								case 4: vs = "100";  break;
								case 5: vs = "101"; break;
								case 6: vs = "110"; break;
								case 7: vs = "111";  break;
								case 8: vs = "1000"; break;
								case 9: vs = "1001"; break;
								}
								break;
							case 8:
								switch(j)
								{
								case 1: vs = "1"; break;
								case 2: vs = "2"; break;
								case 3: vs = "3"; break;
								case 4: vs = "4"; break;
								case 5: vs = "5"; break;
								case 6: vs = "6"; break;
								case 7: vs = "7"; break;
								case 8: vs = "10"; break;
								case 9: vs = "11"; break;
								}
								break;
							case 16:
								switch(j)
								{
								case 1: vs = "1"; break;
								case 2: vs = "2"; break;
								case 3: vs = "3"; break;
								case 4: vs = "4"; break;
								case 5: vs = "5"; break;
								case 6: vs = "6"; break;
								case 7: vs = "7"; break;
								case 8: vs = "8"; break;
								case 9: vs = "9"; break;
								}
								break;
							}
							val = new BarbNumber(vs, radix, ID_OPR_EQUAL);

							nmul = (BarbNumber*)val->Mul(val, mul, ec);
							delete val;
							nacc = (BarbNumber*)acc->Add(acc, nmul, ec);
							delete acc;
							delete nmul;
							acc = nacc;
						}
						nmul = (BarbNumber*)mul->Mul(mul, ten, ec);
						delete mul;
						mul = nmul;
					}
					memcpy(pb, acc->m_val, acc->m_len);
					pb[acc->m_len] = '\0';
					delete acc;
					delete mul;
					delete ten;
				}
				break;
				break;
			}
			break;
		case 16:
			switch(radix)
			{
			case 2:
				pb += m_len * 4;
				*pb-- = 0;
				for(i = m_len - 1, j = d = 0; i >= 0; i--)
				{
					d = XtoI(m_val[i]);
					*pb-- = ItoX((d & 1) ? 1 : 0);
					*pb-- = ItoX((d & 2) ? 1 : 0);
					*pb-- = ItoX((d & 4) ? 1 : 0);
					*pb-- = ItoX((d & 8) ? 1 : 0);
				}
				while(pb[1] == '0')
					pb++;
				if(m_neg)
					*pb = '-';
				else
					pb++;
				return pb;
				break;
			case 8:
			case 10:
				{
					BarbNumber* acc = new BarbNumber("0", radix, ID_OPR_EQUAL);
					BarbNumber* mul = new BarbNumber("1", radix, ID_OPR_EQUAL);
					BarbNumber* hex = new BarbNumber(radix == 10 ? "16" : "20", radix, ID_OPR_EQUAL);
					BarbNumber* nacc, *nmul;

					for(i = m_len - 1; i >= 0; i--)
					{
						if((j = XtoI(m_val[i])) != 0)
						{
							const char* vs;
							BarbNumber* val;
							
							switch(radix)
							{
							case 8:
								switch(j)
								{
								case 1: vs = "1"; break;
								case 2: vs = "2"; break;
								case 3: vs = "3"; break;
								case 4: vs = "4"; break;
								case 5: vs = "5"; break;
								case 6: vs = "6"; break;
								case 7: vs = "7"; break;
								case 8: vs = "10"; break;
								case 9: vs = "11"; break;
								case 10: vs = "12"; break;
								case 11: vs = "13"; break;
								case 12: vs = "14"; break;
								case 13: vs = "15"; break;
								case 14: vs = "16"; break;
								case 15: vs = "17"; break;
								}
								break;
							case 10:
								switch(j)
								{
								case 1: vs = "1"; break;
								case 2: vs = "2"; break;
								case 3: vs = "3"; break;
								case 4: vs = "4"; break;
								case 5: vs = "5"; break;
								case 6: vs = "6"; break;
								case 7: vs = "7"; break;
								case 8: vs = "8"; break;
								case 9: vs = "9"; break;
								case 10: vs = "10"; break;
								case 11: vs = "11"; break;
								case 12: vs = "12"; break;
								case 13: vs = "13"; break;
								case 14: vs = "14"; break;
								case 15: vs = "15"; break;
								}
								break;
							}
							val = new BarbNumber(vs, radix, ID_OPR_EQUAL);
							nmul = (BarbNumber*)val->Mul(val, mul, ec);
							delete val;
							nacc = (BarbNumber*)acc->Add(acc, nmul, ec);
							delete acc;
							delete nmul;
							acc = nacc;
						}
						nmul = (BarbNumber*)mul->Mul(mul, hex, ec);
						delete mul;
						mul = nmul;
					}
					memcpy(pb, acc->m_val, acc->m_len);
					pb[acc->m_len] = '\0';
					delete acc;
					delete mul;
				}
				break;
			case 16:
				memcpy(pb, m_val, m_len);
				pb[m_len] = 0;
				break;
			}
			break;
		}
	}
	else
	{
		*pb++ = '0';
		*pb = 0;
	}
	return m_rb;
}

//**************************************************************************
bool BarbNumber::IsNegative()
{
	return m_neg;
}

//**************************************************************************
bool BarbNumber::IsZero()
{
	return m_len == 0;
}

//**************************************************************************
Bnumber* BarbNumber::Negate(Bnumber* pn, ERRCODE& ec)
{
	BarbNumber* pin = (BarbNumber*)pn;
	BarbNumber* pout= new BarbNumber(pin, ID_OPR_EQUAL);

	pout->m_neg = ! pin->m_neg;
	ec = errOK;
	return pout;
}

//**************************************************************************
Bnumber* BarbNumber::Invert(Bnumber* pn, ERRCODE& ec)
{
	BarbNumber* pin = (BarbNumber*)pn;

	if(pin->m_val == 0)
	{
		ec = errFAILURE;
		return NULL;
	}
	ec = errOK;
	return new BarbNumber(pin, ID_OPR_EQUAL);
}

//**************************************************************************
Bnumber* BarbNumber::Add(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BarbNumber* pina = (BarbNumber*)pna;
	BarbNumber* pinb = (BarbNumber*)pnb;
	BarbNumber* pinr = NULL;

	LPSTR pa, pb, pr, rb;
	int   la, lb;
	int   da, db, dc, dr;

	if(! pina->m_neg && pinb->m_neg)
	{
		BarbNumber* pinc = new BarbNumber(pinb, ID_OPR_EQUAL);

		pinc->m_neg = false;
		pinr = (BarbNumber*)Sub(pinc, pina, ec);
		delete pinc;
		return pinr;
	}
	if(pina->m_neg && ! pinb->m_neg)
	{
		BarbNumber* pinc = new BarbNumber(pina, ID_OPR_EQUAL);

		pinc->m_neg = false;
		pinr = (BarbNumber*)Sub(pinc, pnb, ec);
		delete pinc;
		return pinr;
	}
	la = pina->m_len;
	lb = pinb->m_len;
	pa = pina->m_val + la - 1;
	pb = pinb->m_val + lb - 1;
	if(la == 0) 
		return new BarbNumber(pinb, ID_OPR_EQUAL);
	if(lb == 0) 
		return new BarbNumber(pina, ID_OPR_EQUAL);
	rb = new CHAR [ max(la, lb) + 4 ];
	pr = rb + max(la, lb) + 3;
	*pr-- = 0;
	dc = 0;
	while(la > 0 || lb > 0 || dc != 0) 
	{
		da = la > 0 ? XtoI(*pa) : 0;
		db = lb > 0 ? XtoI(*pb) : 0;
		dr = da + db + dc;
		dc  = 0;
		while(dr >= m_radix)
		{
			dc++;
			dr -= m_radix;
		}
		*pr-- = ItoX(dr);
		la--;
		lb--;
		pa--;
		pb--;
	}
	for(pr++; *pr == '0';)
		pr++;
	pinr = new BarbNumber(pr, m_radix, ID_OPR_EQUAL);
	if(pina->m_neg && pinb->m_neg)
		pinr->m_neg = true;
	ec = errOK;
	delete rb;
	return pinr;
}

//**************************************************************************
Bnumber* BarbNumber::Sub(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BarbNumber* pina = (BarbNumber*)pna;
	BarbNumber* pinb = (BarbNumber*)pnb;
	BarbNumber* pinr = NULL;

	LPSTR pa, pb, pr, rb;
	int   la, lb;
	int   da, db, dc, dr;

	if(! pina->m_neg && pinb->m_neg)
	{
		BarbNumber* pinc = new BarbNumber(pinb, ID_OPR_EQUAL);

		pinc->m_neg = false;
		pinr = (BarbNumber*)Add(pna, pinc, ec);
		pinr->m_neg = true;
		delete pinc;
		return pinr;
	}
	if(pina->m_neg && ! pinb->m_neg)
	{
		BarbNumber* pinc = new BarbNumber(pina, ID_OPR_EQUAL);

		pinc->m_neg = false;
		pinr = (BarbNumber*)Add(pinc, pinb, ec);
		delete pinc;
		return pinr;
	}
	la = pina->m_len;
	lb = pinb->m_len;
	pa = pina->m_val + la - 1;
	pb = pinb->m_val + lb - 1;
	if(la == 0) 
	{
		pinr  = new BarbNumber(pinb, ID_OPR_EQUAL);
		return pinr;
	}
	if(lb == 0)
	{
		pinr  = new BarbNumber(pina, ID_OPR_EQUAL);
		if(pinr->m_len > 0) pinr->m_neg = ! pinb->m_neg;
		else                pinr->m_neg = false;
		return pinr;
	}

	if(Compare(pina, pinb) > 0)
	{
		pinr = (BarbNumber*)Sub(pnb, pna, ec);
		if(pinr->m_len > 0) pinr->m_neg = ! pinr->m_neg;
		else                pinr->m_neg = false;
		return pinr;
	}
	rb = new CHAR [ max(la, lb) + 4 ];
	pr = rb + max(la, lb) + 3;
	*pr-- = 0;
	dc = 0;
	while(lb > 0) 
	{
		da = la > 0 ? XtoI(*pa) : 0;
		db = XtoI(*pb) - (dc ? 1 : 0);
		dc = (db < da) ? m_radix : 0; 
		dr = (dc + db) - da;
		*pr-- = ItoX(dr);
		la--;
		lb--;
		pa--;
		pb--;
	}
	for(pr++; *pr == '0';)
		pr++;
	pinr = new BarbNumber(pr, m_radix, ID_OPR_EQUAL);
	pinr->m_neg = pinr->m_len > 0 ? pinb->m_neg : false;
	ec = errOK;
	delete rb;
	return pinr;
}

//**************************************************************************
Bnumber* BarbNumber::Mul(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BarbNumber* pina = (BarbNumber*)pna;
	BarbNumber* pinb = (BarbNumber*)pnb;
	BarbNumber* pinx, *piny;
	int			dc, dr;
	
	LPSTR		pn, rb;
	int			i, j;

	if(pina->m_len < pinb->m_len)
		return Mul(pnb, pna, ec);

	BarbNumber* pinr = new BarbNumber("0", m_radix, ID_OPR_EQUAL);

	rb = new CHAR [ pina->m_len + pinb->m_len + 2 ];
	
	for(i = pinb->m_len - 1; i >= 0; i--)
	{
		pn  = rb + pina->m_len + pinb->m_len + 1;
		*pn = 0;
		for(dc = 0; dc < (pinb->m_len - 1 - i); dc++)
			*--pn = '0';
		dc = 0;

		for(j = pina->m_len - 1; j >= 0; j--)
		{
			dr = XtoI(pina->m_val[j]) * XtoI(pinb->m_val[i]) + dc;
			dc = 0;
			while(dr >= m_radix)
			{
				dc++;
				dr -= m_radix;
			}
			*--pn = ItoX(dr);
		}
		if(dc)
			*--pn = ItoX(dc);
		pinx = new BarbNumber(pn, m_radix, ID_OPR_EQUAL);
		piny = (BarbNumber*)Add(pinr, pinx, ec);
		delete pinx;
		delete pinr;
		pinr = piny;
	}
	pinr->m_neg = pina->m_neg ^ pinb->m_neg;
	ec = errOK;
	delete rb;
	return pinr;
}

//**************************************************************************
Bnumber* BarbNumber::Div(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BarbNumber* pina = (BarbNumber*)pna;
	BarbNumber* pinb = (BarbNumber*)pnb;
	BarbNumber* pnum, *pnnum;
	BarbNumber* pdem, *pndem;
	BarbNumber* sum,  *nsum, *psum;
	BarbNumber* pres, *pnres, *pnx;
	int			alen, d;
	LPSTR		rb, pn, db;
	int			i;
	int			r;

	// do "b / a"
	//
	if(pina->m_len == 0)
	{
		// divide by 0
		ec = errFAILURE;
		return NULL;
	}
	pnum = new BarbNumber(pinb, ID_OPR_EQUAL);
	pnum->m_neg = false;

	pres = new BarbNumber("0", m_radix, ID_OPR_EQUAL);

	// this makes a number the same length as pinb
	// containing pina left justified
	//
	db	 = new CHAR [ pnum->m_len + 32 ];
	memset(db, '0', pinb->m_len + 32);
	memcpy(db, pina->m_val, pina->m_len);
	db[pinb->m_len] = 0;
	alen = pnum->m_len;
	db[alen] = 0;
	pdem = new BarbNumber(db, m_radix, ID_OPR_EQUAL);

	// position of digit in result
	r    = pnum->m_len - pina->m_len + 1;

	// make sure dem < num
	if((Compare(pnum, pdem) < 0) && (alen > pina->m_len))
	{
		alen--;
		r--;
		db[alen] = 0;
		delete pdem;
		pdem = new BarbNumber(db, m_radix, ID_OPR_EQUAL);
	}

	while(Compare(pnum, pdem) >= 0)
	{
		// denominator will now divide numerator from one to radix - 1 times
		//
		pdem = new BarbNumber(db, m_radix, ID_OPR_EQUAL);
		sum  = new BarbNumber("0", m_radix, ID_OPR_EQUAL);
		d    = 0;
		rb = new CHAR [ r + 32 ];
		memset(rb, '0', r);
		rb[r] = 0;
		i     = 0;
		pn    = rb;
		psum  = NULL;

		while(Compare(sum, pnum) <= 0)
		{
			nsum = (BarbNumber*)Add(sum, pdem, ec);
			if(psum) delete psum;
			psum = sum;
			sum  = nsum;
			d++;
		}
		delete sum;
		if(d > 0)
		{
			d--;
			pn[i++] = ItoX(d);
			pnnum = (BarbNumber*)Sub(psum, pnum, ec);
			delete pnum;
			delete psum;
			pnum = pnnum;
			pnx = new BarbNumber(pn, m_radix, ID_OPR_EQUAL);
			pnres = (BarbNumber*)Add(pnx, pres, ec);
			delete pnx;
			delete pres;
			pres = pnres;
		}
		delete [] rb;
		while(Compare(pnum, pdem) < 0)
		{
			if(alen <= pina->m_len || r < 0)
				break;
			i++;
			alen--;
			r--;
			db[alen] = 0;
			pndem = new BarbNumber(db, m_radix, ID_OPR_EQUAL);
			delete pdem;
			pdem = pndem;
		}
	}
	delete [] db;
	delete pnum;
	delete pdem;
	ec = errOK;
	pres->m_neg = pina->m_neg ^ pinb->m_neg;
	return pres;
}

//**************************************************************************
Bnumber* BarbNumber::XtoY(Bnumber* pna, Bnumber* pnb, ERRCODE& ec)
{
	BarbNumber* pina = (BarbNumber*)pna;
	BarbNumber* pinb = (BarbNumber*)pnb;

	if(pina->m_neg) return new BarbNumber("0", m_radix, ID_OPR_EQUAL);

	BarbNumber* y   = new BarbNumber(pina, ID_OPR_EQUAL);
	BarbNumber* one = new BarbNumber("1", m_radix, ID_OPR_EQUAL);
	BarbNumber* ny;

	BarbNumber* acc = new BarbNumber("1", m_radix, ID_OPR_EQUAL);
	BarbNumber* nacc;

	while(y->m_len)
	{
		nacc = (BarbNumber*)acc->Mul(acc, pinb, ec);
		delete acc;
		acc = nacc;

		ny = (BarbNumber*)y->Sub(one, y, ec);
		delete y;
		y = ny;
	}
	delete y;
	delete one;
	return acc;
}
