#include "stdafx.h"
#include "Gobang_FiveChess.h"
#include "Gobang_FiveChessDlg.h"
#include "afxdialogex.h"
#include "DialogMore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CGobang_FiveChessDlg::CGobang_FiveChessDlg(CWnd* pParent)
    : CDialogEx(CGobang_FiveChessDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

CGobang_FiveChessDlg::~CGobang_FiveChessDlg()
{
}

void CGobang_FiveChessDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGobang_FiveChessDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_LBUTTONUP()
    ON_WM_ERASEBKGND()
    ON_BN_CLICKED(IDC_BUTTON_GAME_START, &CGobang_FiveChessDlg::OnBnClickedButtonGameStart)
    ON_BN_CLICKED(IDC_BUTTON_REGRET, &CGobang_FiveChessDlg::OnBnClickedButtonRegret)
    ON_BN_CLICKED(IDC_BUTTON_MORE, &CGobang_FiveChessDlg::OnBnClickedButtonMore)
    ON_WM_SIZE()
    ON_WM_MOUSEMOVE()
    ON_WM_DRAWITEM()
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

BOOL CGobang_FiveChessDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    SetWindowText(_T("FiveChess · 五子棋"));

    m_fontTitle.CreatePointFont(190, _T("Microsoft YaHei UI"));
    m_fontSubtitle.CreatePointFont(95, _T("Microsoft YaHei UI"));
    m_fontBody.CreatePointFont(95, _T("Microsoft YaHei UI"));
    m_fontButton.CreatePointFont(100, _T("Microsoft YaHei UI"));

    const int buttonIds[] = { IDC_BUTTON_GAME_START, IDC_BUTTON_REGRET, IDC_BUTTON_MORE };
    for (int i = 0; i < 3; ++i)
    {
        CWnd* pButton = GetDlgItem(buttonIds[i]);
        if (pButton)
        {
            pButton->ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW);
            pButton->SetFont(&m_fontButton);
        }
    }

    CRect windowRect;
    GetWindowRect(&windowRect);
    const int targetWidth = max(820, windowRect.Width());
    const int targetHeight = max(590, windowRect.Height());
    SetWindowPos(NULL, 0, 0, targetWidth, targetHeight, SWP_NOMOVE | SWP_NOZORDER);
    CenterWindow();

    CRect client;
    GetClientRect(&client);
    LayoutScene(client.Width(), client.Height());
    UpdateActionState();
    return TRUE;
}

BOOL CGobang_FiveChessDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg && pMsg->message == WM_KEYDOWN)
    {
        const BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (pMsg->wParam == VK_F2 || (ctrl && (pMsg->wParam == 'N' || pMsg->wParam == 'n')))
        {
            OnBnClickedButtonGameStart();
            return TRUE;
        }

        if (ctrl && (pMsg->wParam == 'Z' || pMsg->wParam == 'z'))
        {
            OnBnClickedButtonRegret();
            return TRUE;
        }
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

void CGobang_FiveChessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CGobang_FiveChessDlg::LayoutScene(int cx, int cy)
{
    if (cx <= 0 || cy <= 0)
    {
        return;
    }

    const int margin = 24;
    const int headerHeight = 76;
    const int gap = 18;
    const int panelWidth = min(230, max(200, cx / 4));

    m_rcSidePanel.SetRect(cx - margin - panelWidth, headerHeight, cx - margin, cy - margin);
    m_rcBoardArea.SetRect(margin, headerHeight, m_rcSidePanel.left - gap, cy - margin);

    if (m_rcBoardArea.Width() > 120 && m_rcBoardArea.Height() > 120)
    {
        m_chess.Init(m_rcBoardArea);
    }

    const int buttonLeft = m_rcSidePanel.left + 20;
    const int buttonWidth = m_rcSidePanel.Width() - 40;
    const int buttonHeight = 40;

    struct ButtonLayout
    {
        int id;
        int offsetFromBottom;
    };

    const ButtonLayout layouts[] = {
        { IDC_BUTTON_GAME_START, 166 },
        { IDC_BUTTON_REGRET, 114 },
        { IDC_BUTTON_MORE, 62 }
    };

    for (int i = 0; i < 3; ++i)
    {
        CWnd* pButton = GetDlgItem(layouts[i].id);
        if (pButton && pButton->GetSafeHwnd())
        {
            pButton->MoveWindow(
                buttonLeft,
                m_rcSidePanel.bottom - layouts[i].offsetFromBottom,
                buttonWidth,
                buttonHeight,
                TRUE);
        }
    }
}

void CGobang_FiveChessDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        const int cxIcon = GetSystemMetrics(SM_CXICON);
        const int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        const int x = (rect.Width() - cxIcon + 1) / 2;
        const int y = (rect.Height() - cyIcon + 1) / 2;
        dc.DrawIcon(x, y, m_hIcon);
        return;
    }

    CPaintDC dc(this);
    CRect client;
    GetClientRect(&client);
    dc.FillSolidRect(client, RGB(246, 247, 245));
    dc.SetBkMode(TRANSPARENT);

    CFont* oldFont = dc.SelectObject(&m_fontTitle);
    dc.SetTextColor(RGB(31, 43, 38));
    dc.TextOut(24, 18, _T("FiveChess"));

    dc.SelectObject(&m_fontSubtitle);
    dc.SetTextColor(RGB(115, 124, 119));
    dc.TextOut(25, 49, _T("五子棋 · Alpha-Beta AI"));

    CPen headerLine(PS_SOLID, 1, RGB(226, 230, 227));
    CPen* oldPen = dc.SelectObject(&headerLine);
    dc.MoveTo(24, 70);
    dc.LineTo(client.right - 24, 70);
    dc.SelectObject(oldPen);

    m_chess.Draw(&dc);
    DrawSidebar(&dc);

    dc.SelectObject(oldFont);
}

