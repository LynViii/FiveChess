#pragma once
#include "ChessDraw.h"
#include "ChessAI.h"
#include "ChessCommon.h"

#include <vector>

enum enumWinFlag { FIGHTING, WHITE_WIN, BLACK_WIN, PEACE };
enum enumAIDepth { AI_PRIMARY = 0, AI_MIDDLE = 1, AI_HIGH = 2 };
enum enumVSMode { PERSON_VS_PERSON, PERSON_VS_MACHINE };

struct MOVE_RECORD
{
    CPoint pt;
    enumChessColor color;
};

class CChess
{
public:
    CChess(void);
    ~CChess(void);

    void Init(CRect rect);
    void Draw(CDC* pDC);
    void SetPiecePos(CPoint ptCurrent);

    void NewGame();
    BOOL Regret();
    BOOL CanRegret() const;
    void SetVSMode(enumVSMode emVSMode);
    void SetAIDepth(int emAIDepth);

    enumWinFlag GetWinFlag() const;
    enumVSMode GetVSMode() const;
    int GetAIDepth() const;
    BOOL IsBlackTurn() const;
    int GetMoveCount() const;
    CRect GetRectBoard();

    BOOL SetHoverPoint(CPoint point);
    BOOL ClearHoverPoint();

private:
    enumChessColor m_iPositionPiece[COLUMNS][ROWS];
    CChessDraw m_chessdraw;
    CRect m_rcBoard;
    CPoint m_ptCurrent;
    CPoint m_ptHover;
    CPoint m_ptWinStart;
    CPoint m_ptWinEnd;
    enumWinFlag m_emWin;
    enumVSMode m_emVSMode;
    BOOL m_bTurnBlack;
    int m_emAIDepth;
    BOOL m_bHasWinningLine;
    std::vector<MOVE_RECORD> m_moves;

    BOOL IsWin(UINT uiCol, UINT uiRow, enumChessColor emChessColor);
    BOOL GetBestPosByAI(UINT& uiCol, UINT& uiRow);
    void PlacePiece(UINT uiCol, UINT uiRow, enumChessColor color);
    void RefreshTurnAfterUndo();
};
