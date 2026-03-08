@echo off
chcp 65001 >nul
echo ========================================
echo   Kimi Code UE5 专家模式启动器
echo ========================================
echo.

REM 检查是否已登录
echo [1/3] 检查登录状态...
kimi info >nul 2>&1
if errorlevel 1 (
    echo [提示] 需要登录Kimi账号
    kimi login
)

REM 设置环境变量
echo [2/3] 加载配置...
set MODEL=kimi-k2-5
set CONFIG=.kimi\ue5-expert.toml
set SESSION=niuniu555-dev

echo [3/3] 启动Kimi...
echo.
echo 模型: %MODEL%
echo 配置: %CONFIG%
echo 会话: %SESSION%
echo.

REM 启动Kimi
kimi --model "%MODEL%" --thinking --agent-file "%CONFIG%" --session "%SESSION%" --continue web

pause