void CGobang_FiveChessDlg::DrawSidebar(CDC* pDC)
{
    if (!pDC || m_rcSidePanel.IsRectEmpty())
    {
        return;
    }

    CPen borderPen(PS_SOLID, 1, RGB(222, 227, 223));
    CBrush panelBrush(RGB(255, 255, 255));
    CPen* oldPen = pDC->SelectObject(&borderPen);
    CBrush* oldBrush = pDC->SelectObject(&panelBrush);
    pDC->RoundRect(m_rcSidePanel, CPoint(18, 18));

    pDC->SetBkMode(TRANSPARENT);
    CFont* oldFont = pDC->SelectObject(&m_fontBody);
    pDC->SetTextColor(RGB(42, 52, 47));

    const int x = m_rcSidePanel.left + 20;
    int y = m_rcSidePanel.top + 22;

    pDC->SelectObject(&m_fontButton);
    pDC->TextOut(x, y, _T("对局信息"));
    y += 36;

    CRect statusRect(x, y, m_rcSidePanel.right - 20, y + 34);
    CBrush statusBrush(RGB(40, 81, 65));
    CPen statusPen(PS_SOLID, 1, RGB(40, 81, 65));
    pDC->SelectObject(&statusPen);
    pDC->SelectObject(&statusBrush);
    pDC->RoundRect(statusRect, CPoint(12, 12));
    pDC->SetTextColor(RGB(255, 255, 255));
    pDC->SelectObject(&m_fontBody);
    CString status = GetStatusText();
    pDC->DrawText(status, statusRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(&borderPen);
    pDC->SelectObject(&panelBrush);
    pDC->SetTextColor(RGB(94, 105, 99));
    y += 54;

    CString line;
    line.Format(_T("模式        %s"), GetModeText().GetString());
    pDC->TextOut(x, y, line);
    y += 30;

    line.Format(_T("AI 难度     %s"), GetDifficultyText().GetString());
    pDC->TextOut(x, y, line);
    y += 30;

    line.Format(_T("已落子      %d"), m_chess.GetMoveCount());
    pDC->TextOut(x, y, line);
    y += 30;

    line.Format(_T("可悔棋      %s"), m_chess.CanRegret() ? _T("是") : _T("否"));
    pDC->TextOut(x, y, line);
    y += 42;

    CPen divider(PS_SOLID, 1, RGB(234, 237, 235));
    pDC->SelectObject(&divider);
    pDC->MoveTo(x, y);
    pDC->LineTo(m_rcSidePanel.right - 20, y);
    y += 18;

    pDC->SetTextColor(RGB(132, 140, 136));
    CRect hintRect(x, y, m_rcSidePanel.right - 20, y + 96);
    pDC->DrawText(
        _T("悬停可预览落子位置\n红点标记最近一步\nCtrl+Z 悔棋 · F2 新对局"),
        hintRect,
        DT_LEFT | DT_TOP | DT_WORDBREAK);

    pDC->SelectObject(oldFont);
    pDC->SelectObject(oldBrush);
    pDC->SelectObject(oldPen);
}

void CGobang_FiveChessDlg::UpdateActionState()
{
    CWnd* pUndo = GetDlgItem(IDC_BUTTON_REGRET);
    if (pUndo && pUndo->GetSafeHwnd())
    {
        pUndo->EnableWindow(m_chess.CanRegret());
    }
}

CString CGobang_FiveChessDlg::GetModeText() const
{
    switch (m_chess.GetVSMode())
    {
    case PERSON_VS_PERSON:
        return _T("双人对战");
    case PERSON_VS_MACHINE:
    default:
        return _T("人机对战");
    }
}

CString CGobang_FiveChessDlg::GetDifficultyText() const
{
    if (m_chess.GetVSMode() == PERSON_VS_PERSON)
    {
        return _T("—");
    }

    switch (m_chess.GetAIDepth())
    {
    case AI_PRIMARY:
        return _T("初级");
    case AI_HIGH:
        return _T("高级");
    case AI_MIDDLE:
    default:
        return _T("标准");
    }
}

CString CGobang_FiveChessDlg::GetStatusText() const
{
    switch (m_chess.GetWinFlag())
    {
    case BLACK_WIN:
        return _T("黑棋获胜");
    case WHITE_WIN:
        return _T("白棋获胜");
    case PEACE:
        return _T("本局平局");
    default:
        if (m_chess.GetVSMode() == PERSON_VS_MACHINE)
        {
            return _T("轮到你 · 黑棋");
        }
        return m_chess.IsBlackTurn() ? _T("黑棋回合") : _T("白棋回合");
    }
}

HCURSOR CGobang_FiveChessDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CGobang_FiveChessDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (!m_chess.GetRectBoard().PtInRect(point))
    {
        CDialogEx::OnLButtonUp(nFlags, point);
        return;
    }

    const enumWinFlag before = m_chess.GetWinFlag();
    m_chess.SetPiecePos(point);
    const enumWinFlag after = m_chess.GetWinFlag();
    UpdateActionState();
    Invalidate(FALSE);

    if (before == FIGHTING && after != FIGHTING)
    {
        switch (after)
        {
        case WHITE_WIN:
            AfxMessageBox(_T("白棋获胜。可以悔棋继续本局，或开始新对局。"), MB_OK | MB_ICONINFORMATION);
            break;
        case BLACK_WIN:
            AfxMessageBox(_T("黑棋获胜。可以悔棋继续本局，或开始新对局。"), MB_OK | MB_ICONINFORMATION);
            break;
        case PEACE:
            AfxMessageBox(_T("本局平局。"), MB_OK | MB_ICONINFORMATION);
            break;
        default:
            break;
        }
    }

    CDialogEx::OnLButtonUp(nFlags, point);
}

