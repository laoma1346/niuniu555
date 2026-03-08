# AI行为树系统测试指南

## 概述

本文档指导如何测试和验证 **Phase 4.2 AI行为树系统** 的功能。AI行为树系统采用 **NodeMemory架构** 确保多线程安全和多实例独立。

---

## 2026-03-08 重要修复更新

### 修复摘要

本次更新修复了以下关键问题：

#### 1. 攻击任务动画卡住问题 ✅

**问题原因**：
- 动画播放状态检查不完善，当动画实例失效或动画播放时间过长时，任务无法正确结束
- 后摇等待逻辑没有处理动画异常的情况
- 超时后返回 Failed 导致行为树频繁重试

**修复内容**：
- 添加 `AnimInstance` 弱引用缓存，避免频繁获取
- 添加动画播放时间检测，超过动画长度+0.5秒强制结束
- 超时时间从10秒缩短为5秒
- 超时后返回 **Succeeded** 而非 Failed，避免行为树震荡
- 修复 `FMath::Acos` 输入域错误（添加 Clamp）

#### 2. 攻击-追击循环震荡问题 ✅

**问题原因**：
- 追击任务在目标进入攻击范围后，仍检查目标是否静止才返回成功
- 目标轻微移动导致追击任务反复进入/退出
- 距离检查装饰器与攻击任务的攻击范围不一致

**修复内容**：
- 移除"目标静止"检查，进入攻击范围立即返回成功
- 添加 `bHasEnteredAttackRange` 标志，避免重复判断
- 统一使用平方距离计算（性能优化）
- 追击任务进入攻击范围后停止移动并立即返回

#### 3. 日志编译错误修复 ✅

**问题原因**：
- `DEFINE_LOG_CATEGORY` 在 .cpp 中使用，但头文件没有 `DECLARE_LOG_CATEGORY_EXTERN` 声明
- 其他模块引用日志分类时编译失败

**修复内容**：
- 在 `EnemyBase.h` 添加 `DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All)`
- 在 `EnemyAIController.h` 添加 `DECLARE_LOG_CATEGORY_EXTERN(LogEnemyAI, Log, All)`

#### 4. 黑板枚举暴露方式（已知限制）

**当前状态**：
- `EAIBehaviorState` 枚举已移动到 `Public/AI/` 目录
- 已添加配套 `.cpp` 文件触发反射生成
- **但蓝图编辑器可能仍无法在下拉列表中显示**

**解决方案**：
方法一：手动输入枚举名称
1. 创建黑板 Key，选择 Enum 类型
2. 不点击下拉列表，直接在 **Enum Type** 字段输入：`EAIBehaviorState`
3. 如果字段保持白色（不变红），说明类型已识别

方法二：使用字符串替代（推荐用于测试）
1. 将 `AIState` Key 类型改为 **String** 而非 Enum
2. 在代码中会自动设置状态名称字符串

方法三：重启编辑器并刷新
1. 删除 `Intermediate/Build/Win64/UE5Editor/Inc/niuniu555` 目录
2. 重新编译
3. 重启 UE5 编辑器

---

## 推荐的简化行为树结构

为避免攻击-追击震荡，建议使用以下行为树结构：

```
[Root]
  └─ [Selector] "AI根节点"
       ├─ [Sequence] "战斗逻辑" [Decorator: 黑板 - TargetActor IsSet]
       │    └─ [Selector] "选择攻击或追击" [Service: 更新目标信息]
       │         ├─ [BTTask_Attack] "攻击目标" 
       │         │    【注意：不再使用距离检查装饰器，攻击任务内部检查范围】
       │         └─ [BTTask_ChasePlayer] "追击玩家"
       │
       ├─ [Sequence] "返回出生点" [Decorator: 黑板 - TargetActor IsNotSet]
       │    └─ [BTTask_MoveTo] 目标: HomeLocation
       │
       └─ [Sequence] "巡逻"
            └─ [BTTask_Patrol] "巡逻"
```

**关键改变**：
1. **移除距离检查装饰器**：攻击任务内部会检查攻击范围，无需装饰器
2. **攻击任务直接连接Selector**：如果不在范围，攻击任务返回Failed，自动执行追击
3. **追击任务进入范围立即成功**：不再等待目标静止

---

## 系统架构

### NodeMemory 架构（已优化）

