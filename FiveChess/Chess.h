#pragma once
#include "ChessDraw.h"
#include "ChessAI.h"
#include "ChessCommon.h"

enum enumWinFlag { FIGHTING, WHITE_WIN, BLACK_WIN, PEACE };
enum enumAIDepth { AI_FOOLISH, AI_PRIMARY, AI_MIDDLE, AI_HIGH };
enum enumVSMode { PERSON_VS_PERSON, PERSON_VS_MACHINE, MACHINE_VS_MACHINE };

typedef struct
{
    int iPieceNum;
    CPoint ptWhite;
    CPoint ptBlack;
    CPoint ptLastCurPoint;
} STC_REGRET;

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
    void SetVSMode(enumVSMode emVSMode);
    void SetAIDepth(int emAIDepth);

    enumWinFlag GetWinFlag() const;
    enumVSMode GetVSMode() const;
    int GetAIDepth() const;
    BOOL IsBlackTurn() const;
    int GetMoveCount() const;
    CRect GetRectBoard();

private:
    enumChessColor m_iPositionPiece[COLUMNS][ROWS];
    CChessDraw m_chessdraw;
    CRect m_rcBoard;
    CPoint m_ptCurrent;
    enumWinFlag m_emWin;
    enumVSMode m_emVSMode;
    STC_REGRET m_stcLastPos;
    BOOL m_bTurnBlack;
    int m_emAIDepth;

    BOOL IsWin(UINT uiCol, UINT uiRow, enumChessColor emChessColor);
    BOOL GetBestPosByAI(UINT& uiCol, UINT& uiRow, enumChessColor emEnemyChessColor = BLACK);
};
