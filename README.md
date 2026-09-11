# FiveChess

一个基于 **C++ / MFC** 实现的 Windows 五子棋桌面应用，由李源晟完成。项目起于东南大学 2024 年暑期学校课程实践，现已在原有框架上完成界面重构、AI 搜索优化、对局状态完善与仓库工程化整理。

> 当前版本：**2.1** · Visual Studio 2022 · Win32 · MFC

## 功能

- 15 × 15 棋盘与五子连珠胜负判断
- **人机对战**：玩家执黑，AI 执白
- **双人对战**：本地黑白双方轮流落子
- 三档 AI：初级 / 标准 / 高级
- Alpha-Beta 剪枝、候选点生成、邻域裁剪与走法排序
- 多步落子历史与稳定悔棋逻辑
- 终局后仍可悔棋继续对局
- 最近一步红点标记与获胜连线高亮
- 棋盘交叉点悬停预览
- 自适应窗口布局与独立对局信息面板
- 对局模式、AI 难度、落子数和当前回合实时显示
- `Ctrl + Z` 快速悔棋，`F2` / `Ctrl + N` 快速开始新对局
- GitHub Actions 自动验证 `Release | Win32` 构建

## 界面与交互

主窗口采用左侧棋盘、右侧对局信息卡片的布局。棋盘使用木色底面、深色网格和五个标准星位，黑白棋子带有轮廓、阴影与高光。最近一步使用红点标记，形成五连后会额外绘制获胜连线。

操作区统一为 Owner Draw 圆角按钮；“悔棋”在无可撤销落子时自动禁用。设置窗口只保留实际可用的对战模式与 AI 难度，修改设置后直接开始新对局。

鼠标移动到可落子交叉点时会显示半透明感的棋子预览，双人模式下预览颜色随当前回合切换。

## AI

AI 实现在 `FiveChess/ChessAI.cpp`。搜索前先从已有棋子周围生成候选点，再按照进攻价值、防守价值和中心位置进行排序，避免对 15 × 15 棋盘的全部空点进行无差别搜索。

- **初级**：基于候选点启发式评分直接选点，响应最快
- **标准**：2 层 Alpha-Beta 搜索，兼顾进攻与防守
- **高级**：3 层 Alpha-Beta 搜索，并限制高价值候选点数量以控制计算量

局面评分会分别考虑己方成五、活四、冲四、活三等连续棋型，同时将对手在同一位置的潜在威胁纳入候选点优先级。搜索过程优先展开更有价值的走法，以提高 Alpha-Beta 剪枝效率。

## 悔棋与对局状态

每一步落子都会进入历史记录。双人模式每次撤销一步；人机模式会优先撤销一整个“玩家 + AI”回合，如果玩家刚刚形成胜局、AI 尚未落子，则只撤销玩家最后一步。

悔棋会同步恢复当前回合、最近一步标记和胜负状态，因此即使已经弹出终局提示，也可以撤销最后一轮继续下棋。

## 项目结构

```text
FiveChess/
├─ FiveChess.sln
├─ .github/workflows/build.yml          # Windows Release 构建验证
├─ .gitignore
├─ README.md
└─ FiveChess/
   ├─ Chess.cpp / Chess.h               # 对局状态、历史记录与主流程
   ├─ ChessAI.cpp / ChessAI.h           # 候选点、评分与 Alpha-Beta AI
   ├─ ChessCommon.cpp / .h              # 棋型与公共逻辑
   ├─ ChessDraw.cpp / .h                # 棋盘、棋子、预览与获胜连线
   ├─ Gobang_FiveChessDlg.cpp / .h      # 主窗口、布局与快捷键
   ├─ DialogMore.cpp / .h               # 对局设置
   ├─ FaceFunc.cpp / .h                 # GDI 绘制辅助
   ├─ MyMemDC.h                         # 双缓冲绘制
   └─ res/                               # 图标与资源
```

## 模块关系

```mermaid
flowchart LR
    UI[主窗口 / 设置窗口] --> GAME[CChess 对局状态]
    GAME --> HISTORY[落子历史 / 悔棋]
    GAME --> RULE[胜负判断]
    GAME --> AI[候选点 + Alpha-Beta]
    GAME --> DRAW[棋盘 / 棋子 / 标记]
    DRAW --> GDI[GDI / 双缓冲]
```

## 构建

环境：

- Windows 10 / 11
- Visual Studio 2022
- Desktop development with C++
- MFC / ATL support
- Platform Toolset `v143`

步骤：

1. 克隆仓库。
2. 使用 Visual Studio 打开 `FiveChess.sln`。
3. 选择 `Debug | Win32` 或 `Release | Win32`。
4. Build Solution。
5. 运行生成的 `Gobang_FiveChess.exe`。

Release 配置使用静态 MFC，Debug 配置使用动态 MFC。

## 操作

启动后默认进入人机对战，玩家执黑。点击棋盘交叉点落子，AI 自动完成白棋回合。右侧可以开始新对局、悔棋或打开对局设置。

| 操作 | 快捷键 |
| --- | --- |
| 新对局 | `F2` 或 `Ctrl + N` |
| 悔棋 | `Ctrl + Z` |
| 对局设置 | 右侧“对局设置”按钮 |

## 作者

**李源晟**

东南大学 · FiveChess C++ / MFC 项目
