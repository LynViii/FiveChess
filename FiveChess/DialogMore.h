#pragma once
#include "afxwin.h"
#include "Chess.h"

class CDialogMore : public CDialogEx
{
    DECLARE_DYNAMIC(CDialogMore)

public:
    CDialogMore(CWnd* pParent = NULL);
    virtual ~CDialogMore();

    enum { IDD = IDD_DIALOG_MORE };

    void SetChess(CChess* pChess);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    CChess* m_pChess;
    CComboBox m_comboxAI;
    CComboBox m_comboxVSMode;

    void UpdateControlState();

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnCbnSelchangeVsMode();
};
