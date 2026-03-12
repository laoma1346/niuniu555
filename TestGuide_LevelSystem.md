# 关卡与流程系统 - 结构化测试指南（修复版）

## 📖 阅读指南

本文档采用**测试用例（TC）+ 日志对照**格式，每个测试步骤都标注了：
- **查看日志**：需要关注的 Log Category 和关键字
- **成功标志**：必须出现的日志行（✅）
- **错误警示**：表示失败的日志行（❌）

### 控制台日志过滤命令

在测试前，建议在控制台（`~`键）输入以下过滤命令，减少干扰信息：

```
; 只看关卡系统相关日志
Log LogLevelGenerator All
Log LogRoomSystem All
Log LogRoomManager All
Log LogCombatRoom All
Log LogTemp off  ; 关闭临时日志，减少干扰

; 如需要看AI相关（战斗房测试时）
Log LogEnemyAI All

; 查看玩家数据系统日志
Log LogPlayerData All
```

---

## 第一部分：环境准备检查

### 前置检查清单（带验证日志）

| 检查项 | 验证命令/操作 | 预期日志（必须出现） |
|--------|--------------|---------------------|
| **项目编译** | 启动编辑器 | `LogModuleManager: Display: Loaded module niuniu555` |
| **导航网格** | 按 `P` 键 | 场景显示绿色覆盖层（非日志，视觉验证） |
| **RoomManager初始化** | 点击 Play | `[RoomManager] 初始化成功` ✅ |
| **PlayerDataSubsystem** | 点击 Play | `[PlayerDataSubsystem] 初始化完成` ✅ |
| **作弊系统** | 输入 `Level.ShowMap` | 显示地图信息 ✅ |

---

## 第二部分：控制台命令汇总

### 基础命令

| 命令 | 参数 | 示例 | 说明 |
|------|------|------|------|
| `Level.GenerateMap` | [Seed] [Layers] | `Level.GenerateMap 123 3` | 生成随机地图 |
| `Level.EnterRoom` | [RoomID] | `Level.EnterRoom 0` | 进入指定房间 |
| `Level.ShowMap` | 无 | `Level.ShowMap` | 显示地图信息 |
| `Level.CompleteRoom` | 无 | `Level.CompleteRoom` | 完成当前房间 |
| `Level.GiveGold` | [Amount] | `Level.GiveGold 500` | 给予金币 |
| `Level.GiveFragments` | [Amount] | `Level.GiveFragments 100` | 给予神格碎片 |

### EventRoom专用命令

| 命令 | 参数 | 示例 | 说明 |
|------|------|------|------|
| `Level.ShowEventInfo` | 无 | `Level.ShowEventInfo` | 显示当前事件信息 |
| `Level.ChooseOption` | [Index] | `Level.ChooseOption 0` | 选择事件选项(0,1,2...) |
| `Level.SkipEvent` | 无 | `Level.SkipEvent` | 跳过当前事件 |

### ShopRoom专用命令

| 命令 | 参数 | 示例 | 说明 |
|------|------|------|------|
| `Level.ShowShopInfo` | 无 | `Level.ShowShopInfo` | 显示商店商品列表 |
| `Level.BuyItem` | [Index] | `Level.BuyItem 0` | 购买指定索引商品 |
| `Level.RefreshShop` | 无 | `Level.RefreshShop` | 刷新商店商品 |
| `Level.FinishShopping` | 无 | `Level.FinishShopping` | 完成购物离开商店 |

---

## 第三部分：基础功能测试（核心系统）

### TC-LVL-001: 房间生命周期与状态机

**测试目标**：验证房间基础状态流转（NotEntered → InProgress → Completed）

**操作步骤**：
1. 在场景放置 `BP_CombatRoom_Test`（或任何RoomBase派生类）
2. 设置 Room ID 为 `0`
3. 点击 Play
4. 控制台输入：`Level.EnterRoom 0`

