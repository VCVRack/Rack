#pragma once

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

typedef void LOGCALLBACK ( void *pClass, char *str );

//-------------------------------------------------------------
//-------------------------------------------------------------
class CLog 
{
public:
	CLog();
	~CLog();
    
    LOGCALLBACK     *m_LogCallbackFunc;
    void            *m_pCallbackClass;

    int             m_LogLevel;

    void SetCallback ( LOGCALLBACK *func, void *pClass );
	void f           ( std::string string, ...);
	void fnr         ( std::string string, ...);
	void mem         ( unsigned char *pBuff, unsigned int dwSize, unsigned int dwOff );

	int	 Open        ( std::string strFileName );
    void SetLogLvl   ( int level );
	void Close       ( void );

private:
	FILE	*fp;
    std::string buffer;
};


