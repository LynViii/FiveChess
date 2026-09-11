#include "ChessAI.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
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

    enum TTFlag
    {
        TT_EXACT,
        TT_LOWER_BOUND,
        TT_UPPER_BOUND
    };

    struct TTEntry
    {
        int depth;
        long double score;
        TTFlag flag;
    };

    typedef std::unordered_map<std::uint64_t, TTEntry> TranspositionTable;

    void SetPoint(POINT& pt, int x, int y)
    {
        pt.x = x;
        pt.y = y;
    }

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

    long double WindowThreatScore(const enumChessColor board[][ROWS], int moveX, int moveY,
        enumChessColor color)
    {
        static const int dirs[4][2] = {
            { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, -1 }
        };

        long double score = 0.0L;
        for (int dir = 0; dir < 4; ++dir)
        {
            const int dx = dirs[dir][0];
            const int dy = dirs[dir][1];

            // Inspect every five-cell window that contains the candidate move.
            // Unlike the contiguous counter below, this also notices gapped
            // threats such as XX_XX and X_XXX.
            for (int offset = -4; offset <= 0; ++offset)
            {
                const int startX = moveX + offset * dx;
                const int startY = moveY + offset * dy;
                const int endX = startX + 4 * dx;
                const int endY = startY + 4 * dy;
                if (!IsInside(startX, startY) || !IsInside(endX, endY))
                {
                    continue;
                }

                int own = 0;
                int empty = 0;
                bool blocked = false;
                for (int k = 0; k < 5; ++k)
                {
                    const int x = startX + k * dx;
                    const int y = startY + k * dy;
                    const enumChessColor cell = (x == moveX && y == moveY) ? color : board[x][y];

                    if (cell == color)
                    {
                        ++own;
                    }
                    else if (cell == NONE)
                    {
                        ++empty;
                    }
                    else
                    {
                        blocked = true;
                        break;
                    }
                }

                if (blocked)
                {
                    continue;
                }

                if (own == 5)
                {
                    score += WIN_SCORE / 4.0L;
                }
                else if (own == 4 && empty == 1)
                {
                    score += 12000000.0L;
                }
                else if (own == 3 && empty == 2)
                {
                    score += 180000.0L;
                }
                else if (own == 2 && empty == 3)
                {
                    score += 6000.0L;
                }
            }
        }
        return score;
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

        score += WindowThreatScore(board, x, y, color);

        const int center = 7;
        const int distance = std::abs(x - center) + std::abs(y - center);
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

    bool FindImmediateMove(enumChessColor board[][ROWS], enumChessColor color, POINT& pt)
    {
        long double bestPriority = -INF_SCORE;
        bool found = false;

        for (int x = 0; x < (int)COLUMNS; ++x)
        {
            for (int y = 0; y < (int)ROWS; ++y)
            {
                if (board[x][y] != NONE)
                {
                    continue;
                }

                board[x][y] = color;
                const bool wins = IsFiveAfterPlaced(board, x, y, color);
                board[x][y] = NONE;

                if (!wins)
                {
                    continue;
                }

                const long double priority = MovePatternScore(board, x, y, color);
                if (!found || priority > bestPriority)
                {
                    bestPriority = priority;
                    SetPoint(pt, x, y);
                    found = true;
                }
            }
        }

        return found;
    }

    bool FindTacticalMove(enumChessColor board[][ROWS], POINT& pt)
    {
        if (FindImmediateMove(board, WHITE, pt))
        {
            return true;
        }
        return FindImmediateMove(board, BLACK, pt);
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

    std::uint64_t Mix64(std::uint64_t value)
    {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    std::uint64_t PieceKey(int x, int y, enumChessColor color)
    {
        const std::uint64_t index = (std::uint64_t)(y * COLUMNS + x);
        return Mix64(index * 4ULL + (std::uint64_t)color + 0x51ed2705ULL);
    }

    std::uint64_t HashBoard(const enumChessColor board[][ROWS])
    {
        std::uint64_t hash = 0;
        for (int x = 0; x < (int)COLUMNS; ++x)
        {
            for (int y = 0; y < (int)ROWS; ++y)
            {
                if (board[x][y] != NONE)
                {
                    hash ^= PieceKey(x, y, board[x][y]);
                }
            }
        }
        return hash;
    }

    std::uint64_t PositionKey(std::uint64_t boardHash, bool whiteTurn)
    {
        return boardHash ^ Mix64(whiteTurn ? 0x13579bdfULL : 0x2468ace0ULL);
    }

    long double Search(enumChessColor board[][ROWS], int depth,
        long double alpha, long double beta, bool whiteTurn, int candidateLimit,
        std::uint64_t boardHash, TranspositionTable& table)
    {
        if (depth <= 0)
        {
            return EvaluateBoard(board);
        }

        const long double alphaOriginal = alpha;
        const long double betaOriginal = beta;
        const std::uint64_t key = PositionKey(boardHash, whiteTurn);
        TranspositionTable::const_iterator cached = table.find(key);
        if (cached != table.end() && cached->second.depth == depth)
        {
            const TTEntry& entry = cached->second;
            if (entry.flag == TT_EXACT)
            {
                return entry.score;
            }
            if (entry.flag == TT_LOWER_BOUND && entry.score > alpha)
            {
                alpha = entry.score;
            }
            else if (entry.flag == TT_UPPER_BOUND && entry.score < beta)
            {
                beta = entry.score;
            }
            if (alpha >= beta)
            {
                return entry.score;
            }
        }

        const enumChessColor color = whiteTurn ? WHITE : BLACK;
        std::vector<Candidate> candidates;
        BuildCandidates(board, color, candidateLimit, candidates);
        if (candidates.empty())
        {
            return EvaluateBoard(board);
        }

        long double best = whiteTurn ? -INF_SCORE : INF_SCORE;
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            const Candidate& c = candidates[i];
            board[c.x][c.y] = color;
            const std::uint64_t childHash = boardHash ^ PieceKey(c.x, c.y, color);

            long double score;
            if (IsFiveAfterPlaced(board, c.x, c.y, color))
            {
                score = whiteTurn ? (WIN_SCORE + depth * 1000.0L) : (-WIN_SCORE - depth * 1000.0L);
            }
            else
            {
                score = Search(board, depth - 1, alpha, beta, !whiteTurn,
                    candidateLimit, childHash, table);
            }

            board[c.x][c.y] = NONE;

            if (whiteTurn)
            {
                if (score > best)
                {
                    best = score;
                }
                if (best > alpha)
                {
                    alpha = best;
                }
            }
            else
            {
                if (score < best)
                {
                    best = score;
                }
                if (best < beta)
                {
                    beta = best;
                }
            }

            if (alpha >= beta)
            {
                break;
            }
        }

        TTEntry entry;
        entry.depth = depth;
        entry.score = best;
        if (best <= alphaOriginal)
        {
            entry.flag = TT_UPPER_BOUND;
        }
        else if (best >= betaOriginal)
        {
            entry.flag = TT_LOWER_BOUND;
        }
        else
        {
            entry.flag = TT_EXACT;
        }
        table[key] = entry;
        return best;
    }

    BOOL ChooseBySearch(POINT& pt, enumChessColor board[][ROWS], int depth, int candidateLimit)
    {
        if (FindTacticalMove(board, pt))
        {
            return TRUE;
        }

        std::vector<Candidate> candidates;
        BuildCandidates(board, WHITE, candidateLimit, candidates);
        if (candidates.empty())
        {
            SetPoint(pt, -1, -1);
            return FALSE;
        }

        TranspositionTable table;
        table.reserve(4096);
        const std::uint64_t rootHash = HashBoard(board);

        long double bestScore = -INF_SCORE;
        SetPoint(pt, candidates[0].x, candidates[0].y);

        for (size_t i = 0; i < candidates.size(); ++i)
        {
            const Candidate& c = candidates[i];
            board[c.x][c.y] = WHITE;
            const std::uint64_t childHash = rootHash ^ PieceKey(c.x, c.y, WHITE);

            long double score;
            if (IsFiveAfterPlaced(board, c.x, c.y, WHITE))
            {
                score = WIN_SCORE;
            }
            else
            {
                score = Search(board, depth - 1, -INF_SCORE, INF_SCORE, false,
                    candidateLimit, childHash, table);
            }

            board[c.x][c.y] = NONE;
            if (score > bestScore)
            {
                bestScore = score;
                SetPoint(pt, c.x, c.y);
            }
        }

        return TRUE;
    }
}

BOOL AIPrimary(POINT& pt, enumChessColor emChess[][ROWS])
{
    if (FindTacticalMove(emChess, pt))
    {
        return TRUE;
    }

    std::vector<Candidate> candidates;
    BuildCandidates(emChess, WHITE, 14, candidates);
    if (candidates.empty())
    {
        SetPoint(pt, -1, -1);
        return FALSE;
    }

    SetPoint(pt, candidates[0].x, candidates[0].y);
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