**查看日志（Log Category: `LogRoomSystem`）**：
```log
; ✅ 成功标志（必须按顺序出现）
[RoomBase] 房间初始化: BP_CombatRoom_Test (类型: 战斗房)
[RoomSystem] [BP_CombatRoom_Test] 进入房间
```

**进阶验证（完成房间）**：
5. 控制台输入：`Level.CompleteRoom`

**预期日志**：
```log
; ✅ 成功标志
[RoomSystem] [BP_CombatRoom_Test] 完成房间
```

---

### TC-LVL-002: 地图生成系统

**测试目标**：验证 LevelGenerator 随机地图生成与 RoomManager 注册

**操作步骤**：
1. 确保场景中有至少1个 RoomBase 派生 Actor（如 CombatRoom）
2. 控制台输入：`Level.GenerateMap 12345 3`

**查看日志**：

**Category 1: `LogLevelGenerator`（地图生成）**：
```log
; ✅ 成功标志
[LevelGenerator] 开始生成地图: 3层, 种子: 12345
[LevelGenerator] 地图生成完成: X个节点  (X应为8-12之间)
```

**Category 2: `LogRoomManager`（房间注册）**：
```log
; ✅ 成功标志
[RoomManager] 注册房间: BP_CombatRoom_Test (ID: 0)
[RoomManager] 地图生成完成: 3层
```

---

## 第四部分：玩家数据子系统测试（新增）

### TC-LVL-PD-001: PlayerDataSubsystem基础功能

**测试目标**：验证金币、神格碎片、生命值的读写操作

**操作步骤**：

1. **给予金币测试**
   ```
   Level.GiveGold 500
   ```

**预期日志**：
```log
[PlayerDataSubsystem] 金币变化: 0 -> 500 (变化: +500)
[Cheat] Gave 500 gold, current: 500
```

2. **给予神格碎片测试**
   ```
   Level.GiveFragments 100
   ```

**预期日志**：
```log
[PlayerDataSubsystem] 神格碎片变化: 0 -> 100 (变化: +100)
[Cheat] Gave 100 fragments, current: 100
```

3. **重复给予验证累加**
   ```
   Level.GiveGold 200
   Level.GiveFragments 50
   ```

**预期日志**：
```log
[PlayerDataSubsystem] 金币变化: 500 -> 700 (变化: +200)
[PlayerDataSubsystem] 神格碎片变化: 100 -> 150 (变化: +50)
```

---

### TC-LVL-PD-002: 跨房间数据保持

**测试目标**：验证房间切换时玩家数据保持

**操作步骤**：
1. 给予一些资源：`Level.GiveGold 300`
2. 进入房间：`Level.EnterRoom 0`
3. 完成房间：`Level.CompleteRoom`
4. 查看数据是否保持

**预期行为**：
- 金币数量在房间切换后应保持不变
- PlayerDataSubsystem会在房间切换时自动保存/恢复数据

---

## 第五部分：战斗房间测试（CombatRoom）

### 🔴 TC-LVL-CR-001: 敌人死亡事件绑定（修复验证）

**测试目标**：验证敌人死亡时能正确触发CombatRoom回调

**操作步骤**：
1. 按TC-LVL-003配置CombatRoom和EnemySpawner
2. 确保WaveConfigs配置了有效的EnemyClass
3. 点击 Play
4. 输入：`Level.EnterRoom 0`
5. 等待敌人生成
6. 击杀所有敌人（手动或使用`Level.KillAllEnemies`）

**查看日志（关键测试）**：
```log
; ✅ 成功标志（必须出现）
[CombatRoom] 绑定敌人死亡事件: BP_TestEnemy_C_1
[CombatRoom] 绑定敌人死亡事件: BP_TestEnemy_C_2
[CombatRoom] 绑定敌人死亡事件: BP_TestEnemy_C_3

; 击杀时
[CombatRoom] 敌人死亡回调触发: BP_TestEnemy_C_1
[CombatRoom] 敌人死亡 BP_TestEnemy_C_1，剩余: 2
[CombatRoom] 敌人死亡回调触发: BP_TestEnemy_C_2
[CombatRoom] 敌人死亡 BP_TestEnemy_C_2，剩余: 1
[CombatRoom] 敌人死亡回调触发: BP_TestEnemy_C_3
[CombatRoom] 敌人死亡 BP_TestEnemy_C_3，剩余: 0

; 最终完成
[CombatRoom] 发放奖励: 金币 100, 经验 50
[CombatRoom] 战斗房间完成
```

