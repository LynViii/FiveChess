#pragma once
#include "Chess.h"

class CGobang_FiveChessDlg : public CDialogEx
{
public:
    CGobang_FiveChessDlg(CWnd* pParent = NULL);
    virtual ~CGobang_FiveChessDlg();

    enum { IDD = IDD_GOBANG_FIVECHESS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    HICON m_hIcon;
    CChess m_chess;
    CRect m_rcBoardArea;
    CRect m_rcSidePanel;
    CFont m_fontTitle;
    CFont m_fontSubtitle;
    CFont m_fontBody;
    CFont m_fontButton;

    void LayoutScene(int cx, int cy);
    void DrawSidebar(CDC* pDC);
    CString GetModeText() const;
    CString GetDifficultyText() const;
    CString GetStatusText() const;

    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnBnClickedButtonGameStart();
    afx_msg void OnBnClickedButtonRegret();
    afx_msg void OnBnClickedButtonMore();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

    DECLARE_MESSAGE_MAP()
};
