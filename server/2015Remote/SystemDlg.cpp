// SystemDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "2015Remote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"


// CSystemDlg 对话框

typedef struct ItemData
{
	DWORD ID;
	CString Data[3];
	CString GetData(int index)const {
		return Data[index];
	}
}ItemData;

IMPLEMENT_DYNAMIC(CSystemDlg, CDialog)

	CSystemDlg::CSystemDlg(CWnd* pParent, Server* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: DialogBase(CSystemDlg::IDD, pParent, IOCPServer, ContextObject, IDI_SERVICE)
{
	m_bHow= m_ContextObject->InDeCompressedBuffer.GetBYTE(0);
}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYSTEM, m_ControlList);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SYSTEM, &CSystemDlg::OnNMRClickListSystem)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CSystemDlg::OnHdnItemclickList)
	ON_COMMAND(ID_PLIST_KILL, &CSystemDlg::OnPlistKill)
	ON_COMMAND(ID_PLIST_REFRESH, &CSystemDlg::OnPlistRefresh)
	ON_COMMAND(ID_WLIST_REFRESH, &CSystemDlg::OnWlistRefresh)
	ON_COMMAND(ID_WLIST_CLOSE, &CSystemDlg::OnWlistClose)
	ON_COMMAND(ID_WLIST_HIDE, &CSystemDlg::OnWlistHide)
	ON_COMMAND(ID_WLIST_RECOVER, &CSystemDlg::OnWlistRecover)
	ON_COMMAND(ID_WLIST_MAX, &CSystemDlg::OnWlistMax)
	ON_COMMAND(ID_WLIST_MIN, &CSystemDlg::OnWlistMin)
END_MESSAGE_MAP()


// CSystemDlg 消息处理程序


BOOL CSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	CString str;
	m_bHow==TOKEN_PSLIST 
		? str.Format("%s - 进程管理", m_IPAddress)
		:str.Format("%s - 窗口管理", m_IPAddress);
	SetWindowText(str);//设置对话框标题

	m_ControlList.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT); 
	if (m_bHow==TOKEN_PSLIST)      //进程管理初始化列表
	{
		m_ControlList.InsertColumn(0, "映像名称", LVCFMT_LEFT, 180);
		m_ControlList.InsertColumn(1, "PID", LVCFMT_LEFT, 70);
		m_ControlList.InsertColumn(2, "程序路径", LVCFMT_LEFT, 320);
		ShowProcessList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中\0\0
	}else if (m_bHow==TOKEN_WSLIST)//窗口管理初始化列表
	{
		//初始化 窗口管理的列表
		m_ControlList.InsertColumn(0, "句柄", LVCFMT_LEFT, 80);
		m_ControlList.InsertColumn(1, "窗口名称", LVCFMT_LEFT, 420);
		m_ControlList.InsertColumn(2, "窗口状态", LVCFMT_LEFT, 200);
		ShowWindowsList();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSystemDlg::ShowWindowsList(void)
{
	Buffer tmp = m_ContextObject->InDeCompressedBuffer.GetMyBuffer(1);
	char *szBuffer = tmp.c_str();
	DWORD	dwOffset = 0;
	char	*szTitle = NULL;
	bool isDel=false;

	DeleteAllItems();
	CString	str;
	int i ;
	for ( i = 0; dwOffset <m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1; ++i)
	{
		LPDWORD	lpPID = LPDWORD(szBuffer + dwOffset);   //窗口句柄
		szTitle = (char *)szBuffer + dwOffset + sizeof(DWORD);   //窗口标题    
		str.Format("%5u", *lpPID);
		m_ControlList.InsertItem(i, str);
		m_ControlList.SetItemText(i, 1, szTitle);
		m_ControlList.SetItemText(i, 2, "显示"); //(d) 将窗口状态显示为 "显示"
		// ItemData 为窗口句柄
		auto data = new ItemData{ *lpPID, {str, szTitle,"显示"} };
		m_ControlList.SetItemData(i, (DWORD_PTR)data);  //(d)
		dwOffset += sizeof(DWORD) + lstrlen(szTitle) + 1;
	}
	str.Format("窗口名称    窗口个数【%d】", i);   //修改CtrlList 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(1, &lvc);
}

void CSystemDlg::ShowProcessList(void)
{
	Buffer tmp = m_ContextObject->InDeCompressedBuffer.GetMyBuffer(1);
	char	*szBuffer = tmp.c_str(); //xiaoxi[][][][][]
	char	*szExeFile;
	char	*szProcessFullPath;
	DWORD	dwOffset = 0;
	CString str;
	DeleteAllItems();       
	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0
	int i;
	for (i = 0; dwOffset < m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1; ++i)
	{
		LPDWORD	PID = LPDWORD(szBuffer + dwOffset);        //这里得到进程ID
		szExeFile = szBuffer + dwOffset + sizeof(DWORD);      //进程名就是ID之后的啦
		szProcessFullPath = szExeFile + lstrlen(szExeFile) + 1;  //完整名就是进程名之后的啦
		//他的数据结构的构建很巧妙

		m_ControlList.InsertItem(i, szExeFile);       //将得到的数据加入到列表当中
		str.Format("%5u", *PID);
		m_ControlList.SetItemText(i, 1, str);
		m_ControlList.SetItemText(i, 2, szProcessFullPath);
		// ItemData 为进程ID
		auto data = new ItemData{ *PID, {szExeFile, str, szProcessFullPath} };
		m_ControlList.SetItemData(i, DWORD_PTR(data));

		dwOffset += sizeof(DWORD) + lstrlen(szExeFile) + lstrlen(szProcessFullPath) + 2;   //跳过这个数据结构 进入下一个循环
	}

	str.Format("程序个数 / %d", i); 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(2, &lvc); //在列表中显示有多少个进程
}


void CSystemDlg::OnClose()
{
	CancelIO();
	// 等待数据处理完毕
	if (IsProcessing()) {
		ShowWindow(SW_HIDE);
		return;
	}

	DeleteAllItems();
	DialogBase::OnClose();
}

// 释放资源以后再清空
void  CSystemDlg::DeleteAllItems() {
	for (int i = 0; i < m_ControlList.GetItemCount(); i++)
	{
		auto data = (ItemData*)m_ControlList.GetItemData(i);
		if (NULL != data) {
			delete data;
		}
	}
	m_ControlList.DeleteAllItems();
}

int CALLBACK CSystemDlg::CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	auto* pSortInfo = reinterpret_cast<std::pair<int, bool>*>(lParamSort);
	int nColumn = pSortInfo->first;
	bool bAscending = pSortInfo->second;

	// 获取列值
	ItemData* context1 = (ItemData*)lParam1;
	ItemData* context2 = (ItemData*)lParam2;
	CString s1 = context1->GetData(nColumn);
	CString s2 = context2->GetData(nColumn);

	int result = s1.Compare(s2);
	return bAscending ? result : -result;
}