BOOL CGobang_FiveChessDlg::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CGobang_FiveChessDlg::OnBnClickedButtonGameStart()
{
    m_chess.NewGame();
    UpdateActionState();
    Invalidate(FALSE);
}

void CGobang_FiveChessDlg::OnBnClickedButtonRegret()
{
    if (!m_chess.Regret())
    {
        return;
    }

    UpdateActionState();
    Invalidate(FALSE);
}

void CGobang_FiveChessDlg::OnBnClickedButtonMore()
{
    CDialogMore dlgMore(this);
    dlgMore.SetChess(&m_chess);
    if (dlgMore.DoModal() == IDOK)
    {
        m_chess.NewGame();
        UpdateActionState();
        Invalidate(FALSE);
    }
}

void CGobang_FiveChessDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if (GetSafeHwnd() && cx > 0 && cy > 0)
    {
        LayoutScene(cx, cy);
        Invalidate(FALSE);
    }
}

void CGobang_FiveChessDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    BOOL changed = FALSE;
    if (m_chess.GetRectBoard().PtInRect(point))
    {
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
        changed = m_chess.SetHoverPoint(point);
    }
    else
    {
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        changed = m_chess.ClearHoverPoint();
    }

    if (changed)
    {
        Invalidate(FALSE);
    }

    CDialogEx::OnMouseMove(nFlags, point);
}

void CGobang_FiveChessDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (nIDCtl != IDC_BUTTON_GAME_START
        && nIDCtl != IDC_BUTTON_REGRET
        && nIDCtl != IDC_BUTTON_MORE)
    {
        CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
        return;
    }

    CDC dc;
    dc.Attach(lpDrawItemStruct->hDC);
    CRect rc(lpDrawItemStruct->rcItem);

    const BOOL isPrimary = (nIDCtl == IDC_BUTTON_GAME_START);
    const BOOL isPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
    const BOOL isDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

    COLORREF fill = isPrimary ? RGB(40, 81, 65) : RGB(248, 249, 248);
    COLORREF border = isPrimary ? RGB(40, 81, 65) : RGB(217, 223, 219);
    COLORREF text = isPrimary ? RGB(255, 255, 255) : RGB(55, 67, 61);

    if (isPressed)
    {
        fill = isPrimary ? RGB(31, 65, 52) : RGB(235, 238, 236);
    }
    if (isDisabled)
    {
        fill = RGB(240, 242, 241);
        border = RGB(226, 229, 227);
        text = RGB(166, 173, 169);
    }

    CPen pen(PS_SOLID, 1, border);
    CBrush brush(fill);
    CPen* oldPen = dc.SelectObject(&pen);
    CBrush* oldBrush = dc.SelectObject(&brush);
    dc.RoundRect(rc, CPoint(12, 12));

    CString label;
    GetDlgItem(nIDCtl)->GetWindowText(label);
    CFont* oldFont = dc.SelectObject(&m_fontButton);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(text);
    dc.DrawText(label, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (lpDrawItemStruct->itemState & ODS_FOCUS)
    {
        CRect focus = rc;
        focus.DeflateRect(4, 4);
        dc.DrawFocusRect(focus);
    }

    dc.SelectObject(oldFont);
    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);
    dc.Detach();
}

void CGobang_FiveChessDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    CDialogEx::OnGetMinMaxInfo(lpMMI);
    lpMMI->ptMinTrackSize.x = 760;
    lpMMI->ptMinTrackSize.y = 540;
}
