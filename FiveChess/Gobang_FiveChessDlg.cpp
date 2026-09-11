#include "stdafx.h"
#include "Gobang_FiveChess.h"
#include "Gobang_FiveChessDlg.h"
#include "afxdialogex.h"
#include "DialogMore.h"
#include "MyMemDC.h"

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
    : CDialogEx(CGobang_FiveChessDlg::IDD, pParent),
      m_statusTimer(0),
      m_gameStartTick(0),
      m_gameEndTick(0)
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
    ON_WM_TIMER()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

int CGobang_FiveChessDlg::Scale(int value) const
{
    UINT dpi = 96;
    if (GetSafeHwnd())
    {
        const UINT windowDpi = ::GetDpiForWindow(m_hWnd);
        if (windowDpi > 0)
        {
            dpi = windowDpi;
        }
    }
    return MulDiv(value, dpi, 96);
}

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

    ModifyStyle(0, WS_CLIPCHILDREN);
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    SetWindowText(_T("FiveChess · 五子棋"));

    m_fontTitle.CreatePointFont(205, _T("Microsoft YaHei UI"));
    m_fontSubtitle.CreatePointFont(90, _T("Microsoft YaHei UI"));
    m_fontBody.CreatePointFont(94, _T("Microsoft YaHei UI"));
    m_fontSmall.CreatePointFont(82, _T("Microsoft YaHei UI"));
    m_fontButton.CreatePointFont(96, _T("Microsoft YaHei UI"));

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

    CRect workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    const int maxWidth = max(Scale(780), workArea.Width() - Scale(40));
    const int maxHeight = max(Scale(560), workArea.Height() - Scale(40));
    const int targetWidth = min(Scale(1060), maxWidth);
    const int targetHeight = min(Scale(760), maxHeight);

    SetWindowPos(NULL, 0, 0, targetWidth, targetHeight,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    CenterWindow();

    CRect client;
    GetClientRect(&client);
    LayoutScene(client.Width(), client.Height());
    UpdateActionState();
    ResetGameClock();
    m_statusTimer = SetTimer(1, 1000, NULL);
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

    const int margin = Scale(22);
    const int headerHeight = Scale(92);
    const int gap = Scale(20);
    const int panelMin = Scale(252);
    const int panelMax = Scale(304);
    const int panelWidth = min(panelMax, max(panelMin, cx / 4));

    m_rcSidePanel.SetRect(
        cx - margin - panelWidth,
        headerHeight,
        cx - margin,
        cy - margin);
    m_rcBoardArea.SetRect(
        margin,
        headerHeight,
        m_rcSidePanel.left - gap,
        cy - margin);

    if (m_rcBoardArea.Width() > Scale(160) && m_rcBoardArea.Height() > Scale(160))
    {
        m_chess.Init(m_rcBoardArea);
    }

    const int buttonLeft = m_rcSidePanel.left + Scale(18);
    const int buttonWidth = m_rcSidePanel.Width() - Scale(36);
    const int buttonHeight = Scale(42);
    const int buttonGap = Scale(10);
    const int buttonBottom = m_rcSidePanel.bottom - Scale(18);

    const int ids[] = { IDC_BUTTON_MORE, IDC_BUTTON_REGRET, IDC_BUTTON_GAME_START };
    for (int i = 0; i < 3; ++i)
    {
        CWnd* pButton = GetDlgItem(ids[i]);
        if (pButton && pButton->GetSafeHwnd())
        {
            const int bottom = buttonBottom - i * (buttonHeight + buttonGap);
            pButton->SetWindowPos(NULL,
                buttonLeft,
                bottom - buttonHeight,
                buttonWidth,
                buttonHeight,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
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

    CPaintDC paintDC(this);
    CRect client;
    GetClientRect(&client);

    CMyMemDC dc(&paintDC, &client);
    dc.SetBkMode(TRANSPARENT);
    dc.FillSolidRect(client, RGB(247, 248, 246));

    CFont* oldFont = dc.SelectObject(&m_fontTitle);
    dc.SetTextColor(RGB(28, 37, 33));
    CRect titleRect(Scale(24), Scale(13), client.right - Scale(220), Scale(51));
    dc.DrawText(_T("FiveChess"), titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    dc.SelectObject(&m_fontSubtitle);
    dc.SetTextColor(RGB(105, 115, 110));
    CRect subtitleRect(Scale(25), Scale(49), client.right - Scale(240), Scale(78));
    dc.DrawText(_T("C++ / MFC 五子棋 · Alpha-Beta AI"), subtitleRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    CRect badgeRect(client.right - Scale(184), Scale(23), client.right - Scale(24), Scale(57));
    CPen badgePen(PS_SOLID, 1, RGB(216, 224, 219));
    CBrush badgeBrush(RGB(239, 244, 241));
    CPen* oldPen = dc.SelectObject(&badgePen);
    CBrush* oldBrush = dc.SelectObject(&badgeBrush);
    dc.RoundRect(badgeRect, CPoint(Scale(12), Scale(12)));
    dc.SelectObject(&m_fontSmall);
    dc.SetTextColor(RGB(54, 86, 72));
    dc.DrawText(_T("Native C++ · v2.2"), badgeRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CPen headerLine(PS_SOLID, 1, RGB(225, 230, 227));
    dc.SelectObject(&headerLine);
    dc.MoveTo(Scale(24), Scale(82));
    dc.LineTo(client.right - Scale(24), Scale(82));

    dc.SelectObject(oldBrush);
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

    CPen borderPen(PS_SOLID, 1, RGB(220, 226, 222));
    CBrush panelBrush(RGB(255, 255, 255));
    CPen* oldPen = pDC->SelectObject(&borderPen);
    CBrush* oldBrush = pDC->SelectObject(&panelBrush);
    pDC->RoundRect(m_rcSidePanel, CPoint(Scale(18), Scale(18)));

    pDC->SetBkMode(TRANSPARENT);
    CFont* oldFont = pDC->SelectObject(&m_fontBody);

    const int left = m_rcSidePanel.left + Scale(20);
    const int right = m_rcSidePanel.right - Scale(20);
    int y = m_rcSidePanel.top + Scale(20);

    pDC->SetTextColor(RGB(39, 49, 44));
    pDC->SelectObject(&m_fontButton);
    CRect sectionTitle(left, y, right, y + Scale(24));
    pDC->DrawText(_T("对局状态"), sectionTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    y += Scale(34);

    CRect statusRect(left, y, right, y + Scale(40));
    COLORREF statusColor = RGB(45, 91, 72);
    if (m_chess.GetWinFlag() != FIGHTING)
    {
        statusColor = RGB(151, 94, 49);
    }
    CPen statusPen(PS_SOLID, 1, statusColor);
    CBrush statusBrush(statusColor);
    pDC->SelectObject(&statusPen);
    pDC->SelectObject(&statusBrush);
    pDC->RoundRect(statusRect, CPoint(Scale(12), Scale(12)));
    pDC->SelectObject(&m_fontBody);
    pDC->SetTextColor(RGB(255, 255, 255));
    pDC->DrawText(GetStatusText(), statusRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(&borderPen);
    pDC->SelectObject(&panelBrush);
    y += Scale(54);

    const int rowHeight = Scale(31);
    const int labelWidth = Scale(74);

    auto drawRow = [&](LPCTSTR label, const CString& value)
    {
        CRect labelRect(left, y, left + labelWidth, y + rowHeight);
        CRect valueRect(left + labelWidth, y, right, y + rowHeight);

        pDC->SelectObject(&m_fontSmall);
        pDC->SetTextColor(RGB(135, 144, 139));
        pDC->DrawText(label, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        pDC->SelectObject(&m_fontBody);
        pDC->SetTextColor(RGB(52, 62, 57));
        pDC->DrawText(value, valueRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        y += rowHeight;
    };

    CString moveCount;
    moveCount.Format(_T("%d"), m_chess.GetMoveCount());
    drawRow(_T("模式"), GetModeText());
    drawRow(_T("AI 难度"), GetDifficultyText());
    drawRow(_T("落子数"), moveCount);
    drawRow(_T("最近一步"), GetLastMoveText());
    drawRow(_T("本局用时"), GetElapsedText());

    y += Scale(8);
    CPen divider(PS_SOLID, 1, RGB(235, 238, 236));
    pDC->SelectObject(&divider);
    pDC->MoveTo(left, y);
    pDC->LineTo(right, y);
    y += Scale(14);

    const int controlsTop = m_rcSidePanel.bottom - Scale(176);
    if (controlsTop > y + Scale(38))
    {
        pDC->SelectObject(&m_fontSmall);
        pDC->SetTextColor(RGB(128, 138, 132));
        CRect hintRect(left, y, right, controlsTop - Scale(10));
        pDC->DrawText(
            _T("悬停交叉点可预览落子\n红点表示最近一步，胜局显示连线\nCtrl + Z 悔棋 · F2 新对局"),
            hintRect,
            DT_LEFT | DT_TOP | DT_WORDBREAK);
    }

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

void CGobang_FiveChessDlg::ResetGameClock()
{
    m_gameStartTick = GetTickCount64();
    m_gameEndTick = 0;
}

void CGobang_FiveChessDlg::InvalidateGameView(BOOL includeSidebar)
{
    if (!m_rcBoardArea.IsRectEmpty())
    {
        InvalidateRect(&m_rcBoardArea, FALSE);
    }
    if (includeSidebar && !m_rcSidePanel.IsRectEmpty())
    {
        InvalidateRect(&m_rcSidePanel, FALSE);
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
            return _T("你的回合 · 黑棋");
        }
        return m_chess.IsBlackTurn() ? _T("黑棋回合") : _T("白棋回合");
    }
}

CString CGobang_FiveChessDlg::GetElapsedText() const
{
    if (m_gameStartTick == 0)
    {
        return _T("00:00");
    }

    const ULONGLONG endTick = (m_gameEndTick != 0) ? m_gameEndTick : GetTickCount64();
    const DWORD totalSeconds = (DWORD)((endTick - m_gameStartTick) / 1000);
    const int minutes = (int)(totalSeconds / 60);
    const int seconds = (int)(totalSeconds % 60);

    CString text;
    text.Format(_T("%02d:%02d"), minutes, seconds);
    return text;
}

CString CGobang_FiveChessDlg::GetLastMoveText() const
{
    CPoint point;
    enumChessColor color = NONE;
    if (!m_chess.GetLastMove(point, color))
    {
        return _T("—");
    }

    const TCHAR column = (TCHAR)(_T('A') + point.x);
    CString text;
    text.Format(_T("%c%d · %s"), column, point.y + 1,
        color == BLACK ? _T("黑") : _T("白"));
    return text;
}

HCURSOR CGobang_FiveChessDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CGobang_FiveChessDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (!m_rcBoardArea.PtInRect(point))
    {
        CDialogEx::OnLButtonUp(nFlags, point);
        return;
    }

    const enumWinFlag before = m_chess.GetWinFlag();
    {
        CWaitCursor wait;
        if (!m_chess.SetPiecePos(point))
        {
            CDialogEx::OnLButtonUp(nFlags, point);
            return;
        }
    }

    const enumWinFlag after = m_chess.GetWinFlag();
    if (before == FIGHTING && after != FIGHTING)
    {
        m_gameEndTick = GetTickCount64();
    }

    UpdateActionState();
    InvalidateGameView(TRUE);
    UpdateWindow();

    if (before == FIGHTING && after != FIGHTING)
    {
        switch (after)
        {
        case WHITE_WIN:
            AfxMessageBox(_T("白棋获胜！你可以悔棋继续本局，或开始新对局。"),
                MB_OK | MB_ICONINFORMATION);
            break;
        case BLACK_WIN:
            AfxMessageBox(_T("黑棋获胜！你可以悔棋继续本局，或开始新对局。"),
                MB_OK | MB_ICONINFORMATION);
            break;
        case PEACE:
            AfxMessageBox(_T("棋盘已满，本局平局。"), MB_OK | MB_ICONINFORMATION);
            break;
        default:
            break;
        }
    }

    CDialogEx::OnLButtonUp(nFlags, point);
}

BOOL CGobang_FiveChessDlg::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    return TRUE;
}

void CGobang_FiveChessDlg::OnBnClickedButtonGameStart()
{
    m_chess.NewGame();
    ResetGameClock();
    UpdateActionState();
    InvalidateGameView(TRUE);
}

void CGobang_FiveChessDlg::OnBnClickedButtonRegret()
{
    if (!m_chess.Regret())
    {
        return;
    }

    if (m_chess.GetWinFlag() == FIGHTING)
    {
        m_gameEndTick = 0;
    }
    UpdateActionState();
    InvalidateGameView(TRUE);
}

void CGobang_FiveChessDlg::OnBnClickedButtonMore()
{
    CDialogMore dlgMore(this);
    dlgMore.SetChess(&m_chess);
    if (dlgMore.DoModal() == IDOK)
    {
        m_chess.NewGame();
        ResetGameClock();
        UpdateActionState();
        InvalidateGameView(TRUE);
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

    if (changed && !m_rcBoardArea.IsRectEmpty())
    {
        InvalidateRect(&m_rcBoardArea, FALSE);
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

    COLORREF fill = isPrimary ? RGB(45, 91, 72) : RGB(249, 250, 249);
    COLORREF border = isPrimary ? RGB(45, 91, 72) : RGB(215, 222, 218);
    COLORREF text = isPrimary ? RGB(255, 255, 255) : RGB(54, 65, 59);

    if (isPressed)
    {
        fill = isPrimary ? RGB(34, 73, 57) : RGB(235, 239, 236);
    }
    if (isDisabled)
    {
        fill = RGB(241, 243, 242);
        border = RGB(228, 231, 229);
        text = RGB(166, 173, 169);
    }

    CPen pen(PS_SOLID, 1, border);
    CBrush brush(fill);
    CPen* oldPen = dc.SelectObject(&pen);
    CBrush* oldBrush = dc.SelectObject(&brush);
    dc.RoundRect(rc, CPoint(Scale(11), Scale(11)));

    CString label;
    GetDlgItem(nIDCtl)->GetWindowText(label);
    CFont* oldFont = dc.SelectObject(&m_fontButton);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(text);
    dc.DrawText(label, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (lpDrawItemStruct->itemState & ODS_FOCUS)
    {
        CRect focus = rc;
        focus.DeflateRect(Scale(4), Scale(4));
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
    lpMMI->ptMinTrackSize.x = Scale(800);
    lpMMI->ptMinTrackSize.y = Scale(600);
}

void CGobang_FiveChessDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == m_statusTimer && m_chess.GetWinFlag() == FIGHTING)
    {
        if (!m_rcSidePanel.IsRectEmpty())
        {
            InvalidateRect(&m_rcSidePanel, FALSE);
        }
    }
    CDialogEx::OnTimer(nIDEvent);
}

void CGobang_FiveChessDlg::OnDestroy()
{
    if (m_statusTimer != 0)
    {
        KillTimer(m_statusTimer);
        m_statusTimer = 0;
    }
    CDialogEx::OnDestroy();
}
