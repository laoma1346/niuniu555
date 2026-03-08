# Phase 4.3 敌人生成器测试指南

## 📋 功能概述

敌人生成器系统已完成基础功能，支持**两种工作模式**：

| 模式 | 组件 | 特点 | 适用场景 |
|------|------|------|----------|
| **AutoSpawn** | 仅 `EnemySpawnPoint` | 独立工作，持续自动生成 | 固定刷新点、快速验证 |
| **波次模式** | `EnemySpawner` + `EnemySpawnPoint` | 触发式波次管理 | 副本、关卡节奏控制 |

---

## 🚀 三种测试模式（按需选择）

### 🔥 模式A：零配置快速测试（AutoSpawn）- 2分钟验证

**适用场景**：快速验证生成点功能是否正常，无需配置波次、无需触发区域

#### 配置步骤

1. **拖放即测**
   - 在场景任意位置右键 → **Place Actor** → **Empty Actor**
   - 选中 Actor，在 **Details** 面板点击 **+ Add** → 添加 **Enemy Spawn Point** 组件

2. **关键配置（仅3项）**
   | 参数 | 设置值 | 说明 |
   |------|--------|------|
   | **Auto Spawn**（自动生成） | ✅ 勾选 | 开启独立生成模式 |
   | **Enemy Class**（敌人类型） | `BP_TestEnemy` | 选择要生成的敌人蓝图 |
   | **Max Alive Count** | `3` | 该点最多同时存在3个敌人 |

3. **点击 Play 运行**

#### 预期结果

```
[LogEnemySpawn] [EnemySpawnPoint] 初始化完成
[LogEnemySpawn] 生成敌人: BP_TestEnemy_C_0
[LogEnemySpawn] 生成敌人: BP_TestEnemy_C_1
[LogEnemySpawn] 生成敌人: BP_TestEnemy_C_2
```

- 每隔 **10秒**（默认间隔）自动生成一个敌人
- 敌人正常执行AI（巡逻/追击/攻击）
- 当存在敌人数量达到 **Max Alive Count** 时暂停生成，杀死敌人后继续

#### ✅ 模式A检查清单

- [ ] Enemy Spawn Point 组件已添加
- [ ] **Auto Spawn** 已勾选（关键！）
- [ ] **Enemy Class** 已选择（非None）
- [ ] **Max Alive Count** ≥ 1

---

### 🎯 模式B：标准波次测试（EnemySpawner）- 完整功能

**适用场景**：测试波次管理、触发区域、最大存活数等完整功能

#### 简化配置流程

**步骤1：创建生成器（1分钟）**

1. 右键场景 → **Place Actor** → 搜索 **Enemy Spawner**
2. 调整 **Trigger Volume** 大小：
   - 在 **Details** 面板找到 **Trigger Volume** → **Box Extent**
   - 默认 `500 x 500 x 200`，根据需要调整（这是玩家触发区域）

**步骤2：添加生成点（三选一，推荐方案1）**

| 方案 | 操作步骤 | 适用场景 |
|------|----------|----------|
| **方案1（推荐）** | ① 右键 **EnemySpawner** → **Attach To** → **Create Child Actor**<br>② 在新Actor上添加 **Enemy Spawn Point** 组件 | 生成点跟随生成器移动，管理方便 |
| 方案2 | ① 拖入 **Empty Actor** → 添加 **Enemy Spawn Point** 组件<br>② 在 **World Outliner** 中拖到 **EnemySpawner** 下成为子级 | 已有现成的生成点Actor |
| 方案3 | 将带 **EnemySpawnPoint** 的Actor放在 **EnemySpawner 周围 20米内** | 快速放置，自动收集 |

**步骤3：配置波次（最小配置，2分钟）**

1. 选中 **EnemySpawner**
2. 在 **Details** 面板找到 **Wave Configs** → 点击 **+** 添加元素（元素0）
3. 配置第一波：
   - **Enemy Classes** → 点击 **+** → 选择 `BP_TestEnemy`
   - **Enemy Count** = `3`
   - 其他保持默认
4. （可选）点击 **+** 再添加第二波（元素1），配置不同数量

**步骤4：测试**

1. 点击 **Play** 运行
2. 控制玩家角色走进 **Trigger Volume**（Box区域）
3. 观察日志和场景

#### 预期日志输出