void CSystemDlg::SortByColumn(int nColumn) {
	static int m_nSortColumn = 0;
	static bool m_bSortAscending = false;
	if (nColumn == m_nSortColumn) {
		// 如果点击的是同一列，切换排序顺序
		m_bSortAscending = !m_bSortAscending;
	}
	else {
		// 否则，切换到新列并设置为升序
		m_nSortColumn = nColumn;
		m_bSortAscending = true;
	}

	// 创建排序信息
	std::pair<int, bool> sortInfo(m_nSortColumn, m_bSortAscending);
	m_ControlList.SortItems(CompareFunction, reinterpret_cast<LPARAM>(&sortInfo));
}

void CSystemDlg::OnHdnItemclickList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMHEADER pNMHeader = reinterpret_cast<LPNMHEADER>(pNMHDR);
	int nColumn = pNMHeader->iItem; // 获取点击的列索引
	SortByColumn(nColumn);          // 调用排序函数
	*pResult = 0;
}

void CSystemDlg::OnNMRClickListSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CMenu	Menu;
	if (m_bHow==TOKEN_PSLIST)      //进程管理初始化列表
	{
		Menu.LoadMenu(IDR_PROCESS_LIST);
	}else if (m_bHow==TOKEN_WSLIST)
	{
		Menu.LoadMenu(IDR_WINDOW_LIST);
	}
	CMenu*	SubMenu = Menu.GetSubMenu(0);
	CPoint	Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);

	*pResult = 0;
}

