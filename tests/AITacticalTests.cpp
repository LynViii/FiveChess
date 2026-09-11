#include <cstring>
#include <iostream>

#include "ChessAI.h"

typedef BOOL (*AIFunction)(POINT&, enumChessColor[][ROWS]);

namespace
{
    struct AIEntry
    {
        const char* name;
        AIFunction function;
    };

    void ClearBoard(enumChessColor board[][ROWS])
    {
        for (UINT x = 0; x < COLUMNS; ++x)
        {
            for (UINT y = 0; y < ROWS; ++y)
            {
                board[x][y] = NONE;
            }
        }
    }

    bool BoardsEqual(const enumChessColor lhs[][ROWS], const enumChessColor rhs[][ROWS])
    {
        return std::memcmp(lhs, rhs, sizeof(enumChessColor) * COLUMNS * ROWS) == 0;
    }

    bool IsInside(int x, int y)
    {
        return x >= 0 && x < (int)COLUMNS && y >= 0 && y < (int)ROWS;
    }

    int CountDirection(const enumChessColor board[][ROWS], int x, int y,
        enumChessColor color, int dx, int dy)
    {
        int count = 0;
        x += dx;
        y += dy;
        while (IsInside(x, y) && board[x][y] == color)
        {
            ++count;
            x += dx;
            y += dy;
        }
        return count;
    }

    bool MakesFive(enumChessColor board[][ROWS], const POINT& pt, enumChessColor color)
    {
        if (!IsInside(pt.x, pt.y) || board[pt.x][pt.y] != NONE)
        {
            return false;
        }

        static const int dirs[4][2] = {
            { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 }
        };

        board[pt.x][pt.y] = color;
        bool five = false;
        for (int i = 0; i < 4 && !five; ++i)
        {
            const int count = 1
                + CountDirection(board, pt.x, pt.y, color, dirs[i][0], dirs[i][1])
                + CountDirection(board, pt.x, pt.y, color, -dirs[i][0], -dirs[i][1]);
            five = count >= 5;
        }
        board[pt.x][pt.y] = NONE;
        return five;
    }

    bool RunEmptyBoard(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);

        POINT pt = { -1, -1 };
        return ai.function(pt, board) && pt.x == 7 && pt.y == 7;
    }

    bool RunImmediateWin(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);
        for (int x = 4; x <= 7; ++x)
        {
            board[x][7] = WHITE;
        }

        POINT pt = { -1, -1 };
        return ai.function(pt, board) && MakesFive(board, pt, WHITE);
    }

    bool RunImmediateBlock(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);
        for (int y = 4; y <= 7; ++y)
        {
            board[9][y] = BLACK;
        }
        board[6][6] = WHITE;

        POINT pt = { -1, -1 };
        return ai.function(pt, board) && MakesFive(board, pt, BLACK);
    }

    bool RunDiagonalWin(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);
        for (int i = 4; i <= 7; ++i)
        {
            board[i][i] = WHITE;
        }
        board[10][5] = BLACK;

        POINT pt = { -1, -1 };
        return ai.function(pt, board) && MakesFive(board, pt, WHITE);
    }

    bool RunWinBeforeBlock(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);

        for (int x = 3; x <= 6; ++x)
        {
            board[x][10] = WHITE;
        }
        for (int y = 2; y <= 5; ++y)
        {
            board[11][y] = BLACK;
        }

        POINT pt = { -1, -1 };
        return ai.function(pt, board) && MakesFive(board, pt, WHITE);
    }

    bool RunReturnsLegalMove(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        ClearBoard(board);
        board[7][7] = BLACK;
        board[7][8] = WHITE;
        board[8][7] = BLACK;
        board[6][8] = WHITE;
        board[8][8] = BLACK;

        POINT pt = { -1, -1 };
        return ai.function(pt, board)
            && IsInside(pt.x, pt.y)
            && board[pt.x][pt.y] == NONE;
    }

    bool RunPreservesBoard(const AIEntry& ai)
    {
        enumChessColor board[COLUMNS][ROWS];
        enumChessColor before[COLUMNS][ROWS];
        ClearBoard(board);
        board[7][7] = BLACK;
        board[7][8] = WHITE;
        board[8][7] = BLACK;
        board[6][8] = WHITE;
        board[9][6] = BLACK;
        std::memcpy(before, board, sizeof(board));

        POINT pt = { -1, -1 };
        const BOOL result = ai.function(pt, board);
        return result && BoardsEqual(board, before);
    }

    bool Check(const char* caseName, const AIEntry& ai, bool passed)
    {
        std::cout << (passed ? "[PASS] " : "[FAIL] ")
                  << ai.name << " - " << caseName << std::endl;
        return passed;
    }
}

int main()
{
    const AIEntry ais[] = {
        { "Primary", AIPrimary },
        { "Middle", AIMiddle },
        { "High", AIHigh }
    };

    bool allPassed = true;
    for (int i = 0; i < 3; ++i)
    {
        allPassed &= Check("empty board chooses center", ais[i], RunEmptyBoard(ais[i]));
        allPassed &= Check("takes immediate win", ais[i], RunImmediateWin(ais[i]));
        allPassed &= Check("blocks immediate loss", ais[i], RunImmediateBlock(ais[i]));
        allPassed &= Check("detects diagonal win", ais[i], RunDiagonalWin(ais[i]));
        allPassed &= Check("own win has priority over block", ais[i], RunWinBeforeBlock(ais[i]));
        allPassed &= Check("returns a legal empty point", ais[i], RunReturnsLegalMove(ais[i]));
        allPassed &= Check("search does not mutate board", ais[i], RunPreservesBoard(ais[i]));
    }

    if (!allPassed)
    {
        std::cerr << "AI tactical regression failed." << std::endl;
        return 1;
    }

    std::cout << "All AI tactical regression checks passed." << std::endl;
    return 0;
}
