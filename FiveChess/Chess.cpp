#include "StdAfx.h"
#include "Chess.h"
#include "MyMemDC.h"

CChess::CChess(void)
{
    m_bTurnBlack = TRUE;
    m_emWin = FIGHTING;
    m_emVSMode = PERSON_VS_MACHINE;
    m_emAIDepth = AI_MIDDLE;
    m_ptCurrent = CPoint(-1, -1);
    m_ptHover = CPoint(-1, -1);
    m_ptWinStart = CPoint(-1, -1);
    m_ptWinEnd = CPoint(-1, -1);
    m_bHasWinningLine = FALSE;
    memset(m_iPositionPiece, 0, sizeof(m_iPositionPiece));
}

CChess::~CChess(void)
{
}

void CChess::NewGame()
{
    m_bTurnBlack = TRUE;
    m_emWin = FIGHTING;
    m_ptCurrent = CPoint(-1, -1);
    m_ptHover = CPoint(-1, -1);
    m_ptWinStart = CPoint(-1, -1);
    m_ptWinEnd = CPoint(-1, -1);
    m_bHasWinningLine = FALSE;
    m_moves.clear();
    memset(m_iPositionPiece, 0, sizeof(m_iPositionPiece));
}

void CChess::Init(CRect rect)
{
    const int padding = 68;
    const int usableWidth = max(140, rect.Width() - padding);
    const int usableHeight = max(140, rect.Height() - padding);
    const int cell = max(10, min(usableWidth / (COLUMNS - 1), usableHeight / (ROWS - 1)));
    const int radius = max(4, cell / 2 - 3);

    m_chessdraw.InitBoard(rect, COLUMNS, ROWS, cell, radius);
    m_rcBoard = m_chessdraw.GetRectBoard();
}

CRect CChess::GetRectBoard()
{
    return m_rcBoard;
}

void CChess::Draw(CDC* pDC)
{
    CRect rcDraw = m_chessdraw.GetRectBackground();
    CMyMemDC memDC(pDC, &rcDraw);

    m_chessdraw.SetDC(memDC);
    m_chessdraw.DrawBackground();
    m_chessdraw.DrawBoard();

    for (int x = 0; x < (int)COLUMNS; ++x)
    {
        for (int y = 0; y < (int)ROWS; ++y)
        {
            if (NONE != m_iPositionPiece[x][y])
            {
                m_chessdraw.DrawPiece(x, y, BLACK == m_iPositionPiece[x][y]);
            }
        }
    }

    if (m_emWin == FIGHTING && m_ptHover.x >= 0 && m_ptHover.y >= 0)
    {
        m_chessdraw.DrawGhostPiece(
            m_ptHover.x,
            m_ptHover.y,
            m_bTurnBlack ? TRUE : FALSE);
    }

    m_chessdraw.DrawPieceCur(m_ptCurrent.x, m_ptCurrent.y);

    if (m_bHasWinningLine)
    {
        m_chessdraw.DrawWinningLine(
            m_ptWinStart.x, m_ptWinStart.y,
            m_ptWinEnd.x, m_ptWinEnd.y);
    }
}

void CChess::PlacePiece(UINT uiCol, UINT uiRow, enumChessColor color)
{
    m_iPositionPiece[uiCol][uiRow] = color;

    MOVE_RECORD move;
    move.pt = CPoint(uiCol, uiRow);
    move.color = color;
    m_moves.push_back(move);

    m_ptCurrent = move.pt;
    m_ptHover = CPoint(-1, -1);
}

void CChess::SetPiecePos(CPoint ptCurrent)
{
    if (FIGHTING != m_emWin)
    {
        return;
    }

    UINT uiPosX = 0;
    UINT uiPosY = 0;
    if (!m_chessdraw.GetCoordinateWithPoint(ptCurrent, &uiPosX, &uiPosY))
    {
        return;
    }
    if (NONE != m_iPositionPiece[uiPosX][uiPosY])
    {
        return;
    }

    const enumChessColor playerColor = m_bTurnBlack ? BLACK : WHITE;
    PlacePiece(uiPosX, uiPosY, playerColor);

    if (IsWin(uiPosX, uiPosY, playerColor))
    {
        return;
    }

    if ((int)m_moves.size() >= (int)(COLUMNS * ROWS))
    {
        m_emWin = PEACE;
        return;
    }

    m_bTurnBlack = !m_bTurnBlack;

    if (PERSON_VS_MACHINE == m_emVSMode)
    {
        UINT aiX = 0;
        UINT aiY = 0;
        if (!GetBestPosByAI(aiX, aiY))
        {
            m_emWin = PEACE;
            return;
        }

        PlacePiece(aiX, aiY, WHITE);
        if (IsWin(aiX, aiY, WHITE))
        {
            return;
        }

        if ((int)m_moves.size() >= (int)(COLUMNS * ROWS))
        {
            m_emWin = PEACE;
            return;
        }

        m_bTurnBlack = TRUE;
    }
}

enumWinFlag CChess::GetWinFlag() const
{
    return m_emWin;
}

enumVSMode CChess::GetVSMode() const
{
    return m_emVSMode;
}