**❌ 错误诊断**：
```log
; 如果没有看到"绑定敌人死亡事件"日志
→ 检查：RegisterSpawnedEnemy中是否正确绑定了委托
→ 检查：EnemyBase是否有OnDeath委托

; 如果看到"绑定"日志但看不到"回调触发"日志
→ 检查：敌人死亡时是否调用了OnDeath.Broadcast()
→ 检查：CombatRoom是否在EndPlay前被销毁
```

---

### 🔴 TC-LVL-CR-002: 波次完成判定逻辑（修复验证）

**测试目标**：验证修复后的AreAllWavesComplete逻辑

**测试用例1：无波次配置**
1. 配置CombatType为Normal但不清WaveConfigs（空数组）
2. 进入房间

**预期**：房间应立即完成（无敌人需要击杀）
```log
[CombatRoom] 警告：没有配置波次！请检查 WaveConfigs
[CombatRoom] 战斗房间完成
```

**测试用例2：单波次完成判定**
1. 配置WaveConfigs只有1个波次
2. 配置SpawnGroups生成3个敌人
3. 进入房间，击杀所有敌人

**预期**：击杀最后一个敌人后立即完成
```log
[CombatRoom] 敌人死亡 XXX，剩余: 0
[CombatRoom] 发放奖励: 金币 100, 经验 50
[CombatRoom] 战斗房间完成
```

**测试用例3：多波次切换**
1. 配置WaveConfigs有2个波次
2. 完成第一波后观察第二波是否自动开始

**预期日志**：
```log
; 第一波完成
[CombatRoom] 波次 0 完成
[CombatRoom] 开始波次 1: XXX
```

---

### TC-LVL-CR-003: 波次检查定时器频率（修复验证）

**测试目标**：验证波次检查间隔从1秒改为0.3秒

**操作步骤**：
1. 进入战斗房间
2. 快速击杀所有敌人
3. 观察完成响应时间

**预期**：击杀最后一个敌人后0.3秒内触发完成（而不是1秒）

---

### TC-LVL-003: 基础战斗房配置（单波次）

**测试目标**：验证波次配置解析、敌人生成、完成判定

**详细配置步骤**：

1. **放置EnemySpawner**：在场景中放置2-3个EnemySpawner
2. **放置CombatRoom**：设置RoomID为0
3. **配置WaveConfigs**：
   - WaveName: `第一波敌人`
   - DelayBeforeStart: `3.0`
   - bWaitForClear: `true`
   - SpawnGroups: 配置EnemyClass和Count

**测试步骤**：
1. 点击 Play
2. 输入：`Level.EnterRoom 0`
3. 观察3秒后敌人生成
4. 击杀所有敌人

**预期日志**：
```log
[CombatRoom] 进入战斗房间 (类型: 普通, 波次数: 1)
[CombatRoom] 开始波次 0: 第一波敌人
[CombatRoom] 注册敌人 BP_TestEnemy_C_1，当前存活: 1
[CombatRoom] 绑定敌人死亡事件: BP_TestEnemy_C_1
...
[CombatRoom] 敌人死亡回调触发: BP_TestEnemy_C_1
[CombatRoom] 敌人死亡 BP_TestEnemy_C_1，剩余: 0
[CombatRoom] 发放奖励: 金币 100, 经验 50
[CombatRoom] 战斗房间完成
```

---

## 第六部分：事件房间测试（EventRoom）

### 控制台命令（新增）

| 命令 | 参数 | 说明 |
|------|------|------|
| `Level.ShowEventInfo` | 无 | 显示当前事件信息（事件名称、描述、选项） |
| `Level.ChooseOption [Index]` | Index: 选项索引(0,1,2...) | 选择事件选项 |
| `Level.SkipEvent` | 无 | 跳过当前事件 |

