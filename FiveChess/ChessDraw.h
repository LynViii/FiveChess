#pragma once

class CChessDraw
{
public:
    CChessDraw(void);
    ~CChessDraw(void);

    void InitBoard(CRect rect, UINT uiRows, UINT uiCols, UINT uiBoardWidth, UINT uiPieceRadius);

    void SetDC(CDC* pDC);
    void DrawBackground();
    void DrawBoard();
    BOOL DrawPiece(UINT uiX, UINT uiY, BOOL bBlack);
    void DrawGhostPiece(UINT uiX, UINT uiY, BOOL bBlack);
    void DrawPieceCur(UINT uiX, UINT uiY);
    void DrawWinningLine(UINT x1, UINT y1, UINT x2, UINT y2);

    CRect GetRectBoard();
    CRect GetRectBackground();
    BOOL GetCoordinateWithPoint(CPoint& pt, UINT* puiX = NULL, UINT* puiY = NULL);
    BOOL GetPointWithCoordinate(CPoint& pt, UINT uiX, UINT uiY);

private:
    CDC* m_pDC;
    CRect m_rcBK;
    CRect m_rcBoard;

    UINT m_uiBoardCols;
    UINT m_uiBoardRows;
    UINT m_uiPieceRadius;
    UINT m_uiBoardWidth;

    COLORREF m_crBackground;
    COLORREF m_crBoardSurface;
    COLORREF m_crGrid;
    COLORREF m_crAccent;

    void DrawSpecialPoints();
    void DrawCoordinateLabels();
};
