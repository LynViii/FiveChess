#include "StdAfx.h"
#include "ChessAI.h"

#include <algorithm>
#include <vector>

namespace
{
    const long double WIN_SCORE = 1000000000000.0L;
    const long double INF_SCORE = 1000000000000000.0L;

    struct Candidate
    {
        int x;
        int y;
        long double priority;
    };

    bool CandidateGreater(const Candidate& lhs, const Candidate& rhs)
    {
        return lhs.priority > rhs.priority;
    }

    bool IsInside(int x, int y)
    {
        return x >= 0 && x < (int)COLUMNS && y >= 0 && y < (int)ROWS;
    }

    bool IsBoardEmpty(const enumChessColor board[][ROWS])
    {
        for (int x = 0; x < (int)COLUMNS; ++x)
        {
            for (int y = 0; y < (int)ROWS; ++y)
            {
                if (board[x][y] != NONE)
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool HasNeighbor(const enumChessColor board[][ROWS], int x, int y, int radius)
    {
        for (int dx = -radius; dx <= radius; ++dx)
        {
            for (int dy = -radius; dy <= radius; ++dy)
            {
                if (dx == 0 && dy == 0)
                {
                    continue;
                }

                const int nx = x + dx;
                const int ny = y + dy;
                if (IsInside(nx, ny) && board[nx][ny] != NONE)
                {
                    return true;
                }
            }
        }
        return false;
    }

    int CountOneSide(const enumChessColor board[][ROWS], int x, int y,
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

    int CountOpenEnd(const enumChessColor board[][ROWS], int x, int y,
        enumChessColor color, int dx, int dy)
    {
        x += dx;
        y += dy;
        while (IsInside(x, y) && board[x][y] == color)
        {
            x += dx;
            y += dy;
        }
        return IsInside(x, y) && board[x][y] == NONE ? 1 : 0;
    }

    long double LineScore(int count, int openEnds)
    {
        if (count >= 5)
        {
            return WIN_SCORE;
        }
        if (count == 4)
        {
            return openEnds == 2 ? 50000000.0L : (openEnds == 1 ? 8000000.0L : 250000.0L);
        }
        if (count == 3)
        {
            return openEnds == 2 ? 900000.0L : (openEnds == 1 ? 150000.0L : 8000.0L);
        }
        if (count == 2)
        {
            return openEnds == 2 ? 35000.0L : (openEnds == 1 ? 7000.0L : 500.0L);
        }
        if (count == 1)
        {
            return openEnds == 2 ? 1200.0L : 150.0L;
        }
        return 0.0L;
    }

    long double MovePatternScore(const enumChessColor board[][ROWS], int x, int y,
        enumChessColor color)
    {
        if (!IsInside(x, y) || board[x][y] != NONE)
        {
            return -INF_SCORE;
        }

        static const int dirs[4][2] = {
            { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 }
        };

        long double score = 0.0L;
        for (int i = 0; i < 4; ++i)
        {
            const int dx = dirs[i][0];
            const int dy = dirs[i][1];
            const int count = 1
                + CountOneSide(board, x, y, color, dx, dy)
                + CountOneSide(board, x, y, color, -dx, -dy);
            const int openEnds =
                CountOpenEnd(board, x, y, color, dx, dy)
                + CountOpenEnd(board, x, y, color, -dx, -dy);
            score += LineScore(count, openEnds);
        }

        const int center = 7;
        const int distance = abs(x - center) + abs(y - center);
        score += (14 - distance) * 12.0L;
        return score;
    }

    bool IsFiveAfterPlaced(const enumChessColor board[][ROWS], int x, int y,
        enumChessColor color)
    {
        static const int dirs[4][2] = {
            { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 }
        };

        for (int i = 0; i < 4; ++i)
        {
            const int count = 1
                + CountOneSide(board, x, y, color, dirs[i][0], dirs[i][1])
                + CountOneSide(board, x, y, color, -dirs[i][0], -dirs[i][1]);
            if (count >= 5)
            {
                return true;
            }
        }
        return false;
    }

    void BuildCandidates(const enumChessColor board[][ROWS], enumChessColor color,
        int limit, std::vector<Candidate>& out)
    {
        out.clear();

        if (IsBoardEmpty(board))
        {
            Candidate center = { 7, 7, WIN_SCORE / 1000000.0L };
            out.push_back(center);
            return;
        }

        const enumChessColor opponent = (color == WHITE) ? BLACK : WHITE;
        for (int x = 0; x < (int)COLUMNS; ++x)
        {
            for (int y = 0; y < (int)ROWS; ++y)
            {
                if (board[x][y] != NONE || !HasNeighbor(board, x, y, 2))
                {
                    continue;
                }

                const long double attack = MovePatternScore(board, x, y, color);
                const long double defend = MovePatternScore(board, x, y, opponent);

                Candidate candidate;
                candidate.x = x;
                candidate.y = y;
                candidate.priority = attack + defend * 0.96L;
                out.push_back(candidate);
            }
        }

        std::sort(out.begin(), out.end(), CandidateGreater);
        if (limit > 0 && (int)out.size() > limit)
        {
            out.resize(limit);
        }
    }

    long double EvaluateBoard(const enumChessColor board[][ROWS])
    {
        if (IsBoardEmpty(board))
        {
            return 0.0L;
        }

        long double whiteBest = 0.0L;
        long double whiteSecond = 0.0L;
        long double blackBest = 0.0L;
        long double blackSecond = 0.0L;

        for (int x = 0; x < (int)COLUMNS; ++x)
        {
            for (int y = 0; y < (int)ROWS; ++y)
            {
                if (board[x][y] != NONE || !HasNeighbor(board, x, y, 2))
                {
                    continue;
                }

                const long double white = MovePatternScore(board, x, y, WHITE);
                const long double black = MovePatternScore(board, x, y, BLACK);

                if (white > whiteBest)
                {
                    whiteSecond = whiteBest;
                    whiteBest = white;
                }
                else if (white > whiteSecond)
                {
                    whiteSecond = white;
                }

                if (black > blackBest)
                {
                    blackSecond = blackBest;
                    blackBest = black;
                }
                else if (black > blackSecond)
                {
                    blackSecond = black;
                }
            }
        }

        return (whiteBest + whiteSecond * 0.30L)
            - (blackBest + blackSecond * 0.30L) * 1.04L;
    }

    long double Search(enumChessColor board[][ROWS], int depth,
        long double alpha, long double beta, bool whiteTurn, int candidateLimit)
    {
        if (depth <= 0)
        {
            return EvaluateBoard(board);
        }

        const enumChessColor color = whiteTurn ? WHITE : BLACK;
        std::vector<Candidate> candidates;
        BuildCandidates(board, color, candidateLimit, candidates);
        if (candidates.empty())
        {
            return EvaluateBoard(board);
        }

        if (whiteTurn)
        {
            long double best = -INF_SCORE;
            for (size_t i = 0; i < candidates.size(); ++i)
            {
                const Candidate& c = candidates[i];
                board[c.x][c.y] = WHITE;

                long double score;
                if (IsFiveAfterPlaced(board, c.x, c.y, WHITE))
                {
                    score = WIN_SCORE + depth * 1000.0L;
                }
                else
                {
                    score = Search(board, depth - 1, alpha, beta, false, candidateLimit);
                }

                board[c.x][c.y] = NONE;
                if (score > best)
                {
                    best = score;
                }
                if (best > alpha)
                {
                    alpha = best;
                }
                if (alpha >= beta)
                {
                    break;
                }
            }
            return best;
        }

        long double best = INF_SCORE;
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            const Candidate& c = candidates[i];
            board[c.x][c.y] = BLACK;

            long double score;
            if (IsFiveAfterPlaced(board, c.x, c.y, BLACK))
            {
                score = -WIN_SCORE - depth * 1000.0L;
            }
            else
            {
                score = Search(board, depth - 1, alpha, beta, true, candidateLimit);
            }

            board[c.x][c.y] = NONE;
            if (score < best)
            {
                best = score;
            }
            if (best < beta)
            {
                beta = best;
            }
            if (alpha >= beta)
            {
                break;
            }
        }
        return best;
    }

    BOOL ChooseBySearch(POINT& pt, enumChessColor board[][ROWS], int depth, int candidateLimit)
    {
        std::vector<Candidate> candidates;
        BuildCandidates(board, WHITE, candidateLimit, candidates);
        if (candidates.empty())
        {
            pt = CPoint(-1, -1);
            return FALSE;
        }

        long double bestScore = -INF_SCORE;
        pt = CPoint(candidates[0].x, candidates[0].y);

        for (size_t i = 0; i < candidates.size(); ++i)
        {
            const Candidate& c = candidates[i];
            board[c.x][c.y] = WHITE;

            long double score;
            if (IsFiveAfterPlaced(board, c.x, c.y, WHITE))
            {
                score = WIN_SCORE;
            }
            else
            {
                score = Search(board, depth - 1, -INF_SCORE, INF_SCORE, false, candidateLimit);
            }

            board[c.x][c.y] = NONE;
            if (score > bestScore)
            {
                bestScore = score;
                pt = CPoint(c.x, c.y);
            }
        }

        return TRUE;
    }
}

BOOL AIPrimary(POINT& pt, enumChessColor emChess[][ROWS])
{
    std::vector<Candidate> candidates;
    BuildCandidates(emChess, WHITE, 14, candidates);
    if (candidates.empty())
    {
        pt = CPoint(-1, -1);
        return FALSE;
    }

    pt = CPoint(candidates[0].x, candidates[0].y);
    return TRUE;
}

BOOL AIMiddle(POINT& pt, enumChessColor emChess[][ROWS])
{
    return ChooseBySearch(pt, emChess, 2, 10);
}

BOOL AIHigh(POINT& pt, enumChessColor emChess[][ROWS])
{
    return ChooseBySearch(pt, emChess, 3, 8);
}
