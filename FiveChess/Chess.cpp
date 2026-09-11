#include "StdAfx.h"
#include "Chess.h"
#include "MyMemDC.h"

CChess::CChess(void)
{
    m_bTurnBlack = TRUE;
    m_emWin = FIGHTING;
    m_emVSMode = PERSON_VS_MACHINE;
    m_emAIDepth = AI_PRIMARY;
    m_ptCurrent = CPoint(-1, -1);
    memset(m_iPositionPiece, 0, sizeof(m_iPositionPiece));

    m_stcLastPos.iPieceNum = 0;
    m_stcLastPos.ptBlack = CPoint(0, 0);
    m_stcLastPos.ptWhite = CPoint(0, 0);
    m_stcLastPos.ptLastCurPoint = CPoint(-1, -1);
}

CChess::~CChess(void)
{
}

void CChess::NewGame()
{
    m_bTurnBlack = TRUE;
    m_emWin = FIGHTING;
    m_ptCurrent = CPoint(-1, -1);
    memset(m_iPositionPiece, 0, sizeof(m_iPositionPiece));

    m_stcLastPos.iPieceNum = 0;
    m_stcLastPos.ptBlack = CPoint(0, 0);
    m_stcLastPos.ptWhite = CPoint(0, 0);
    m_stcLastPos.ptLastCurPoint = CPoint(-1, -1);
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

    if (FIGHTING == m_emWin)
    {
        m_emWin = PEACE;
    }

    for (int i = 0; i < COLUMNS; ++i)
    {
        for (int j = 0; j < ROWS; ++j)
        {
            if (NONE != m_iPositionPiece[i][j])
            {
                m_chessdraw.DrawPiece(i, j, BLACK == m_iPositionPiece[i][j]);
            }
            else if (PEACE == m_emWin)
            {
                m_emWin = FIGHTING;
            }
        }
    }

    m_chessdraw.DrawPieceCur(m_ptCurrent.x, m_ptCurrent.y);
}

void CChess::SetPiecePos(CPoint ptCurrent)
{
    if (FIGHTING != m_emWin)
    {
        return;
    }

    UINT uiPosX, uiPosY;
    if (!m_chessdraw.GetCoordinateWithPoint(ptCurrent, &uiPosX, &uiPosY))
    {
        return;
    }
    if (NONE != m_iPositionPiece[uiPosX][uiPosY])
    {
        return;
    }

    const int turns = (PERSON_VS_MACHINE == m_emVSMode) ? 2 : 1;
    for (int i = 0; i < turns; ++i)
    {
        m_iPositionPiece[uiPosX][uiPosY] = m_bTurnBlack ? BLACK : WHITE;

        if (IsWin(uiPosX, uiPosY, m_iPositionPiece[uiPosX][uiPosY]))
        {
            m_ptCurrent = CPoint(uiPosX, uiPosY);
            return;
        }

        if ((PERSON_VS_PERSON == m_emVSMode) || (m_bTurnBlack && PERSON_VS_MACHINE == m_emVSMode))
        {
            m_stcLastPos.ptLastCurPoint = m_ptCurrent;
        }

        if (m_bTurnBlack)
        {
            m_stcLastPos.ptBlack = CPoint(uiPosX, uiPosY);
        }
        else
        {
            m_stcLastPos.ptWhite = CPoint(uiPosX, uiPosY);
        }
        m_stcLastPos.iPieceNum++;

        m_ptCurrent = CPoint(uiPosX, uiPosY);
        m_bTurnBlack = !m_bTurnBlack;

        if (!m_bTurnBlack && PERSON_VS_MACHINE == m_emVSMode)
        {
            if (!GetBestPosByAI(uiPosX, uiPosY, BLACK))
            {
                return;
            }
        }
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
    int count = 0;
    for (int i = 0; i < COLUMNS; ++i)
    {
        for (int j = 0; j < ROWS; ++j)
        {
            if (NONE != m_iPositionPiece[i][j])
            {
                ++count;
            }
        }
    }
    return count;
}

BOOL CChess::IsWin(UINT uiCol, UINT uiRow, enumChessColor emChessColor)
{
    int iSameColor[MAXCREASE];

    GetSameColor(uiCol, uiRow, emChessColor, iSameColor, m_iPositionPiece);
    for (int m = 0; m < MAXCREASE; ++m)
    {
        if (iSameColor[m] > MAXCREASE)
        {
            m_emWin = (BLACK == emChessColor) ? BLACK_WIN : WHITE_WIN;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CChess::Regret()
{
    if ((m_stcLastPos.iPieceNum > 0) && (FIGHTING == m_emWin))
    {
        m_stcLastPos.iPieceNum = 0;
        m_ptCurrent = m_stcLastPos.ptLastCurPoint;

        if (PERSON_VS_MACHINE == m_emVSMode)
        {
            m_iPositionPiece[m_stcLastPos.ptBlack.x][m_stcLastPos.ptBlack.y] = NONE;
            m_iPositionPiece[m_stcLastPos.ptWhite.x][m_stcLastPos.ptWhite.y] = NONE;
        }
        else
        {
            if (!m_bTurnBlack)
            {
                m_iPositionPiece[m_stcLastPos.ptBlack.x][m_stcLastPos.ptBlack.y] = NONE;
            }
            else
            {
                m_iPositionPiece[m_stcLastPos.ptWhite.x][m_stcLastPos.ptWhite.y] = NONE;
            }
            m_bTurnBlack = !m_bTurnBlack;
        }
        return TRUE;
    }

    return FALSE;
}

void CChess::SetVSMode(enumVSMode emVSMode)
{
    m_emVSMode = emVSMode;
}

void CChess::SetAIDepth(int emAIDepth)
{
    m_emAIDepth = emAIDepth;
}

BOOL CChess::GetBestPosByAI(UINT& uiCol, UINT& uiRow, enumChessColor emEnemyChessColor)
{
    POINT ptPosWhite = CPoint(-1, -1);
    BOOL bContinue = TRUE;

    switch (m_emAIDepth)
    {
    case AI_PRIMARY:
        bContinue = ::AIPrimary(ptPosWhite, m_iPositionPiece);
        break;

    case AI_HIGH:
        bContinue = ::AIHigh(ptPosWhite, m_iPositionPiece);
        break;

    default:
        bContinue = ::AIPrimary(ptPosWhite, m_iPositionPiece);
        break;
    }

    uiCol = ptPosWhite.x;
    uiRow = ptPosWhite.y;

    if (!bContinue)
    {
        m_emWin = PEACE;
        return FALSE;
    }
    return TRUE;
}