所有行为树任务节点使用 **实例内存（Instance Memory）** 存储状态，确保：

- ✅ **线程安全**：每个AI实例拥有独立的内存空间
- ✅ **多实例支持**：多个敌人可同时执行相同任务而不会相互干扰
- ✅ **性能优化**：使用平方距离计算，降低距离检查频率

### 任务节点内存结构

```cpp
// BTTask_ChasePlayer - FChaseTaskMemory
struct FChaseTaskMemory
{
    float RepathTimer;              // 重新寻路计时器
    FVector ChaseStartLocation;     // 追击起始位置
    FVector LastTargetLocation;     // 上次目标位置
    float NextDistanceCheckTime;    // 下次距离检查时间（性能优化）
    bool bHasEnteredAttackRange;    // 是否已进入攻击范围（防震荡）
};

// BTTask_Patrol - FPatrolTaskMemory
struct FPatrolTaskMemory
{
    float WaitTimer;        // 等待计时器
    bool bIsWaiting;        // 是否在等待中
    bool bIsMoving;         // 是否在移动中
};

// BTTask_Attack - FAttackTaskMemory
struct FAttackTaskMemory
{
    uint8 CurrentState;     // 当前攻击状态
    float StateTimer;       // 状态计时器
    bool bHasDealtDamage;   // 是否已经造成伤害
    TWeakObjectPtr<UAnimInstance> AnimInstance;  // 动画实例缓存
    float AnimStartTime;    // 动画开始时间
};
```

---

## 配置参数参考

### 黑板（Blackboard）必需Key

创建黑板时需要添加以下Key：

| Key Name | Key Type | 默认值 | 说明 |
|----------|----------|--------|------|
| TargetActor | Object (Actor) | None | 当前目标（玩家）|
| HomeLocation | Vector | 0,0,0 | 出生点位置（自动初始化）|
| PatrolCenter | Vector | 0,0,0 | 巡逻中心点（自动初始化为出生点）|
| PatrolRadius | Float | 500.0 | 巡逻半径 |
| TargetLocation | Vector | 0,0,0 | 目标位置（自动更新）|
| TargetDirection | Vector | 0,0,0 | 目标方向（自动更新）|
| ChaseStartLocation | Vector | 0,0,0 | 追击起始位置（发现目标时自动设置）|
| AIState | Enum 或 String | None | AI当前状态（EAIBehaviorState）|

> **关于 AIState 枚举**：如果 `EAIBehaviorState` 枚举在蓝图中无法找到，可以：
> 1. 使用 **String** 类型替代 Enum
> 2. 手动输入枚举名称 `EAIBehaviorState`
> 3. 查看【2026-03-08 重要修复更新】章节的详细解决方案

### 黑板Key配置详解

#### TargetActor（关键）
- **Type**: Object
- **Base Class**: 必须设置为 **Actor**（否则MoveTo任务会忽略）
- **用途**: 存储当前目标（通常是玩家角色）
- **设置方式**: 由 EnemyAIController 的感知系统自动设置

#### AIState
- **Type**: Enum（或 String 作为备选）
- **Enum Type**: EAIBehaviorState（如果可用）
- **默认值**: None
- **用途**: 当前AI状态，用于调试

---

## 测试前准备

### 1. 创建黑板（Blackboard）

1. 在内容浏览器中右键 → **Artificial Intelligence → Blackboard**
2. 命名为 `BB_Enemy`
3. 按照【黑板必需Key】表格添加所有Key

**重要提示**：如果 `EAIBehaviorState` 枚举无法在蓝图中找到：
- 将 `AIState` Key 类型设为 **String** 而非 Enum
- 或直接在 Enum Type 字段手动输入 `EAIBehaviorState`

### 2. 创建行为树（Behavior Tree）

1. 在内容浏览器中右键 → **Artificial Intelligence → Behavior Tree**
2. 命名为 `BT_Enemy`
3. 选择黑板为 `BB_Enemy`
4. 构建【推荐的简化行为树结构】（见上文）

**关键配置**：

| 节点 | 配置 |
|------|------|
| 更新目标信息服务 | TargetActorKey: TargetActor, TargetLocationKey: TargetLocation, Interval: 0.5 |
| 攻击任务 | TargetActorKey: TargetActor, AttackRange: 250, AttackRecoveryTime: 0.5 |
| 追击任务 | TargetActorKey: TargetActor, AttackRange: 250, MaxChaseDistance: 5000 |
| 巡逻任务 | PatrolCenterKey: PatrolCenter, PatrolRadiusKey: PatrolRadius |

