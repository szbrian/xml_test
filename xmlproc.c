#include <iconv.h>
#include <errno.h>
#include "xmlproc.h"

int main(int argc, char **argv)
{
	char szTradeCode[11]={0}, szReturnCode[20]={0}, szReturnMessage[50]={0};
	char szXmlNode[50] = {0}, szCustNo[20]={0}, szCustName[100]={0};
	char szXmlBuff[1024] = {0};

	CreateXML( szTradeCode, "12345", "00000001", szXmlBuff );
	printf( szXmlBuff );

	FILE *fp = NULL;

	fp = fopen( "request.xml", "r" );

	memset( szXmlBuff, 0x00, sizeof(szXmlBuff) );
	fread( szXmlBuff, 1, sizeof(szXmlBuff)-1, fp );
	fclose( fp );

	TransXML( szXmlBuff, "ACMT000080", szReturnCode, szXmlNode, szReturnMessage, szCustNo, szCustName );

	printf( "szTradeCode = %s\n", szTradeCode );
	printf( "szReturnCode = %s\n", szReturnCode );
	printf( "szReturnMessage = %s\n", szReturnMessage );
	printf( "szCustNo = %s\n", szCustNo );
	printf( "szCustName = %s\n", szCustName );

	return(0);
}

BOOL TransXML(const char* szXmlbuf, const char *szTradeCode, char* szReturnCode, char* szXmlNode, char* szReturnMessage, char *szCustNo, char *szCustName )
{
	xmlDocPtr pXmlDoc = NULL;
	xmlNodePtr pRoot_node = NULL, pHead_node = NULL, pBody_node = NULL, pCur_node = NULL, pResp_node = NULL;
	xmlChar *pXmlChar = NULL;

	//pXmlDoc = xmlParseMemory( szXmlbuf, strlen(szXmlbuf) );

	pXmlDoc = xmlReadMemory( szXmlbuf, strlen(szXmlbuf), NULL, "GBK", XML_PARSE_RECOVER );

	if ( pXmlDoc == NULL )
	{
		return FALSE;
	}
	pRoot_node = xmlDocGetRootElement( pXmlDoc ); 
	if ( pRoot_node == NULL )
	{
		xmlFreeDoc(pXmlDoc);
		return FALSE;
	}
	if ( xmlStrcmp(pRoot_node->name, (xmlChar*)"Service") != 0  )
	{
		xmlFreeDoc(pXmlDoc);
		xmlCleanupParser();
		xmlMemoryDump(); 
		return FALSE;
	}

	// 读取xml文件中的Header和Body节点
	pCur_node = pRoot_node->children;
	while ( pCur_node != NULL )
	{
		if ( xmlStrcmp(pCur_node->name, (xmlChar*)"Header") == 0 )
		{
			pHead_node = pCur_node;
		}
		else if ( xmlStrcmp(pCur_node->name, (xmlChar*)"Body") == 0 )
		{
			pBody_node = pCur_node;
		}

		//printf( "pCur_node name = %s\n", pCur_node->name );
		pCur_node = pCur_node->next;
	}

	if ( pHead_node == NULL || pBody_node == NULL )
	{
		xmlFreeDoc(pXmlDoc);
		xmlCleanupParser();
		xmlMemoryDump(); 
		return FALSE;
	}

	// 读取header节点下的Response子节点
	pCur_node = pHead_node->children;
	while ( pCur_node != NULL )
	{
		if ( xmlStrcmp(pCur_node->name, (xmlChar*)"Response") == 0 )
		{
			pResp_node = pCur_node;
		}
		//printf( "pCur_node name = %s\n", pCur_node->name );
		pCur_node = pCur_node->next;
	}

	pCur_node = pResp_node->children;
	strcpy( szReturnCode, "FF" );
	while ( pCur_node != NULL )
	{
		if ( xmlStrcmp(pCur_node->name, (xmlChar*)"ReturnCode") == 0 )
		{
			pXmlChar = xmlNodeGetContent( pCur_node );
			//printf( "ReturnCode = %s\n", pXmlChar );
			strcpy( szReturnCode, (const char*)pXmlChar );
			xmlFree( pXmlChar );
			pXmlChar = NULL;
		}
		else if ( xmlStrcmp(pCur_node->name, (xmlChar*)"ReturnMessage") == 0 )
		{
			pXmlChar = xmlNodeGetContent( pCur_node );
			if ( *pXmlChar != 0x00 )
			{
				char *pChar = u2g( (char*)pXmlChar );
				if ( pChar != NULL )
				{
					strcpy( szReturnMessage, (const char*)pChar );
				}
				xmlFree( pChar );
			}
			xmlFree( pXmlChar );
			pXmlChar = NULL;
		}
		pCur_node = pCur_node->next;
	}

	// 查询账号信息，需要取得客户号和客户名称
	int nRet;
	sscanf( szReturnCode, "%d", &nRet );
	if ( nRet == 0 && memcmp( szTradeCode, "ACMT000080", 10 ) == 0 )
	{
		// 读取body节点下的Response子节点
		pCur_node = pBody_node->children;
		while ( pCur_node != NULL )
		{
			if ( xmlStrcmp(pCur_node->name, (xmlChar*)"Response") == 0 )
			{
				pResp_node = pCur_node;	
			}

			pCur_node = pCur_node->next;
		}
		pCur_node = pResp_node->children;
		while ( pCur_node != NULL )
		{
			if ( xmlStrcmp(pCur_node->name, (xmlChar*)"CustNo") == 0 )
			{
				pXmlChar = xmlNodeGetContent( pCur_node );
				//printf( "CustNo = %s\n", pXmlChar );
				strcpy( szCustNo, (const char*)pXmlChar );
				xmlFree( pXmlChar );
				pXmlChar = NULL;
			}
			//else if ( xmlStrcmp(pCur_node->name, (xmlChar*)"CustName35") == 0)	// modified by brian 20130701
			else if ( xmlStrcmp(pCur_node->name, (xmlChar*)"AcctName") == 0)	// end modified 20130701
			{
				pXmlChar = xmlNodeGetContent( pCur_node );
				char *pChar = u2g( (char*)pXmlChar );
				//printf( "AcctName(gb2312) = %s\n", pChar );
				if ( pChar != NULL )
				{
					strcpy( szCustName, (const char*)pChar );
				}
				xmlFree( pChar );				
				xmlFree( pXmlChar );
				pXmlChar = NULL;
			}
			else if ( xmlStrcmp(pCur_node->name, (xmlChar*)"OpenBranchCode") == 0)
			{
				pXmlChar = xmlNodeGetContent( pCur_node );
				strcpy( szXmlNode, (const char*)pXmlChar );
				xmlFree( pXmlChar );
				pXmlChar = NULL;
			}
			pCur_node = pCur_node->next;
		}
	}
	
	xmlFreeDoc(pXmlDoc);
	xmlCleanupParser();
	xmlMemoryDump(); 
	return TRUE;
}
 
