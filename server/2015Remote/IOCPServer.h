#pragma once

#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include "CpuUseage.h"
#include "Buffer.h"
#if USING_CTX
#include "zstd/zstd.h"
#endif

#include <Mstcpip.h>
#define PACKET_LENGTH   0x2000

#define FLAG_LENGTH   5
#define HDR_LENGTH   13

#define	NC_CLIENT_CONNECT		0x0001
#define	NC_RECEIVE				0x0004
#define NC_RECEIVE_COMPLETE		0x0005 // 完整接收

std::string GetRemoteIP(SOCKET sock);

enum IOType 
{
	IOInitialize,
	IORead,
	IOWrite,   
	IOIdle
};

enum {
	COMPRESS_UNKNOWN		= -2,			// 未知压缩算法
	COMPRESS_ZLIB			= -1,			// 以前版本使用的压缩方法
	COMPRESS_ZSTD			= 0,			// 当前使用的压缩方法
};

typedef struct _CONTEXT_OBJECT 
{
	CString  sClientInfo[10];
	SOCKET   sClientSocket;
	WSABUF   wsaInBuf;
	WSABUF	 wsaOutBuffer;  
	char     szBuffer[PACKET_LENGTH];
	CBuffer				InCompressedBuffer;	        // 接收到的压缩的数据
	CBuffer				InDeCompressedBuffer;	    // 解压后的数据
	CBuffer             OutCompressedBuffer;
	int				    v1;
	HANDLE              hDlg;
	void				*olps;						// OVERLAPPEDPLUS
	int					CompressMethod;				// 压缩算法

	VOID InitMember()
	{
		memset(szBuffer,0,sizeof(char)*PACKET_LENGTH);
		v1 = 0;
		hDlg = NULL;
		sClientSocket = INVALID_SOCKET;
		memset(&wsaInBuf,0,sizeof(WSABUF));
		memset(&wsaOutBuffer,0,sizeof(WSABUF));
		olps = NULL;
		CompressMethod = COMPRESS_ZSTD;
	}
	VOID SetClientInfo(CString s[10]){
		for (int i=0; i<sizeof(sClientInfo)/sizeof(CString);i++)
		{
			sClientInfo[i] = s[i];
		}
	}
	CString GetClientData(int index) const{
		return sClientInfo[index];
	}
}CONTEXT_OBJECT,*PCONTEXT_OBJECT;

typedef CList<PCONTEXT_OBJECT> ContextObjectList;

class IOCPServer
{
public:
	SOCKET    m_sListenSocket;
	HANDLE    m_hCompletionPort;
	UINT      m_ulMaxConnections;          // 最大连接数
	HANDLE    m_hListenEvent;
	HANDLE    m_hListenThread;
	BOOL      m_bTimeToKill;
	HANDLE    m_hKillEvent;

	ULONG m_ulThreadPoolMin;
	ULONG m_ulThreadPoolMax;
	ULONG m_ulCPULowThreadsHold; 
	ULONG m_ulCPUHighThreadsHold;
	ULONG m_ulCurrentThread;
	ULONG m_ulBusyThread;

#if USING_CTX
	ZSTD_CCtx* m_Cctx; // 压缩上下文
	ZSTD_DCtx* m_Dctx; // 解压上下文
#endif

	CCpuUsage m_cpu;

	ULONG  m_ulKeepLiveTime;

	char   m_szPacketFlag[FLAG_LENGTH + 3];

	typedef void (CALLBACK *pfnNotifyProc)(CONTEXT_OBJECT* ContextObject);
	typedef void (CALLBACK *pfnOfflineProc)(CONTEXT_OBJECT* ContextObject);
	UINT StartServer(pfnNotifyProc NotifyProc, pfnOfflineProc OffProc, USHORT uPort);

	static DWORD WINAPI ListenThreadProc(LPVOID lParam);
	BOOL InitializeIOCP(VOID);
	static DWORD WINAPI WorkThreadProc(LPVOID lParam);
	ULONG   m_ulWorkThreadCount;
	VOID OnAccept();
	static CRITICAL_SECTION	m_cs;

	/************************************************************************/
	//上下背景文对象
	ContextObjectList				m_ContextConnectionList;
	ContextObjectList               m_ContextFreePoolList;
	PCONTEXT_OBJECT AllocateContext();
	VOID RemoveStaleContext(CONTEXT_OBJECT* ContextObject);
	VOID MoveContextToFreePoolList(CONTEXT_OBJECT* ContextObject);

	VOID PostRecv(CONTEXT_OBJECT* ContextObject);

	VOID ExitWorkThread() { EnterCriticalSection(&m_cs); --m_ulWorkThreadCount; LeaveCriticalSection(&m_cs); }

	/************************************************************************/
	//请求得到完成
	BOOL HandleIO(IOType PacketFlags,PCONTEXT_OBJECT ContextObject, DWORD dwTrans);
	BOOL OnClientInitializing(PCONTEXT_OBJECT  ContextObject, DWORD dwTrans);
	BOOL OnClientReceiving(PCONTEXT_OBJECT  ContextObject, DWORD dwTrans);  
	VOID OnClientPreSending(CONTEXT_OBJECT* ContextObject, PBYTE szBuffer , size_t ulOriginalLength);
	VOID Send(CONTEXT_OBJECT* ContextObject, PBYTE szBuffer, ULONG ulOriginalLength) {
		OnClientPreSending(ContextObject, szBuffer, ulOriginalLength);
	}
	BOOL OnClientPostSending(CONTEXT_OBJECT* ContextObject,ULONG ulCompressedLength);
	void UpdateMaxConnection(int maxConn);
	IOCPServer(void);
	~IOCPServer(void);

	pfnNotifyProc m_NotifyProc;
	pfnOfflineProc m_OfflineProc;
};

class CLock     
{
public:
	CLock(CRITICAL_SECTION& cs)
	{
		m_cs = &cs;
		Lock();
	}
	~CLock()
	{
		Unlock();
	}

	void Unlock()
	{
		LeaveCriticalSection(m_cs);
	}

	void Lock()
	{
		EnterCriticalSection(m_cs);
	}

protected:
	CRITICAL_SECTION*	m_cs;
};

#define TRACK_OVERLAPPEDPLUS 0

class OVERLAPPEDPLUS   
{
public:

	OVERLAPPED			m_ol;    
	IOType				m_ioType;

	OVERLAPPEDPLUS(IOType ioType)
	{
#if TRACK_OVERLAPPEDPLUS
		char szLog[100];
		sprintf_s(szLog, "=> [new] OVERLAPPEDPLUS %p by thread [%d].\n", this, GetCurrentThreadId());
		OutputDebugStringA(szLog);
#endif
		ZeroMemory(this, sizeof(OVERLAPPEDPLUS));
		m_ioType = ioType;
	}

	~OVERLAPPEDPLUS()
	{
#if TRACK_OVERLAPPEDPLUS
		char szLog[100];
		sprintf_s(szLog, "=> [delete] OVERLAPPEDPLUS %p by thread [%d].\n", this, GetCurrentThreadId());
		OutputDebugStringA(szLog);
#endif
	}
};

typedef IOCPServer CIOCPServer;

typedef CONTEXT_OBJECT ClientContext;

#define m_Socket sClientSocket
#define m_DeCompressionBuffer InDeCompressedBuffer
