#pragma once
#include "ChessCommon.h"

// AI 固定执白。
BOOL AIPrimary(POINT& pt, enumChessColor emChess[][ROWS]);
BOOL AIMiddle(POINT& pt, enumChessColor emChess[][ROWS]);
BOOL AIHigh(POINT& pt, enumChessColor emChess[][ROWS]);
