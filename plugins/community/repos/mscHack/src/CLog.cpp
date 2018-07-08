#include "rack.hpp"
#include "CLog.h"

char Days[7][4]=
{
	{"Sun"},
	{"Mon"},
	{"Tue"},
	{"Wed"},
	{"Thu"},
	{"Fri"},
	{"Sat"}
};

char Months[12][4]=
{
	{"Jan"},
	{"Feb"},
	{"Mar"},
	{"Apr"},
	{"May"},
	{"Jun"},
	{"Jul"},
	{"Aug"},
	{"Sep"},
	{"Oct"},
	{"Nov"},
	{"Dec"}
};

//-----------------------------------------------------
// Function:				
//
//-----------------------------------------------------
CLog::CLog()
{
    m_LogLevel = 1;
    fp = NULL;
    m_LogCallbackFunc = NULL;
}

//-----------------------------------------------------
// Function:				
//
//-----------------------------------------------------
CLog::~CLog()
{
    Close();
}

//-----------------------------------------------------
// Function:				
//
//-----------------------------------------------------
int CLog::Open( std::string strFileName )
{
	// test if this file is valid
	if ( (fp = fopen( strFileName.c_str(), "w+" ) ) == NULL )
	    return 0;

	// return success
	return 1;
} 

//-----------------------------------------------------
// Function:				
//
//-----------------------------------------------------
void CLog::Close(void)
{
	// if file not open then bail
	if (fp)
	{
		// close the file handle
		fclose(fp);
		fp = NULL;
	}
}

//-----------------------------------------------------
// Function:	SetLogLvl			
//
//-----------------------------------------------------
void CLog::SetLogLvl( int level )
{
    m_LogLevel = level;
}

//-----------------------------------------------------
// Function:	SetCallback			
//
//-----------------------------------------------------
void CLog::SetCallback ( LOGCALLBACK *func, void *pClass )
{
    m_LogCallbackFunc = func;
    m_pCallbackClass  = pClass;
}

void CLog::f( std::string string, ... )
{
	// if file not open then bail
	if (!fp)
	   return;

    //_Xtime_get_ticks(

    va_list arglist; // variable argument list

	// print out the string using the variable number of arguments on stack
	va_start( arglist, string );
	vsprintf( (char*)buffer.c_str(), string.c_str(), arglist );
	va_end( arglist );

	// write string to file
	fprintf( fp,(char*)buffer.c_str() );

	// write string to callback so user can display it somewhere
    if( m_LogCallbackFunc )
		m_LogCallbackFunc( m_pCallbackClass, (char*)buffer.c_str() );

	fflush(fp);
}

//-----------------------------------------------------
// Function:    no timestamp	
//
//-----------------------------------------------------
void CLog::fnr( std::string string, ...)
{
	
	va_list arglist; // variable argument list

	// if file not open then bail
	if (!fp)
	   return;

	// print out the string using the variable number of arguments on stack
	va_start( arglist,string );
	vsprintf( (char*)buffer.c_str(), string.c_str(),arglist );
	va_end( arglist );

	// write string to file
	fprintf( fp, (char*)buffer.c_str() );

	// write string to callback so user can display it somewhere
    if( m_LogCallbackFunc )
		m_LogCallbackFunc( m_pCallbackClass, (char*)buffer.c_str() );

	fflush(fp);
}

//-----------------------------------------------------
// Function: mem - memory dump			
//
//-----------------------------------------------------
void CLog::mem( unsigned char *pBuff, unsigned int dwSize, unsigned int dwOff )
{
	int i;
	unsigned int Offset=dwOff, index=0;
	unsigned char Val[16];

	// if file not open then bail
	if (!fp)
	   return;

	do{
		f("0x%.4x: ", dwOff);
		
		Offset+=16;
		dwOff+=16;
		
		for(i=0; i<16; i++)
		{
			// extra space every four
			if(i%4 == 0)
				f(" ");

			if( index < dwSize )
			{
				Val[i]=pBuff[index];
				f("%.2x ", pBuff[index++]);
			}
			else
			{
				Val[i]=0;
				f("-- ", pBuff[index++]);
			}
		}

		// print characters
		for(i=0; i<16; i++)
		{
			if( isprint( Val[i] ) )
				f("%c", Val[i]);
			else
				f(".");
		}

		f("\n");

	}while(index < dwSize);

	f("\n\n");
}