### TC-LVL-ER-001: 玩家数据集成（修复验证）

**测试目标**：验证EventRoom使用PlayerDataSubsystem获取/扣除资源

**操作步骤**：
1. 给予金币：`Level.GiveGold 1000`
2. 进入EventRoom：`	`（假设ID为1）
3. 查看事件信息：`Level.ShowEventInfo`
4. 选择需要金币的选项：`Level.ChooseOption 0`

**预期日志**：
```log
[PlayerDataSubsystem] 金币变化: 1000 -> 900 (变化: -100)
[EventRoom] 选择选项 0: 押注金币
```

**❌ 错误诊断**：
```log
; 如果看到金币未变化
→ 检查：EventRoom是否包含了PlayerDataSubsystem头文件
→ 检查：PlayerDataSubsystem是否正确初始化
```

---

## 第七部分：商店房间测试（ShopRoom）

### 控制台命令（新增）

| 命令 | 参数 | 说明 |
|------|------|------|
| `Level.ShowShopInfo` | 无 | 显示商店信息（商品列表、价格、折扣） |
| `Level.BuyItem [Index]` | Index: 商品索引(0,1,2...) | 购买指定商品 |
| `Level.RefreshShop` | 无 | 刷新商店商品（消耗金币） |
| `Level.FinishShopping` | 无 | 完成购物并关闭商店 |

### 🔴 TC-LVL-SR-001: 商店房完成机制（新增功能）

**测试目标**：验证FinishShopping功能

**操作步骤**：
1. 配置ShopRoom并放置到场景
2. 进入商店房间
3. 查看商品信息：`Level.ShowShopInfo`
4. 购买商品：`Level.BuyItem 0`
5. 完成购物：`Level.FinishShopping`

**蓝图调用方式**：
```
[Get Player Pawn] → [Get Current Room] (Cast to ShopRoom)
    |
    v
[Finish Shopping]
```

**预期日志**：
```log
[ShopRoom] 完成购物，离开商店
[ShopRoom] 关闭商店
[RoomSystem] 完成房间
```

**测试用例2：自动完成**
1. 配置ShopTrigger，使玩家离开后自动调用FinishShopping
2. 进入商店范围 → 离开商店范围
3. 观察房间是否自动完成

---

### TC-LVL-SR-002: 玩家数据集成（修复验证）

**测试目标**：验证商店购买使用PlayerDataSubsystem

**操作步骤**：
1. 给予金币：`Level.GiveGold 500`
2. 进入ShopRoom
3. 尝试购买商品

**预期日志**：
```log
[PlayerDataSubsystem] 金币变化: 500 -> 400 (变化: -100)
[ShopRoom] 购买商品: 生命药水 (价格: 100)
```

**测试用例：金币不足**
1. 设置金币为0：`Level.GiveGold -500`（或使用SetGold）
2. 尝试购买

**预期**：购买失败，金币不足提示
```log
[ShopRoom] 购买失败：金币不足
[PlayerDataSubsystem] 扣除金币失败: 需要 100, 当前 0
```

---

## 第八部分：技能房间测试（SkillRoom）

### TC-LVL-KR-001: 玩家数据集成（修复验证）

**测试目标**：验证SkillRoom使用PlayerDataSubsystem

**操作步骤**：
1. 给予碎片：`Level.GiveFragments 200`
2. 进入SkillRoom
3. 尝试升级技能

**预期日志**：
```log
[PlayerDataSubsystem] 神格碎片变化: 200 -> 150 (变化: -50)
[SkillRoom] 技能升级成功: 0
```

---

## 第九部分：章节配置测试（新增）

### 🔴 TC-LVL-CC-001: ChapterConfigDataAsset使用

**测试目标**：验证RoomManager支持章节配置