void CSystemDlg::OnPlistKill()
{
	CListCtrl	*ListCtrl = NULL;
	if (m_ControlList.IsWindowVisible())
		ListCtrl = &m_ControlList;
	else
		return;

	//[KILL][ID][ID][iD][ID]
	//非配缓冲区
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (ListCtrl->GetSelectedCount() * 4));//1.exe  4  ID   Handle
	//加入结束进程的数据头
	szBuffer[0] = COMMAND_KILLPROCESS; 
	//显示警告信息
	char *szTips = "警告: 终止进程会导致不希望发生的结果，\n"
		"包括数据丢失和系统不稳定。在被终止前，\n"
		"进程将没有机会保存其状态和数据。";
	CString str;
	if (ListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%s确实\n想终止这%d项进程吗?", szTips, ListCtrl->GetSelectedCount());	
	}
	else
	{
		str.Format("%s确实\n想终止该项进程吗?", szTips);
	}
	if (::MessageBox(m_hWnd, str, "进程结束警告", MB_YESNO | MB_ICONQUESTION) == IDNO) {
		LocalFree(szBuffer);
		return;
	}

	DWORD	dwOffset = 1;
	POSITION Pos = ListCtrl->GetFirstSelectedItemPosition(); 
	//得到要结束哪个进程
	while(Pos) 
	{
		int	nItem = ListCtrl->GetNextSelectedItem(Pos);
		auto data = (ItemData*)ListCtrl->GetItemData(nItem);
		DWORD dwProcessID = data->ID;
		memcpy(szBuffer + dwOffset, &dwProcessID, sizeof(DWORD));  //sdkfj101112
		dwOffset += sizeof(DWORD);
	}
	//发送数据到被控端在被控端中查找COMMAND_KILLPROCESS这个数据头
	m_ContextObject->Send2Client(szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);

	Sleep(100);

	OnPlistRefresh();
}


VOID CSystemDlg::OnPlistRefresh()
{
	if (m_ControlList.IsWindowVisible())
	{
		DeleteAllItems();
		GetProcessList();
		ShowProcessList();
	}
}


VOID CSystemDlg::GetProcessList(void)
{
	BYTE bToken = COMMAND_PSLIST;
	m_ContextObject->Send2Client(&bToken, 1);
}


void CSystemDlg::OnWlistRefresh()
{
	GetWindowsList();
}


void CSystemDlg::GetWindowsList(void)
{
	BYTE bToken = COMMAND_WSLIST;
	m_ContextObject->Send2Client(&bToken, 1);
}


void CSystemDlg::OnReceiveComplete(void)
{
	switch (m_ContextObject->InDeCompressedBuffer.GetBYTE(0))
	{
	case TOKEN_PSLIST:
		{
			ShowProcessList();

			break;
		}
	case TOKEN_WSLIST:
		{
			ShowWindowsList();
			break;
		}

	default:
		// 传输发生异常数据
		break;
	}
}


void CSystemDlg::OnWlistClose()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{

		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=CMD_WINDOW_CLOSE;           //注意这个就是我们的数据头
		auto data = (ItemData*)pListCtrl->GetItemData(nItem);
		DWORD hwnd = data->ID; //得到窗口的句柄一同发送  4   djfkdfj  dkfjf  4
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));   //1 4
		m_ContextObject->Send2Client(lpMsgBuf, sizeof(lpMsgBuf));			

	}
}


void CSystemDlg::OnWlistHide()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=CMD_WINDOW_TEST;             //窗口处理数据头
		auto data = (ItemData*)pListCtrl->GetItemData(nItem); 
		DWORD hwnd = data->ID;  //得到窗口的句柄一同发送
		pListCtrl->SetItemText(nItem,2,"隐藏");      //注意这时将列表中的显示状态为"隐藏"
		//这样在删除列表条目时就不删除该项了 如果删除该项窗口句柄会丢失 就永远也不能显示了
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));      //得到窗口的句柄一同发送
		DWORD dHow=SW_HIDE;                          //窗口处理参数 0
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_ContextObject->Send2Client(lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistRecover()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]= CMD_WINDOW_TEST;
		auto data = (ItemData*)pListCtrl->GetItemData(nItem);
		DWORD hwnd = data->ID;
		pListCtrl->SetItemText(nItem,2,"显示");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_NORMAL;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_ContextObject->Send2Client(lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistMax()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]= CMD_WINDOW_TEST;
		auto data = (ItemData*)pListCtrl->GetItemData(nItem);
		DWORD hwnd = data->ID;
		pListCtrl->SetItemText(nItem,2,"显示");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_MAXIMIZE;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_ContextObject->Send2Client(lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistMin()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]= CMD_WINDOW_TEST;
		auto data = (ItemData*)pListCtrl->GetItemData(nItem);
		DWORD hwnd = data->ID;
		pListCtrl->SetItemText(nItem,2,"显示");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_MINIMIZE;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_ContextObject->Send2Client(lpMsgBuf, sizeof(lpMsgBuf));	
	}
}

void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (!m_ControlList.GetSafeHwnd()) return; // 确保控件已创建

	// 计算新位置和大小
	CRect rc;
	m_ControlList.GetWindowRect(&rc);
	ScreenToClient(&rc);

	// 重新设置控件大小
	m_ControlList.MoveWindow(0, 0, cx, cy, TRUE);
}
