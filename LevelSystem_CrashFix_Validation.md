# Phase 6 LevelSystem 崩溃修复验证清单

## 修复概述

**崩溃类型**: EXCEPTION_ACCESS_VIOLATION at 0x000000000000002c  
**根本原因**: 初始化时序竞争条件  
**修复策略**: 延迟初始化 + 空指针检查 + IsValid验证  
**编译状态**: ✅ 成功

---

## 快速验证步骤（5分钟）

### Step 1: 编译验证 ✅

```bash
# 编译命令（已执行）
Build.bat niuniu555Editor Win64 Development -Project=niuniu555.uproject
```

**预期结果**: `Result: Succeeded`

---

### Step 2: 编辑器启动测试

1. 双击 `niuniu555.uproject` 启动编辑器
2. **观察**: 是否崩溃？

**✅ 通过**: 编辑器正常启动到主界面  
**❌ 失败**: 启动时崩溃 → 查看 `Saved\Logs\niuniu555.log` 最后50行

---

### Step 3: 房间系统功能测试

1. 打开任意测试关卡（如 `TestMap`）
2. 在场景中放置 `BP_CombatRoom`（如有）或任意Room子类
3. 点击 **Play (PIE)**
4. 打开输出日志窗口（Window → Developer Tools → Output Log）

**预期日志**（按时间顺序）：
```log
[RoomManager] 初始化完成
[RoomSystem] 房间初始化: XXX (类型: Combat)
[RoomSystem] 房间注册到管理器成功
[CombatRoom] 找到 X 个敌人生成器  ; ← 延迟约1秒后
```

**✅ 通过**: 所有日志正常出现，无红色Error  
**❌ 失败**: 缺少某条日志或有Error → 查看测试文档故障排查章节

---

### Step 4: 调试命令测试

在编辑器控制台（按 `~` 键）输入：

```
Level.ShowMap
```

**预期输出**：
```
========== 地图信息 ==========
地图已生成: Yes/No
总房间数: X
当前房间: Y
=============================
```

**✅ 通过**: 命令执行无崩溃，有输出  
**❌ 失败**: 崩溃或无输出

---

### Step 5: 房间切换测试

```
Level.GenerateMap 12345 3
Level.EnterRoom 0
```

**预期行为**：
1. 地图生成成功
2. 进入房间0无崩溃
3. 日志显示 `[RoomSystem] 进入房间`

---

## 详细验证（完整测试）

参考完整测试文档：`TestGuide_LevelSystem.md`

重点执行：
- 第九部分：崩溃修复验证（VF-001 ~ VF-005）
- TC-LVL-001 到 TC-LVL-003

---

## 常见问题速查

### Q: 编辑器启动时仍然崩溃？

**排查步骤**：
1. 检查是否编译了 Development Editor 配置
2. 删除 `Intermediate` 和 `Binaries` 文件夹后重新编译
3. 检查日志中崩溃前的最后一行

### Q: `[CombatRoom] 找到 0 个敌人生成器`？

**解决**：
- 在场景中手动放置 `BP_EnemySpawner`（在 CombatRoom 20米范围内）
- 或使用蓝图在 BeginPlay 中手动注册：`CombatRoom->RegisterEnemySpawner(Spawner)`

### Q: `[RoomManager] 初始化失败：World/GameInstance 为 nullptr`？

**分析**：这是一个保护性日志，说明防御代码生效了。但正常情况下不应出现。

**排查**：
- 检查引擎启动流程
- 确认GameInstance类设置正确

---

## 代码变更摘要

| 文件 | 关键变更 |
|------|----------|
| `RoomManager.h` | 添加 `bool bInitialized = false` |
| `RoomManager.cpp` | 延迟创建LevelGenerator；添加空指针检查；使用IsValid |
| `RoomBase.h` | 变量名 `RoomManager` → `RoomManagerPtr` |
| `RoomBase.cpp` | 延迟0.2秒注册；缓存RoomManagerPtr；EndPlay使用缓存注销 |
| `CombatRoom.cpp` | 延迟1.0秒查找EnemySpawner；添加IsValid检查 |

---

## 验证结果记录

| 检查项 | 状态 | 时间 | 备注 |
|--------|------|------|------|
| 编译成功 | ⬜ | | |
| 编辑器启动 | ⬜ | | |
| RoomManager初始化 | ⬜ | | |
| 房间注册成功 | ⬜ | | |
| CombatRoom查找生成器 | ⬜ | | |
| 调试命令执行 | ⬜ | | |
| 房间切换无崩溃 | ⬜ | | |

**测试人员**: ___________  
**测试日期**: ___________  
**UE版本**: 5.6  
**项目版本**: Phase 6 + 崩溃修复

---

*文档版本: 1.0*  
*最后更新: 2026-03-09*
