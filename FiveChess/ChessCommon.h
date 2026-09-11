#pragma once

#include <windows.h>

const int MAXCREASE = 4;
const UINT COLUMNS = 15;
const UINT ROWS = 15;

enum enumChessColor
{
    NONE,
    WHITE,
    BLACK
};

void SameColorAdder(
    UINT uiCol,
    UINT uiRow,
    enumChessColor emChessColor,
    BOOL bResetCount,
    int& iColorCount,
    BOOL& bOver,
    const enumChessColor iChess[][ROWS],
    int* piBlankCount = NULL);

void GetSameColor(
    UINT uiCol,
    UINT uiRow,
    enumChessColor emChessColor,
    int* iSameColor,
    const enumChessColor iChess[][ROWS],
    int* piCreaseDirect = NULL,
    BOOL bIgnoreBlank = FALSE,
    int* piBlankFlag = NULL);

BOOL PtInBroad(UINT uiCol, UINT uiRow);
