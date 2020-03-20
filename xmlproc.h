#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#define BOOL int
#endif

int code_convert( const char* from_charset, const char* to_charset, 
				 char* inbuf, size_t inlen, char* outbuf, size_t outlen);
char* u2g(char *inbuf);
char* g2u(char *inbuf);

void CreateXML( const char *szTradeCode, const char *szAcc, const char *szNode, char *pszXmlBuf );
BOOL TransXML(const char* szXmlbuf, const char *szTradeCode, char* szReturnCode, char* szXmlNode, char* szReturnMessage, char *szCustNo, char *szCustName );