void CreateXML( const char *szTradeCode, const char *szAcc, const char *szNode, char *pszXmlBuf )
{
	char szDate[9]={0}, szDateTime[15]={0};
	char *pszName = NULL;
	xmlDocPtr pXmlDoc = NULL;
	xmlNodePtr pRoot_node = NULL, pHead_node = NULL, pBody_node = NULL, pTmpNode = NULL;

	memcpy( szDate, "20191018", 8 );

	pXmlDoc = xmlNewDoc((xmlChar*)"1.0");
	pRoot_node = xmlNewNode(NULL, (xmlChar*)"Service");
	xmlDocSetRootElement(pXmlDoc, pRoot_node);

	pHead_node = xmlNewNode( pRoot_node->ns, (xmlChar*)"Header" );
	xmlAddChild(pRoot_node, pHead_node );
	xmlNewChild(pHead_node, NULL, (xmlChar*)"ServiceCode", (xmlChar*)szTradeCode);  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"RequestTime", (xmlChar*)szDateTime);  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"Version", (xmlChar*)"1.0");  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"RequestType", (xmlChar*)"0");  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"Encrypt", (xmlChar*)"0");  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"RequestOperatorId", (xmlChar*)"FB.PCS.01");  
	xmlNewChild(pHead_node, NULL, (xmlChar*)"RequestOperatorType", (xmlChar*)"0");  

	char szChina[200] = {"中国"};
	pszName = g2u( szChina );
	if ( pszName != NULL )
	{
	    xmlNewChild(pHead_node, NULL, (xmlChar*)"CountryUTF", (xmlChar*)pszName );
	    free( pszName );
	}
	
	pBody_node = xmlNewNode( pRoot_node->ns, (xmlChar*)"Body" );
	xmlAddChild(pRoot_node, pBody_node );
	pTmpNode = xmlNewNode( pBody_node->ns, (xmlChar*)"Request" );
	xmlAddChild( pBody_node, pTmpNode );
	xmlNewChild(pTmpNode, NULL, (xmlChar*)"TransactionId19", (xmlChar*)szAcc); 

	if ( xmlStrcmp((xmlChar*)szTradeCode, (xmlChar*)"ACMT000080") == 0 )
	{
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"Pwd18", (xmlChar *)""); 
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"LPinpadId", (xmlChar*)""); 
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"MediumAcctNo", (xmlChar*)""); 
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"Currency", (xmlChar*)""); 
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"FcyType10", (xmlChar*)""); 
	}
	else if ( xmlStrcmp((xmlChar*)szTradeCode, (xmlChar*)"ACMT000120") == 0 )
	{
		xmlNewChild(pTmpNode, NULL, (xmlChar*)"PayPwdFlag", (xmlChar*)"Y"); 
	}

	xmlChar *pszOutbuf;
	int nOutLen;
	//xmlDocDumpFormatMemoryEnc( pXmlDoc, &pszOutbuf, &nOutLen, "UTF-8", 1 );
	xmlDocDumpFormatMemoryEnc( pXmlDoc, &pszOutbuf, &nOutLen, "GBK", 1 );
	memcpy( pszXmlBuf, pszOutbuf, nOutLen );

	xmlSaveFormatFileEnc( "request_utf.xml", pXmlDoc, "UTF-8", 1 );
	xmlSaveFormatFileEnc( "request_gbk.xml", pXmlDoc, "GBK", 1 );
	
	xmlFree( pszOutbuf );
	xmlFreeDoc(pXmlDoc);
	xmlCleanupParser();
	xmlMemoryDump(); 

	return;
}


