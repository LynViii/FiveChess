#include "StdAfx.h"
#include "ChessDraw.h"
#include "FaceFunc.h"

CChessDraw::CChessDraw()
{
    m_pDC = NULL;
    m_rcBK.SetRectEmpty();
    m_rcBoard.SetRectEmpty();

    m_crBackground = RGB(246, 247, 245);
    m_crBoardSurface = RGB(226, 190, 132);
    m_crGrid = RGB(92, 72, 50);
    m_crAccent = RGB(181, 67, 55);

    m_uiPieceRadius = 0;
    m_uiBoardRows = 0;
    m_uiBoardCols = 0;
    m_uiBoardWidth = 0;
}

CChessDraw::~CChessDraw()
{
}

void CChessDraw::InitBoard(CRect rect, UINT uiRows, UINT uiCols, UINT uiBoardWidth, UINT uiPieceRadius)
{
    m_rcBK = rect;
    m_uiBoardRows = uiRows;
    m_uiBoardCols = uiCols;

    const int padding = 38;
    const int usableWidth = max(1, rect.Width() - padding * 2);
    const int usableHeight = max(1, rect.Height() - padding * 2);
    const int autoWidth = min(
        usableWidth / max(1, (int)uiCols - 1),
        usableHeight / max(1, (int)uiRows - 1));

    m_uiBoardWidth = uiBoardWidth > 0 ? min((int)uiBoardWidth, autoWidth) : autoWidth;
    m_uiBoardWidth = max(10, (int)m_uiBoardWidth);

    m_uiPieceRadius = uiPieceRadius > 0
        ? min((int)uiPieceRadius, (int)m_uiBoardWidth / 2 - 2)
        : (int)m_uiBoardWidth / 2 - 3;
    m_uiPieceRadius = max(4, (int)m_uiPieceRadius);

    const int boardWidth = m_uiBoardWidth * (m_uiBoardCols - 1);
    const int boardHeight = m_uiBoardWidth * (m_uiBoardRows - 1);
    const int left = rect.left + (rect.Width() - boardWidth) / 2;
    const int top = rect.top + (rect.Height() - boardHeight) / 2;

    m_rcBoard.SetRect(left, top, left + boardWidth, top + boardHeight);
}

CRect CChessDraw::GetRectBoard()
{
    return m_rcBoard;
}

CRect CChessDraw::GetRectBackground()
{
    return m_rcBK;
}

void CChessDraw::SetDC(CDC* pDC)
{
    m_pDC = pDC;
}

void CChessDraw::DrawBackground()
{
    if (!m_pDC)
    {
        return;
    }

    m_pDC->FillSolidRect(m_rcBK, m_crBackground);

    CRect shadowCard = m_rcBoard;
    shadowCard.InflateRect(27, 27);
    shadowCard.OffsetRect(0, 3);

    CPen shadowPen(PS_SOLID, 1, RGB(230, 223, 211));
    CBrush shadowBrush(RGB(230, 223, 211));
    CPen* oldPen = m_pDC->SelectObject(&shadowPen);
    CBrush* oldBrush = m_pDC->SelectObject(&shadowBrush);
    m_pDC->RoundRect(shadowCard, CPoint(18, 18));

    CRect boardCard = m_rcBoard;
    boardCard.InflateRect(26, 26);

    CPen borderPen(PS_SOLID, 1, RGB(201, 167, 117));
    CBrush boardBrush(m_crBoardSurface);
    m_pDC->SelectObject(&borderPen);
    m_pDC->SelectObject(&boardBrush);
    m_pDC->RoundRect(boardCard, CPoint(18, 18));

    m_pDC->SelectObject(oldBrush);
    m_pDC->SelectObject(oldPen);
}

void CChessDraw::DrawBoard()
{
    if (!m_pDC)
    {
        return;
    }

    CPen gridPen(PS_SOLID, 1, m_crGrid);
    CPen borderPen(PS_SOLID, 2, RGB(68, 54, 39));
    CPen* oldPen = m_pDC->SelectObject(&gridPen);

    int ptTop = m_rcBoard.top;
    for (UINT i = 0; i < m_uiBoardRows; ++i)
    {
        m_pDC->MoveTo(m_rcBoard.left, ptTop);
        m_pDC->LineTo(m_rcBoard.right, ptTop);
        ptTop += m_uiBoardWidth;
    }

    int ptLeft = m_rcBoard.left;
    for (UINT i = 0; i < m_uiBoardCols; ++i)
    {
        m_pDC->MoveTo(ptLeft, m_rcBoard.top);
        m_pDC->LineTo(ptLeft, m_rcBoard.bottom);
        ptLeft += m_uiBoardWidth;
    }

    m_pDC->SelectObject(&borderPen);
    m_pDC->MoveTo(m_rcBoard.left, m_rcBoard.top);
    m_pDC->LineTo(m_rcBoard.right, m_rcBoard.top);
    m_pDC->LineTo(m_rcBoard.right, m_rcBoard.bottom);
    m_pDC->LineTo(m_rcBoard.left, m_rcBoard.bottom);
    m_pDC->LineTo(m_rcBoard.left, m_rcBoard.top);

    m_pDC->SelectObject(oldPen);
    DrawSpecialPoints();
    DrawCoordinateLabels();
}