```
[LogEnemySpawner] [EnemySpawner] 初始化完成，收集到 1 个生成点
[LogEnemySpawner] 玩家触发生成器 EnemySpawner_0
[LogEnemySpawner] [EnemySpawner] 开始生成，波次数: 2
[LogEnemySpawner] 开始波次 0
[LogEnemySpawner] 波次 0 生成敌人 1/3: BP_TestEnemy_C_0
[LogEnemySpawner] 波次 0 生成敌人 2/3: BP_TestEnemy_C_1
[LogEnemySpawner] 波次 0 生成敌人 3/3: BP_TestEnemy_C_2
[LogEnemySpawner] 波次 0 完成
[LogEnemySpawner] 开始波次 1
...
```

#### ✅ 模式B检查清单

- [ ] **EnemySpawner** Actor 已放置到场景
- [ ] 至少 **1个生成点** 已关联（子级或20米内）
- [ ] **WaveConfigs** 数组至少配置了 **1个元素**
- [ ] **WaveConfigs[0].EnemyClasses** 至少选择了 **1个敌人类型**

---

### 💥 模式C：压力测试（多生成点）

**适用场景**：测试性能、多路出兵、包围战术

#### 配置步骤

1. 在 **EnemySpawner** 下创建 **3-5个子 Actor**，每个都添加 **EnemySpawnPoint** 组件
2. 设置不同的 **Spawn Radius**（生成半径）形成包围圈：
   - 生成点1：Radius = `0`（中心）
   - 生成点2：Radius = `300`（东侧）
   - 生成点3：Radius = `300`（西侧）
   - 生成点4：Radius = `500`（北侧）
   - 生成点5：Radius = `500`（南侧）
3. 配置波次：
   - **Enemy Count** = `10`
   - **Spawn Interval** = `0.5`（快速生成）
   - **Max Alive Enemies** = `20`（提高上限）

#### 预期结果

- 敌人从不同方向同时生成
- 测试多实例并发性能（NodeMemory隔离）
- 观察是否有卡顿或逻辑冲突

---

## 📖 功能详解

### Auto Spawn（自动生成）功能

```cpp
// EnemySpawnPoint.h 中的配置属性
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "自动生成")
bool bAutoSpawn = false;  // 勾选后无需Spawner即可独立工作

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "自动生成")
float AutoSpawnInterval = 10.0f;  // 生成间隔（秒）
```

#### 使用场景

- **独立刷新点**：场景中的固定刷新点（如据点、营地），不需要玩家触发，持续刷怪
- **后台压力源**：配合 **Max Alive Count** 限制，保持场景内始终有 N 个敌人游荡
- **快速原型验证**：不用配置 Spawner 即可测试敌人生成和AI行为

#### 两种模式对比

| 特性 | AutoSpawn 模式 | Spawner 波次模式 |
|------|----------------|------------------|
| **触发方式** | 自动持续生成 | 玩家进入触发区域/手动开始 |
| **波次控制** | 无（持续生成） | 有（分波次，可配置延迟） |
| **生成点管理** | 单个独立工作 | 多个由Spawner统一管理 |
| **事件通知** | 仅生成事件 | 波次开始/完成/所有完成 |
| **配置复杂度** | 极低（勾选即可） | 中等（需配置波次） |
| **适用场景** | 固定刷新点、环境压力 | 副本、关卡节奏控制 |

---

## ⚙️ 配置参数详解

### EnemySpawnPoint 组件默认值

| 参数 | 默认值 | 说明 | 建议 |
|------|--------|------|------|
| **Spawn Radius** | `0` | 生成随机偏移半径 | 保持0（精确位置）或设置200-500（随机分布） |
| **Spawn Height Offset** | `100` | 向上偏移防止卡地面 | 通常无需修改，地形不平可增加到150-200 |
| **Cooldown Time** | `5.0` | 同一位置生成间隔（秒） | 保持5秒，防止连续生成 |
| **Max Alive Count** | `1` | 该点最多同时存在数 | 单个刷新点建议1-3，群体刷新点建议5-10 |
| **Auto Spawn** | `false` | 独立生成模式 | 无需Spawner时勾选 |
| **Auto Spawn Interval** | `10.0` | 自动生成间隔（秒） | 根据节奏调整，建议5-15秒 |

### EnemySpawner 默认值

| 参数 | 默认值 | 说明 | 建议 |
|------|--------|------|------|
| **Max Alive Enemies** | `10` | 场景内最多同时存在数 | 根据性能调整，建议5-20 |
| **Wave Start Delay** | `3.0` | 波次开始前等待（秒） | 给玩家准备时间，建议2-5秒 |
| **b Wait For Clear** | `true` | 必须清完当前波次才刷下一波 | 推荐保持开启，避免敌人堆积 |
| **b Use Trigger Volume** | `true` | 使用触发区域 | 玩家触发时开启，或关闭后使用bAutoStart |

