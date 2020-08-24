#ifndef LANGPARSE_H
#define LANGPARSE_H 1

// an object that can enumerate a language 
//**************************************************************************
class BlanguageParser
{
public:
	BlanguageParser(void) { };
public:
	virtual ERRCODE EnumFunctions	(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog = NULL) { return errOK; }
};

#endif
