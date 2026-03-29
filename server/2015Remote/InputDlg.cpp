// InputDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDialog 对话框

IMPLEMENT_DYNAMIC(CInputDialog, CDialogEx)

CInputDialog::CInputDialog(CWnd* pParent /*=nullptr*/)
    : CDialogLangEx(IDD_DIALOG_INPUT, pParent)
    , m_sSecondInput(_T(""))
    , m_sThirdInput(_T(""))
    , m_sTipInfo(_T(""))
{
    m_hIcon = NULL;
}

CInputDialog::~CInputDialog()
{
}

void CInputDialog::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_SECOND, m_Static2thInput);
    DDX_Control(pDX, IDC_EDIT_SECOND, m_Edit2thInput);
    DDX_Text(pDX, IDC_EDIT_SECOND, m_sSecondInput);
    DDV_MaxChars(pDX, m_sSecondInput, 100);
    DDX_Control(pDX, IDC_STATIC_THIRD, m_Static3rdInput);
    DDX_Control(pDX, IDC_EDIT_THIRD, m_Edit3rdInput);
    DDX_Text(pDX, IDC_EDIT_THIRD, m_sThirdInput);
    DDV_MaxChars(pDX, m_sThirdInput, 100);
    DDX_Control(pDX, IDC_STATIC_TIPINFO, m_StaticTipInfo);
    DDX_Text(pDX, IDC_STATIC_TIPINFO, m_sTipInfo);
    DDV_MaxChars(pDX, m_sTipInfo, 64);
}


BEGIN_MESSAGE_MAP(CInputDialog, CDialogEx)
    ON_BN_CLICKED(IDOK, &CInputDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputDialog 消息处理程序
BOOL CInputDialog::Init(LPCTSTR caption, LPCTSTR prompt)
{
    m_sCaption = caption;
    m_sPrompt = prompt;

    return TRUE;
}

void CInputDialog::Init2(LPCTSTR name, LPCTSTR defaultValue)
{
    m_sItemName = name;
    m_sSecondInput = defaultValue;
}

void CInputDialog::Init3(LPCTSTR name, LPCTSTR defaultValue)
{
    m_sItemName3 = name;
    m_sThirdInput = defaultValue;
}

BOOL CInputDialog::OnInitDialog()
{
    __super::OnInitDialog();
    // 多语言翻译 - Static控件
    SetDlgItemText(IDC_STATIC_SECOND, _TR("另一个输入框:"));
    SetDlgItemText(IDC_STATIC_THIRD, _TR("第三个输入框:"));
    SetDlgItemText(IDC_STATIC_TIPINFO, _TR("提示信息"));
    SetDlgItemText(IDC_STATIC_INPUT_PROMPT, _TR("请输入目录:"));
    SetDlgItemText(IDOK, _TR("确定"));
    SetDlgItemText(IDCANCEL, _TR("取消"));

    SetIcon(m_hIcon, FALSE);

    SetWindowText(m_sCaption);
    SetDlgItemText(IDC_STATIC_INPUT_PROMPT, m_sPrompt);
    GetDlgItem(IDC_EDIT_FOLDERNAME)->SetWindowText(m_str);

    // 设置输入框内容和显示状态
    m_Static2thInput.SetWindowTextA(m_sItemName);
    m_Static2thInput.ShowWindow(m_sItemName.IsEmpty() ? SW_HIDE : SW_SHOW);
    m_Edit2thInput.SetWindowTextA(m_sSecondInput);
    m_Edit2thInput.ShowWindow(m_sItemName.IsEmpty() ? SW_HIDE : SW_SHOW);
    m_Static3rdInput.SetWindowTextA(m_sItemName3);
    m_Static3rdInput.ShowWindow(m_sItemName3.IsEmpty() ? SW_HIDE : SW_SHOW);
    m_Edit3rdInput.SetWindowTextA(m_sThirdInput);
    m_Edit3rdInput.ShowWindow(m_sItemName3.IsEmpty() ? SW_HIDE : SW_SHOW);
    m_StaticTipInfo.SetWindowTextA(m_sTipInfo);
    m_StaticTipInfo.ShowWindow(m_sTipInfo.IsEmpty() ? SW_HIDE : SW_SHOW);

    // 根据输入框数量动态调整对话框高度
    int inputCount = 1;  // 至少有第一个输入框
    if (!m_sItemName.IsEmpty()) inputCount = 2;
    if (!m_sItemName3.IsEmpty()) inputCount = 3;

    // 计算新高度和按钮位置 (对话框单位)
    // 1个输入框: 高度57, 按钮y=36
    // 2个输入框: 高度81, 按钮y=60
    // 3个输入框: 高度105, 按钮y=84
    int dlgHeight = 57 + (inputCount - 1) * 24;
    int buttonY = 36 + (inputCount - 1) * 24;
    int tipY = buttonY - 12;

    // 将对话框单位转换为像素
    CRect dlgRect;
    GetWindowRect(&dlgRect);
    CRect clientRect;
    GetClientRect(&clientRect);

    // 使用 MapDialogRect 将对话框单位转换为像素
    CRect unitRect(0, 0, 4, dlgHeight);
    MapDialogRect(&unitRect);
    int newHeightPixels = unitRect.bottom + (dlgRect.Height() - clientRect.Height());

    // 调整对话框大小
    SetWindowPos(NULL, 0, 0, dlgRect.Width(), newHeightPixels, SWP_NOMOVE | SWP_NOZORDER);

    // 调整按钮位置
    CRect btnRect(0, 0, 4, buttonY);
    MapDialogRect(&btnRect);
    int btnYPixels = btnRect.bottom;

    CWnd* pBtnOK = GetDlgItem(IDOK);
    CWnd* pBtnCancel = GetDlgItem(IDCANCEL);
    if (pBtnOK && pBtnCancel) {
        CRect okRect, cancelRect;
        pBtnOK->GetWindowRect(&okRect);
        ScreenToClient(&okRect);
        pBtnCancel->GetWindowRect(&cancelRect);
        ScreenToClient(&cancelRect);

        pBtnOK->SetWindowPos(NULL, okRect.left, btnYPixels, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        pBtnCancel->SetWindowPos(NULL, cancelRect.left, btnYPixels, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    // 调整提示信息位置
    if (!m_sTipInfo.IsEmpty()) {
        CRect tipUnitRect(0, 0, 4, tipY);
        MapDialogRect(&tipUnitRect);
        CRect tipRect;
        m_StaticTipInfo.GetWindowRect(&tipRect);
        ScreenToClient(&tipRect);
        m_StaticTipInfo.SetWindowPos(NULL, tipRect.left, tipUnitRect.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}


void CInputDialog::OnBnClickedOk()
{
    GetDlgItem(IDC_EDIT_FOLDERNAME)->GetWindowText(m_str);

    __super::OnOK();
}