---

## ✅ 功能验证清单

| 功能 | 验证方法 | 预期结果 |
|------|----------|----------|
| **AutoSpawn独立工作** | 模式A配置后Play | 无需Spawner，自动生成敌人 |
| **触发区域检测** | 玩家进入/离开Box区域 | 进入时日志显示"玩家触发" |
| **波次生成** | 观察日志和场景 | 按配置数量生成，间隔正确 |
| **波次切换** | 杀死所有敌人或等待 | 当前波次完成后自动开始下一波 |
| **生成点随机选择** | 多个生成点时观察 | 敌人在不同位置生成 |
| **最大存活限制** | 设置MaxAliveEnemies为2 | 同时存在的敌人不超过2个 |
| **冷却时间** | 检查SpawnPoint的Cooldown | 同一位置不会连续生成 |
| **所有波次完成** | 等待最后波次结束 | 日志显示"所有波次完成"，停止生成 |

---

## 🔧 蓝图事件绑定（可选）

可以在蓝图中绑定生成器事件：

1. 选中场景中的 **EnemySpawner**
2. 在 **Details** 面板找到 **Events** 部分：
   - **On Wave Started** - 波次开始事件
   - **On Wave Completed** - 波次完成事件
   - **On All Waves Completed** - 所有波次完成事件
   - **On Enemy Spawned** - 敌人生成事件

3. 点击事件旁边的 **+** 按钮创建绑定
4. 在事件图表中实现自定义逻辑（如播放音效、显示UI等）

---

## 🐛 常见问题排查

### 问题1：生成器不工作

**排查步骤**：
1. 检查 **WaveConfigs** 是否配置了敌人类型
2. 检查 **EnemyClass** 在生成点中是否正确设置
3. 检查触发区域 **bUseTriggerVolume** 和 **bAutoStart** 设置
4. 查看日志 `[LogEnemySpawner]` 输出

### 问题2：敌人生成位置不正确

**解决方案**：
1. 调整 **SpawnHeightOffset**（默认100，增加防止卡地面）
2. 调整 **SpawnRadius**（0表示精确位置，>0表示随机偏移）
3. 确保生成点Actor位置正确

### 问题3：生成点没有被收集

**解决方案**：
1. 确保生成点作为生成器的子Actor，或在 **20米** 范围内（不是2米）
2. 检查生成点是否有 **EnemySpawnPoint** 组件
3. 查看日志是否显示"收集到 X 个生成点"

### 问题4：编辑器崩溃（已修复）

**崩溃位置**：`EnemySpawner::BeginPlay()` → `CollectSpawnPoints()`

**崩溃原因**：代码中错误使用 `GetOwner()->GetComponents()`，在Actor类中 `GetOwner()` 返回 nullptr

**修复状态**：✅ 已修复（2026-03-08）
- 将 `GetOwner()->GetComponents()` 改为 `GetComponents()`
- 重新编译后即可正常使用

### 问题4：波次不切换

**排查步骤**：
1. 检查 **bWaitForClear** 设置
   - `true`：需要杀死所有敌人才会切换
   - `false`：生成完数量后立即切换
2. 检查敌人是否正确死亡（调用Destroy）
3. 查看日志是否显示"波次 X 完成"

---

## 📊 性能建议

1. **生成点数量**：每个生成器建议 1-5 个生成点
2. **最大存活数**：根据场景大小设置，建议 5-20
3. **生成间隔**：建议 > 1.0 秒，避免瞬间生成大量敌人
4. **触发区域大小**：根据关卡设计调整，不宜过大

---

## 🎯 下一步开发（Phase 4.3 剩余）

基础功能已可用，可选扩展：

1. **波次配置数据资产** - 将波次配置抽取为独立的 DataAsset，便于复用
2. **生成特效** - 添加粒子效果（黑雾/光芒）和音效
3. **动态难度调整** - 根据玩家强度调整敌人数量和属性
4. **稀有敌人刷出** - 添加精英怪生成概率控制

---

## 📁 相关文件

| 文件 | 说明 |
|------|------|
| `EnemySpawnPoint.h/cpp` | 生成点组件（支持AutoSpawn独立模式） |
| `EnemySpawner.h/cpp` | 生成器管理器（波次管理） |
| `TestGuide_EnemySpawner.md` | 本测试指南 |

---

*最后更新：2026-03-08*  
*版本：Phase 4.3 基础功能（支持AutoSpawn + Spawner波次两种模式）*