int CChess::GetAIDepth() const
{
    return m_emAIDepth;
}

BOOL CChess::IsBlackTurn() const
{
    return m_bTurnBlack;
}

int CChess::GetMoveCount() const
{
    return (int)m_moves.size();
}

BOOL CChess::IsWin(UINT uiCol, UINT uiRow, enumChessColor emChessColor)
{
    static const int dirs[4][2] = {
        { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 }
    };

    for (int i = 0; i < 4; ++i)
    {
        const int dx = dirs[i][0];
        const int dy = dirs[i][1];

        int startX = (int)uiCol;
        int startY = (int)uiRow;
        int endX = (int)uiCol;
        int endY = (int)uiRow;
        int count = 1;

        int x = (int)uiCol - dx;
        int y = (int)uiRow - dy;
        while (x >= 0 && x < (int)COLUMNS && y >= 0 && y < (int)ROWS
            && m_iPositionPiece[x][y] == emChessColor)
        {
            startX = x;
            startY = y;
            ++count;
            x -= dx;
            y -= dy;
        }

        x = (int)uiCol + dx;
        y = (int)uiRow + dy;
        while (x >= 0 && x < (int)COLUMNS && y >= 0 && y < (int)ROWS
            && m_iPositionPiece[x][y] == emChessColor)
        {
            endX = x;
            endY = y;
            ++count;
            x += dx;
            y += dy;
        }

        if (count >= 5)
        {
            m_emWin = (BLACK == emChessColor) ? BLACK_WIN : WHITE_WIN;
            m_bHasWinningLine = TRUE;
            m_ptWinStart = CPoint(startX, startY);
            m_ptWinEnd = CPoint(endX, endY);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CChess::CanRegret() const
{
    return !m_moves.empty();
}

void CChess::RefreshTurnAfterUndo()
{
    if (PERSON_VS_MACHINE == m_emVSMode)
    {
        m_bTurnBlack = TRUE;
        return;
    }

    if (m_moves.empty())
    {
        m_bTurnBlack = TRUE;
    }
    else
    {
        m_bTurnBlack = (m_moves.back().color == WHITE);
    }
}

BOOL CChess::Regret()
{
    if (m_moves.empty())
    {
        return FALSE;
    }

    int removeCount = 1;
    if (PERSON_VS_MACHINE == m_emVSMode
        && m_moves.size() >= 2
        && m_moves.back().color == WHITE)
    {
        removeCount = 2;
    }

    while (removeCount-- > 0 && !m_moves.empty())
    {
        const MOVE_RECORD move = m_moves.back();
        m_iPositionPiece[move.pt.x][move.pt.y] = NONE;
        m_moves.pop_back();
    }

    m_emWin = FIGHTING;
    m_bHasWinningLine = FALSE;
    m_ptWinStart = CPoint(-1, -1);
    m_ptWinEnd = CPoint(-1, -1);
    m_ptHover = CPoint(-1, -1);
    m_ptCurrent = m_moves.empty() ? CPoint(-1, -1) : m_moves.back().pt;
    RefreshTurnAfterUndo();
    return TRUE;
}

void CChess::SetVSMode(enumVSMode emVSMode)
{
    m_emVSMode = emVSMode;
}

void CChess::SetAIDepth(int emAIDepth)
{
    if (emAIDepth < AI_PRIMARY)
    {
        emAIDepth = AI_PRIMARY;
    }
    if (emAIDepth > AI_HIGH)
    {
        emAIDepth = AI_HIGH;
    }
    m_emAIDepth = emAIDepth;
}

BOOL CChess::GetBestPosByAI(UINT& uiCol, UINT& uiRow)
{
    POINT ptWhite = CPoint(-1, -1);
    BOOL result = FALSE;

    switch (m_emAIDepth)
    {
    case AI_PRIMARY:
        result = ::AIPrimary(ptWhite, m_iPositionPiece);
        break;
    case AI_MIDDLE:
        result = ::AIMiddle(ptWhite, m_iPositionPiece);
        break;
    case AI_HIGH:
        result = ::AIHigh(ptWhite, m_iPositionPiece);
        break;
    default:
        result = ::AIMiddle(ptWhite, m_iPositionPiece);
        break;
    }

    if (!result || ptWhite.x < 0 || ptWhite.y < 0)
    {
        return FALSE;
    }

    uiCol = (UINT)ptWhite.x;
    uiRow = (UINT)ptWhite.y;
    return TRUE;
}

BOOL CChess::SetHoverPoint(CPoint point)
{
    CPoint snapped = point;
    UINT x = 0;
    UINT y = 0;
    CPoint next(-1, -1);

    if (m_emWin == FIGHTING
        && m_chessdraw.GetCoordinateWithPoint(snapped, &x, &y)
        && m_iPositionPiece[x][y] == NONE)
    {
        next = CPoint(x, y);
    }

    if (next == m_ptHover)
    {
        return FALSE;
    }

    m_ptHover = next;
    return TRUE;
}

BOOL CChess::ClearHoverPoint()
{
    if (m_ptHover.x < 0 && m_ptHover.y < 0)
    {
        return FALSE;
    }

    m_ptHover = CPoint(-1, -1);
    return TRUE;
}