void CChessDraw::DrawCoordinateLabels()
{
    if (!m_pDC)
    {
        return;
    }

    const int oldBkMode = m_pDC->SetBkMode(TRANSPARENT);
    const COLORREF oldText = m_pDC->SetTextColor(RGB(112, 86, 56));

    for (UINT x = 0; x < m_uiBoardCols; ++x)
    {
        CString label;
        label.Format(_T("%c"), _T('A') + x);
        CPoint pt;
        if (GetPointWithCoordinate(pt, x, 0))
        {
            CRect rc(pt.x - 12, m_rcBoard.top - 24, pt.x + 12, m_rcBoard.top - 7);
            m_pDC->DrawText(label, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    for (UINT y = 0; y < m_uiBoardRows; ++y)
    {
        CString label;
        label.Format(_T("%d"), y + 1);
        CPoint pt;
        if (GetPointWithCoordinate(pt, 0, y))
        {
            CRect rc(m_rcBoard.left - 28, pt.y - 9, m_rcBoard.left - 7, pt.y + 9);
            m_pDC->DrawText(label, rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    m_pDC->SetTextColor(oldText);
    m_pDC->SetBkMode(oldBkMode);
}

BOOL CChessDraw::DrawPiece(UINT uiX, UINT uiY, BOOL bBlack)
{
    CPoint pt;
    if (!m_pDC || !GetPointWithCoordinate(pt, uiX, uiY))
    {
        return FALSE;
    }

    CRect shadowRect(
        pt.x - m_uiPieceRadius + 2,
        pt.y - m_uiPieceRadius + 3,
        pt.x + m_uiPieceRadius + 2,
        pt.y + m_uiPieceRadius + 3);

    CPen shadowPen(PS_SOLID, 1, RGB(179, 153, 116));
    CBrush shadowBrush(RGB(179, 153, 116));
    CPen* oldPen = m_pDC->SelectObject(&shadowPen);
    CBrush* oldBrush = m_pDC->SelectObject(&shadowBrush);
    m_pDC->Ellipse(shadowRect);

    CRect pieceRect(
        pt.x - m_uiPieceRadius,
        pt.y - m_uiPieceRadius,
        pt.x + m_uiPieceRadius,
        pt.y + m_uiPieceRadius);

    CPen piecePen(PS_SOLID, 1, bBlack ? RGB(18, 20, 19) : RGB(170, 166, 154));
    CBrush pieceBrush(bBlack ? RGB(37, 41, 39) : RGB(247, 245, 238));
    m_pDC->SelectObject(&piecePen);
    m_pDC->SelectObject(&pieceBrush);
    m_pDC->Ellipse(pieceRect);

    CRect highlight = pieceRect;
    highlight.DeflateRect(max(2, (int)m_uiPieceRadius / 3), max(2, (int)m_uiPieceRadius / 3));
    highlight.OffsetRect(-max(1, (int)m_uiPieceRadius / 5), -max(1, (int)m_uiPieceRadius / 5));

    CPen highlightPen(PS_SOLID, 1, bBlack ? RGB(83, 89, 85) : RGB(255, 255, 252));
    CBrush highlightBrush(bBlack ? RGB(83, 89, 85) : RGB(255, 255, 252));
    m_pDC->SelectObject(&highlightPen);
    m_pDC->SelectObject(&highlightBrush);
    m_pDC->Ellipse(highlight);

    m_pDC->SelectObject(oldBrush);
    m_pDC->SelectObject(oldPen);
    return TRUE;
}

void CChessDraw::DrawGhostPiece(UINT uiX, UINT uiY, BOOL bBlack)
{
    CPoint pt;
    if (!m_pDC || !GetPointWithCoordinate(pt, uiX, uiY))
    {
        return;
    }

    const int radius = max(4, (int)m_uiPieceRadius - 2);
    CRect rc(pt.x - radius, pt.y - radius, pt.x + radius, pt.y + radius);

    const COLORREF border = bBlack ? RGB(104, 111, 107) : RGB(177, 172, 159);
    const COLORREF fill = bBlack ? RGB(177, 183, 179) : RGB(250, 248, 240);
    CPen pen(PS_DOT, 1, border);
    CBrush brush(fill);
    CPen* oldPen = m_pDC->SelectObject(&pen);
    CBrush* oldBrush = m_pDC->SelectObject(&brush);
    m_pDC->Ellipse(rc);
    m_pDC->SelectObject(oldBrush);
    m_pDC->SelectObject(oldPen);
}

void CChessDraw::DrawPieceCur(UINT uiX, UINT uiY)
{
    CPoint pt;
    if (!m_pDC || !GetPointWithCoordinate(pt, uiX, uiY))
    {
        return;
    }

    const int radius = max(2, (int)m_uiPieceRadius / 4);
    CRect marker(pt.x - radius, pt.y - radius, pt.x + radius + 1, pt.y + radius + 1);
    CPen markerPen(PS_SOLID, 1, m_crAccent);
    CBrush markerBrush(m_crAccent);
    CPen* oldPen = m_pDC->SelectObject(&markerPen);
    CBrush* oldBrush = m_pDC->SelectObject(&markerBrush);
    m_pDC->Ellipse(marker);
    m_pDC->SelectObject(oldBrush);
    m_pDC->SelectObject(oldPen);
}

void CChessDraw::DrawWinningLine(UINT x1, UINT y1, UINT x2, UINT y2)
{
    CPoint start;
    CPoint end;
    if (!m_pDC
        || !GetPointWithCoordinate(start, x1, y1)
        || !GetPointWithCoordinate(end, x2, y2))
    {
        return;
    }

    CPen linePen(PS_SOLID, 3, m_crAccent);
    CPen* oldPen = m_pDC->SelectObject(&linePen);
    m_pDC->MoveTo(start);
    m_pDC->LineTo(end);
    m_pDC->SelectObject(oldPen);
}

void CChessDraw::DrawSpecialPoints()
{
    if (!m_pDC)
    {
        return;
    }

    const int points[][2] = {
        { 3, 3 }, { 11, 3 }, { 7, 7 }, { 3, 11 }, { 11, 11 }
    };
    const int radius = max(2, (int)m_uiBoardWidth / 10);

    CPen pointPen(PS_SOLID, 1, RGB(67, 51, 35));
    CBrush pointBrush(RGB(67, 51, 35));
    CPen* oldPen = m_pDC->SelectObject(&pointPen);
    CBrush* oldBrush = m_pDC->SelectObject(&pointBrush);

    for (int i = 0; i < 5; ++i)
    {
        CPoint pt;
        if (GetPointWithCoordinate(pt, points[i][0], points[i][1]))
        {
            m_pDC->Ellipse(pt.x - radius, pt.y - radius, pt.x + radius + 1, pt.y + radius + 1);
        }
    }

    m_pDC->SelectObject(oldBrush);
    m_pDC->SelectObject(oldPen);
}

BOOL CChessDraw::GetCoordinateWithPoint(CPoint& pt, UINT* puiX, UINT* puiY)
{
    CRect rcTmp = m_rcBoard;
    rcTmp.InflateRect(CSize(m_uiBoardWidth / 2, m_uiBoardWidth / 2));
    if (!rcTmp.PtInRect(pt))
    {
        return FALSE;
    }

    UINT uiPosX = (pt.x - rcTmp.left) / m_uiBoardWidth;
    UINT uiPosY = (pt.y - rcTmp.top) / m_uiBoardWidth;

    if (uiPosX >= m_uiBoardCols || uiPosY >= m_uiBoardRows)
    {
        return FALSE;
    }

    pt.x = m_rcBoard.left + m_uiBoardWidth * uiPosX;
    pt.y = m_rcBoard.top + m_uiBoardWidth * uiPosY;

    if (puiX)
    {
        *puiX = uiPosX;
    }
    if (puiY)
    {
        *puiY = uiPosY;
    }

    return TRUE;
}

BOOL CChessDraw::GetPointWithCoordinate(CPoint& pt, UINT uiX, UINT uiY)
{
    if (uiX >= m_uiBoardCols || uiY >= m_uiBoardRows)
    {
        return FALSE;
    }

    pt.x = m_rcBoard.left + m_uiBoardWidth * uiX;
    pt.y = m_rcBoard.top + m_uiBoardWidth * uiY;
    return TRUE;
}