//代码转换:从一种编码转为另一种编码

int code_convert( const char* from_charset, const char* to_charset, char* inbuf,
               size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char** pin = &inbuf;  
    char** pout = &outbuf;
    size_t nLeft = inlen;
    cd = iconv_open(to_charset,from_charset);  
    if(cd == 0)
       return -1;
    memset(outbuf,0,outlen);  
    if( iconv(cd, pin, (size_t *)&nLeft, pout, (size_t*)&outlen) == -1)
    {
       printf( "iconv() return error, errno = %d", errno );
       return -1;  
    }
    iconv_close(cd);
    return 0;  
}

//UNICODE码转为gbk码  

//成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL
char* u2g(char *inbuf)  
{
	size_t nOutLen = 2 * strlen(inbuf) - 1;

	if ( nOutLen <= 0 )
	{
		return NULL;
	}
	char* szOut = (char*)malloc(nOutLen);
	int nRet = 0;
	nRet = code_convert( "utf-8", "gbk", inbuf, strlen(inbuf), szOut, nOutLen );
	return szOut;
}  

//gbk码转为UNICODE码

//成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL

char* g2u(char *inbuf)  
{
	size_t nOutLen = 2 * strlen(inbuf) - 1;

	if ( nOutLen <= 0 )
	{
		return NULL;
	}
	char* szOut = (char*)malloc(nOutLen);
	int nRet = 0;
	size_t inlen = strlen(inbuf);
	nRet = code_convert( "gbk", "utf-8", inbuf, inlen, szOut, nOutLen );
	return szOut;
}
/* 

用下面的命令编译通过。
1、安装libxml2-sources-2.9.0.tar.gz
2、设置LD_LIBRARY_PATH=/usr/local/lib
3、gcc -I /usr/local/include/libxml2  -L /usr/local/lib -lxml2 test.c -o test

或使用makefile，其内容如下
CFLAGS=`xml2-config --cflags`
LIBS=`xml2-config --libs`
all:test
test: test.o
        cc $(CFLAGS) -o test test.o $(LIBS)
.c.o:
        cc $(CFLAGS) -c $*.c

*/