**操作步骤**：
1. 在内容浏览器创建 `DA_TestChapter`（ChapterConfigDataAsset）
2. 配置参数：
   - ChapterName: `测试章节`
   - NumLayers: `4`
   - MinRoomsPerLayer: `2`
   - MaxRoomsPerLayer: `3`
   - MinShopRooms: `2`

3. 蓝图调用（或修改测试代码）：
```cpp
// 获取RoomManager和ChapterConfig
URoomManager* RoomManager = URoomManager::Get(this);
UChapterConfigDataAsset* Config = LoadObject<UChapterConfigDataAsset>(...);
RoomManager->GenerateMapFromConfig(Config, 12345);
```

**预期日志**：
```log
[RoomManager] 使用章节配置生成地图: 测试章节
[LevelGenerator] 开始生成地图: 4层, 种子: 12345
[LevelGenerator] 地图生成完成: X个节点
```

**验证要点**：
- 生成的地图层数应为4层
- 商店房数量应至少为2个

---

## 第十部分：稳定性与崩溃修复验证

### 🔴 VF-006: Lambda定时器移除验证

**测试目标**：验证所有Lambda定时器已改为成员函数绑定

**测试步骤**：
1. 测试关卡过渡：`LevelTransitionManager::PlayTransition`
2. 触发房间切换

**预期**：无崩溃，过渡效果正常
```log
[LevelTransitionManager] 播放过渡效果
[LevelTransitionManager] 执行延迟加载
[LevelTransitionManager] 过渡完成
```

---

### 🔴 VF-007: ActiveWaveConfigs内存安全

**测试目标**：验证移除UPROPERTY后的ActiveWaveConfigs

**测试步骤**：
1. 快速多次进入/离开CombatRoom
2. 观察是否有内存相关崩溃

**预期**：无崩溃，无GC相关错误

---

### VF-008: 敌人死亡事件解绑

**测试目标**：验证EndPlay时正确解绑死亡事件

**测试步骤**：
1. 进入CombatRoom
2. 等待敌人生成
3. 停止PIE（强制触发EndPlay）

**预期日志**：
```log
[RoomSystem] 房间销毁
; 无崩溃，无野指针访问错误
```

---

## 第十一部分：性能与压力测试

### TC-PERF-003: PlayerDataSubsystem性能

**测试目标**：验证高频数据操作不会导致性能问题

**测试步骤**：
1. 循环执行大量金币操作：
```
for i in range(100):
    Level.GiveGold 1
    Level.GiveGold -1
```

**预期**：
- 帧率保持稳定
- 无内存泄漏
- 金币数值正确

---

## 附录：修复对照表

| 修复项 | 修复前 | 修复后 | 测试用例 |
|--------|--------|--------|----------|
| Lambda定时器 | 使用Lambda捕获this | 成员函数绑定 | VF-006 |
| 敌人死亡事件 | 未绑定 | 正确绑定OnDeath | TC-LVL-CR-001 |
| 波次判定逻辑 | 边界条件错误 | 完善边界处理 | TC-LVL-CR-002 |
| ActiveWaveConfigs | 错误标记UPROPERTY | 移除UPROPERTY | VF-007 |
| PlayerData | TODO占位 | 完整实现 | TC-LVL-PD-001 |
| 商店完成 | 无法完成房间 | 添加FinishShopping | TC-LVL-SR-001 |
| ChapterConfig | 未使用 | GenerateMapFromConfig | TC-LVL-CC-001 |

---

## 附录：新增Cheat命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `Level.GiveGold [Amount]` | 给予金币 | `Level.GiveGold 500` |
| `Level.GiveFragments [Amount]` | 给予神格碎片 | `Level.GiveFragments 100` |

---

## 文档更新记录

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| 1.0 | 2026-03-09 | 初始版本 |
| 2.0 | 2026-03-09 | 添加崩溃修复验证 |
| 3.0 | 2026-03-10 | 修复后版本：PlayerDataSubsystem、事件绑定、商店完成、章节配置 |

---

*最后更新: 2026-03-10*  
*修复版本: Phase 6 LevelSystem Fix*