### 3. 创建敌人数据资产

1. 在内容浏览器中右键 → **Miscellaneous → Data Asset**
2. 选择 `EnemyDataAsset` 作为父类
3. 命名为 `DA_TestEnemy`
4. 配置：
   - BehaviorTree: `BT_Enemy`
   - MinAttackInterval: 1.0
   - MaxAttackInterval: 2.0
   - PatrolRadius: 500.0

### 4. 创建敌人蓝图

1. 右键 → **Blueprint Class**，选择 `EnemyBase`
2. 命名为 `BP_TestEnemy`
3. 在 **Class Defaults** 中：
   - 勾选 **Show Debug Info**
   - 设置 **EnemyData** 为 `DA_TestEnemy`

### 5. 配置玩家角色

1. **Tags**: 添加 `"Player"`
2. **AIPerceptionStimuliSource**: 注册 `AISense_Sight`

---

## 功能测试

### 测试1：攻击动画不卡住

**目标**：验证攻击动画异常时能正确结束

**步骤**：
1. 放置敌人并进入战斗
2. 观察攻击流程

**预期结果**：
- 攻击动画播放，触发伤害
- 即使动画异常，5秒后强制结束
- 超时后返回 Succeeded，而非 Failed

**验证日志**：
```
[LogAIAttack] 播放攻击动画，时长: 1.2
[LogAIAttack] 伤害已触发，进入攻击判定阶段
[LogAIAttack] 攻击完成（动画:0 后摇:0.5），返回Succeeded
```

### 测试2：攻击-追击不震荡

**目标**：验证敌人不会反复在攻击和追击之间切换

**步骤**：
1. 让敌人追击玩家
2. 进入攻击范围后观察行为

**预期结果**：
- 进入攻击范围（250单位）后，追击任务立即返回成功
- 攻击任务接管，执行攻击
- 攻击完成后，如果目标仍在范围，继续攻击；如果移动远离，追击
- 日志中不应出现频繁的"进入攻击范围且目标静止"重复输出

**验证日志**：
```
[LogAIChase] 进入攻击范围(距离:248.5)，停止追击，返回成功
[LogAIAttack] 开始攻击目标: Player
[LogAIAttack] 攻击完成（动画:0 后摇:0.5），返回Succeeded
```

### 测试3：多实例并发

**目标**：验证多个敌人同时执行不会相互干扰

**步骤**：
1. 放置 5-10 个敌人
2. 控制玩家角色在敌人之间移动

**预期结果**：
- 每个敌人独立巡逻、追击、攻击
- 不会相互干扰（NodeMemory隔离）

---

## 常见问题排查

### 问题1：攻击动画卡住

**现象**：敌人进入攻击范围后一直播放动画，不结束

**解决方案**：
- 已修复：攻击任务添加动画播放时间检测
- 超时5秒后强制结束并返回 Succeeded
- 检查日志 `[LogAIAttack]` 查看具体状态

### 问题2：攻击-追击震荡

**现象**：敌人反复在攻击和追击之间切换，日志刷屏

**解决方案**：
- 已修复：移除"目标静止"检查
- 进入攻击范围立即停止移动并返回成功
- 移除行为树中的距离检查装饰器（攻击任务内部已检查）

### 问题3：黑板中找不到 EAIBehaviorState 枚举

**现象**：创建 AIState Key 时，Enum Type 下拉列表为空

**解决方案**：
1. 将 AIState Key 类型改为 **String**（推荐）
2. 或手动输入枚举名称：`EAIBehaviorState`
3. 查看【2026-03-08 重要修复更新】章节的详细方案

---

## 版本历史

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| 1.0 | 2026-03-07 | 初始版本 |
| 2.0 | 2026-03-08 | 添加NodeMemory架构、性能优化 |
| 2.1 | 2026-03-08 | 修复攻击动画卡住、攻击-追击震荡、日志编译错误 |

---

## 相关文档

- [AGENTS.md](AGENTS.md) - 项目开发状态文档
- [TestGuide_EnemySystem.md](TestGuide_EnemySystem.md) - 敌人系统测试指南
