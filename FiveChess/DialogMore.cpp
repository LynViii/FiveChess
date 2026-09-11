#include "stdafx.h"
#include "Gobang_FiveChess.h"
#include "DialogMore.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDialogMore, CDialogEx)

CDialogMore::CDialogMore(CWnd* pParent)
    : CDialogEx(CDialogMore::IDD, pParent), m_pChess(NULL)
{
}

CDialogMore::~CDialogMore()
{
}

void CDialogMore::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_AI, m_comboxAI);
    DDX_Control(pDX, IDC_COMBO_VS_MODE, m_comboxVSMode);
}

BEGIN_MESSAGE_MAP(CDialogMore, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDialogMore::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_COMBO_VS_MODE, &CDialogMore::OnCbnSelchangeVsMode)
END_MESSAGE_MAP()

BOOL CDialogMore::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(_T("对局设置"));

    m_comboxVSMode.ResetContent();
    m_comboxVSMode.AddString(_T("人机对战 · 玩家执黑"));
    m_comboxVSMode.AddString(_T("双人对战 · 本地轮流"));

    m_comboxAI.ResetContent();
    m_comboxAI.AddString(_T("初级 · 启发式选点"));
    m_comboxAI.AddString(_T("标准 · 2 层搜索"));
    m_comboxAI.AddString(_T("高级 · 3 层剪枝"));

    int modeIndex = 0;
    int aiIndex = 1;
    if (m_pChess)
    {
        modeIndex = (m_pChess->GetVSMode() == PERSON_VS_PERSON) ? 1 : 0;

        switch (m_pChess->GetAIDepth())
        {
        case AI_PRIMARY:
            aiIndex = 0;
            break;
        case AI_HIGH:
            aiIndex = 2;
            break;
        case AI_MIDDLE:
        default:
            aiIndex = 1;
            break;
        }
    }

    m_comboxVSMode.SetCurSel(modeIndex);
    m_comboxAI.SetCurSel(aiIndex);
    UpdateControlState();
    return TRUE;
}

void CDialogMore::OnBnClickedOk()
{
    if (m_pChess)
    {
        const int modeIndex = m_comboxVSMode.GetCurSel();
        const int aiIndex = m_comboxAI.GetCurSel();

        m_pChess->SetVSMode(modeIndex == 1 ? PERSON_VS_PERSON : PERSON_VS_MACHINE);

        switch (aiIndex)
        {
        case 0:
            m_pChess->SetAIDepth(AI_PRIMARY);
            break;
        case 2:
            m_pChess->SetAIDepth(AI_HIGH);
            break;
        case 1:
        default:
            m_pChess->SetAIDepth(AI_MIDDLE);
            break;
        }
    }

    CDialogEx::OnOK();
}

void CDialogMore::OnCbnSelchangeVsMode()
{
    UpdateControlState();
}

void CDialogMore::UpdateControlState()
{
    const BOOL enableAI = (m_comboxVSMode.GetCurSel() == 0);
    m_comboxAI.EnableWindow(enableAI);
}

void CDialogMore::SetChess(CChess* pChess)
{
    m_pChess = pChess;
}
