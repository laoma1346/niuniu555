# 财神末法冒险 (niuniu555) - 开发状态文档

## 项目概述

- **项目名称**: 财神末法冒险
- **类型**: 3D ARPG Roguelike
- **引擎**: Unreal Engine 5.6
- **项目路径**: `C:\Users\Win10\Desktop\niuniu555`

---

## 开发路线图（10个阶段）

### 📋 阶段总览

| 阶段 | 名称 | 状态 | 核心内容 |
|------|------|------|----------|
| Phase 1 | 基础架构与数据层 | ✅ 完成 | 存档、输入、状态机、对象池、属性系统 |
| Phase 2 | 角色基础与移动 | ⚠️ 基本完成 | 移动✅、闪避✅、钩索✅、**角色状态机** ⏳ |
| Phase 3 | 战斗核心系统 | ⚠️ 基本完成 | 武器✅、连招✅、伤害✅、**准星/锁定** ⏳、**派生攻击** ⏳ |
| Phase 4 | 敌人与 AI | ⚠️ 基本完成 | 敌人基类✅、行为树✅、生成器⏳、掉落系统⏳ |

### Phase 4 开发状态细分
- **4.1 敌人基础架构**: ✅ 完成 (EnemyBase + EnemyStateMachineComponent + EnemyDataAsset)
- **4.2 AI行为树系统**: ✅ 完成 (EnemyAIController + BTTasks + BTDecorators + BTServices)
- **4.3 敌人生成器**: ⏳ 待开发
- **4.4 敌人死亡与掉落系统**: ⏳ 待开发
| Phase 5 | 装备与背包系统 | ⏳ 待开发 | 装备数据、随机词条、套装系统、背包 |
| Phase 6 | 关卡与流程 | ⏳ 待开发 | 房间/节点系统、地图生成、节点交互 |
| Phase 7 | 经济与成长系统 | ⏳ 待开发 | 金币、神格碎片、技能树、局内成长 |
| Phase 8 | UI 与体验 | ⏳ 待开发 | 漫画风格UI、HUD、引导系统 |
| Phase 9 | 美术表现与特效 | ⏳ 待开发 | VFX、角色外观、场景表现 |
| Phase 10 | 平衡与配置 | ⏳ 待开发 | 数值配置、调试工具、平衡报表 |

---

## Phase 1 - 基础架构与数据层 (100% 完成)

| 系统 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **SaveSystem** | ✅ 完成 | `SaveSystem/` 文件夹 | 本地存档加密/解密、跨局持久化数据、多存档槽位管理、云同步接口预留 |
| **InputSystem** | ✅ 完成 | `InputSystem/` 文件夹 | 输入设备抽象（键鼠/手柄热切换）、战斗输入缓冲、连招输入序列缓存、UI与Gameplay输入优先级分离 |
| **GameStateMachine** | ✅ 完成 | `GameFlow/` 文件夹 | 游戏循环状态机（主菜单→营地→关卡→战斗→暂停→结算→返回）、异步场景加载、内存管理 |
| **ObjectPool** | ✅ 完成 | `ObjectPool/` 文件夹 | 全局对象池管理器（特效/伤害数字/掉落物） |
| **AttributeSystem** | ✅ 完成 | `AttributeSystem/` 文件夹 | 基础属性结构体（生命/攻击/防御/移速/暴击/爆伤/金币获取率）、属性修饰器接口、属性变化事件分发 |

---

## Phase 2 - 角色基础与移动 (100% 完成)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **AdvancedMovementComponent** | ✅ 完成 | `CharacterSystem/AdvancedMovementComponent.h/cpp` | 3D角色移动逻辑（奔跑/急停/转向插值）、移动状态机（Idle/Walking/Running/Sprinting/Stopping） |
| **CameraControllerComponent** | ✅ 完成 | `CharacterSystem/CameraControllerComponent.h/cpp` | 摄像机模式切换（自由/锁定/固定）、锁定目标管理 |
| **CharacterStateMachine** | ⏳ 待开发 | - | 完整角色状态机（Idle/Move/Attack/Dodge/Hit/Stun/Dead），协调各组件状态切换 |
| **DodgeComponent** | ✅ 完成 | `CharacterSystem/DodgeComponent.h/cpp` | 闪避动作资源管理（动画/位移曲线）、无敌帧时间窗口配置、闪避体力消耗与恢复、闪避取消攻击后摇（Cancel Window） |
| **HookshotComponent** | ✅ 完成（已简化） | `CharacterSystem/HookshotComponent.h/cpp` | 钩索瞄准射线检测、钩索CD与能量消耗、**仅支持将敌人拉回玩家**（移除玩家飞向目标功能） |

**HookshotComponent 简化说明**:
- ✅ 鼠标瞄准（屏幕位置转世界方向）
- ✅ 专属碰撞通道检测（可配置 `HookshotTraceChannel`）
- ✅ 标签验证（`bUseTagValidation` + `PullableActorTag`）
- ✅ **仅支持**: 目标拉向玩家（敌人拉回）
- ✅ 完整的错误处理（无效目标检测）

**Blueprint 可配置属性**:
- `HookshotTraceChannel`: 钩索检测碰撞通道（默认 GameTraceChannel1）
- `PullableActorTag`: 可拉取目标标签（默认 "Enemy"）
- `bUseTagValidation`: 是否启用标签验证
- `MaxRange`: 最大射程
- `FireSpeed`/`PullSpeed`: 发射/拉回速度

---

## Phase 3 - 战斗核心系统 (🚧 进行中)

**目标**: 构建完整的战斗核心玩法

### 3.1 武器系统架构 (✅ 已完成 - 2026-03-06)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **WeaponTypes** | ✅ 完成 | `WeaponSystem/WeaponTypes.h` | 武器类型枚举、伤害结构体、连招信息结构体 |
| **WeaponInterface** | ✅ 完成 | `WeaponSystem/WeaponInterface.h/cpp` | IWeaponInterface 接口定义（BlueprintNativeEvent）|
| **WeaponBase** | ✅ 完成 | `WeaponSystem/WeaponBase.h/cpp` | AWeaponBase 武器基类，实现轻击/重击/蓄力攻击逻辑 |
| **WeaponDataAsset** | ✅ 完成 | `WeaponSystem/WeaponDataAsset.h/cpp` | UWeaponDataAsset 数据资产，支持蓝图配置武器属性 |
| **WeaponManagerComponent** | ✅ 完成 | `WeaponSystem/WeaponManagerComponent.h/cpp` | UWeaponManagerComponent 武器管理器，管理武器注册/切换/缓存 |

**技术要点**: 
- 数据与逻辑分离架构：属性通过 `UWeaponDataAsset` 在蓝图中配置
- 所有 `BlueprintNativeEvent` 接口函数必须使用 `IWeaponInterface::Execute_` 前缀调用
- 武器实例缓存机制：切换武器时复用已有实例，避免重复创建

**武器类型设计**:

| 武器类型 | 设计理念 | 核心机制 |
|----------|----------|----------|
| **轩辕剑** | 近战武器 | 连击系统、格挡反击 |
| **盘古斧** | 近战/远程混合 | 近战蓄力、远程投掷、召回机制 |
| **财神金杖** | 远程法术 | 弹道法术、范围效果 |
| **射日弓** | 远程物理 | 蓄力射击、贯穿、散射 |

**技术要点**:
- 武器数据-逻辑分离（ScriptableObject配置 / Runtime武器实例）
- 武器切换逻辑（动画过渡、当前武器状态保存）

### 3.2 连招系统 (✅ 已完成 - 2026-03-06)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **ComboTypes** | ✅ 完成 | `WeaponSystem/ComboTypes.h` | 连招数据类型（EWeaponComboInputType, FWeaponComboSequence, FWeaponComboStageConfig） |
| **ComboDetectorComponent** | ✅ 完成 | `WeaponSystem/ComboDetectorComponent.h/cpp` | UComboDetectorComponent 输入序列检测、连招状态机 |
| **WeaponComboIntegration** | ✅ 完成 | `WeaponSystem/WeaponComboIntegration.h/cpp` | UWeaponComboIntegration 武器与连招绑定、自动触发武器攻击 |
| **ComboInputBuffer** | ✅ 完成 | `WeaponSystem/ComboInputBuffer.h/cpp` | UComboInputBuffer 全局输入缓冲子系统、容错输入处理 |
| **ComboWindowController** | ✅ 完成 | `WeaponSystem/ComboWindowController.h/cpp` | UComboWindowController 连招取消窗口管理、阶段过渡控制 |

**核心机制**:
- ✅ 连招序列配置表（轻击-L / 重击-H的排列组合，如LLH/LHH）
- ✅ 连招阶段窗口（输入容忍时间 / 取消窗口）
- ✅ 输入缓冲系统（提前输入缓存，提升操作手感）
- ✅ 连招保护（被击中时连招中断）- 受击时自动中断连招
- 派生攻击触发器（特定连招最后一击触发特殊效果）- ⏳ 待扩展（如：LLH最后一击触发爆炸效果）
- 武器切换取消（攻击中途切换武器触发独特转场技）- ⏳ 待扩展（高级功能）

### 3.3 伤害计算与受击反馈 (✅ 已完成 - 2026-03-06)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **DamageTypes** | ✅ 完成 | `CombatSystem/DamageTypes.h` | 伤害计算参数（FDamageCalculationParams）、详细结果（FDamageCalculationResult） |
| **DamageCalculator** | ✅ 完成 | `CombatSystem/DamageCalculator.h/cpp` | UDamageCalculator 通用伤害计算工具类，支持防御/抗性/暴击/浮动 |
| **DamageableInterface** | ✅ 完成 | `CombatSystem/DamageableInterface.h` | IDamageableInterface 可受伤对象接口、FHitInfo受击信息 |
| **HitReactionComponent** | ✅ 完成 | `CombatSystem/HitReactionComponent.h/cpp` | UHitReactionComponent 受击反馈（材质闪白、顿帧、击退、受击动画） |

**伤害公式:**
```
基础伤害 = 攻击力 × 技能倍率
防御减免 = Max(基础伤害 × 0.1, 基础伤害 - 防御力)  // 最低10%保底
抗性减免 = 防御减免 × (1 - 抗性)  // 抗性上限90%
暴击伤害 = 减免后伤害 × 暴击倍率  // 暴击时
最终伤害 = 暴击伤害 × 随机浮动(0.9-1.1) + 固定加成
```

**受击反馈类型:**
- **材质闪白**: 受击时角色模型闪红/闪白
- **顿帧(Hit Stop)**: 暴击或重击时游戏时间变为5%（子弹时间效果），0.05秒后恢复
- **击退**: 根据攻击方向使用 LaunchCharacter 推开目标
- **受击动画**: 轻击/重击/击倒不同动画反应

**已知限制:**
- **击退视觉表现**: 测试用的 BP_TestDummy 无 AI Controller，`LaunchCharacter` 无法产生明显位移动画（UE5引擎限制）。实际游戏中敌人有 AI Controller 时此问题自动解决。如需立即可见的击退效果，可将 Dummy 的 Capsule 开启物理模拟。
- **HitInfo 结构体**: `Damage` 字段当前为 `BlueprintReadOnly`，如需在蓝图中动态修改伤害值（如 Buff 系统），需改为 `BlueprintReadWrite`（不影响当前功能）

### 3.3 伤害计算与战斗公式

**伤害公式**:
```
最终伤害 = (攻击力 × 技能倍率 - 敌人防御) × 暴击倍率 × 随机浮动(0.9-1.1)
```

**伤害类型**:
- 物理 / 火焰 / 雷电 / 神圣
- 抗性计算

**技术组件**:
- Hitbox管理系统（攻击框生成 / 持续帧 / 已命中列表去重）
- 受击方反馈（材质闪白 / 顿帧 / 击退方向计算）

### 3.4 准星与锁定系统 ⏳ 待开发

**状态**: 尚未实现（可选补充功能，不影响基础战斗）

**功能要点**:
- **软锁定逻辑**: 摄像机自动向最近敌人吸附
- **硬锁定切换**: 手动切换目标 / 锁定图标UI
- **准星扩散**: 移动/射击时准星UI动态变化
- **远程武器抛物线预览**: 射日弓/金杖的弹道辅助线

**开发建议**: 可在 Phase 4 敌人系统完成后开发，因为有敌人才能测试锁定功能

---

## Phase 4 - 敌人与 AI

**目标**: 构建敌人与战斗环境

### 4.1 敌人基础架构 (✅ 已完成 - 2026-03-07)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **EnemyTypes** | ✅ 完成 | `EnemySystem/EnemyTypes.h` | 敌人类型/状态/阵营枚举，AI感知配置结构 |
| **EnemyDataAsset** | ✅ 完成 | `EnemySystem/EnemyDataAsset.h/cpp` | 敌人数据资产，支持蓝图配置属性/AI/特效/音效 |
| **DropTableDataAsset** | ✅ 完成 | `EnemySystem/DropTableDataAsset.h/cpp` | 掉落表数据资产，支持权重随机和保底机制 |
| **EnemyStateMachineComponent** | ✅ 完成 | `EnemySystem/EnemyStateMachineComponent.h/cpp` | 敌人状态机组件，管理Spawn/Idle/Patrol/Chase/Attack/Hit/Stun/Return/Dead状态 |
| **EnemyBase** | ✅ 完成 | `EnemySystem/EnemyBase.h/cpp` | 敌人基类，整合属性系统+状态机+受击反馈，实现IDamageableInterface |

**功能验证状态** (2026-03-07):
- ✅ 敌人数据资产（Data Asset）正确创建和配置
- ✅ 敌人蓝图（BP_TestEnemy）从EnemyBase继承
- ✅ 血量初始化（HP:100/100正确显示，非0/0）
- ✅ 攻击扣血（血量减少，日志输出正常）
- ✅ 状态机切换（Idle → Hit → Idle自动恢复）
- ✅ 受击反馈（闪白/击退/音效）
- ✅ 死亡流程（血量归零 → Dead状态 → 尸体消失）

**Debug Log**: 详见「开发规范与常见陷阱」章节，记录了函数名不匹配、蓝图覆盖、血量初始化等关键问题及解决方案。

### 4.2 AI行为树系统 (🚧 进行中)

| 子模块 | 状态 | 文件 | 功能 |
|--------|------|------|------|
| **EnemyAIController** | ✅ 完成 (4.2.1) | `AI/EnemyAIController.h/cpp` | AI控制器基类，运行行为树，管理感知和目标，黑板Key管理 |
| AIPerceptionComponent | ✅ 完成 (4.2.2) | `AI/EnemyAIController.h/cpp` | 视觉感知配置（半径3000，角度200°） |
| **BTTask_Patrol** | ✅ 完成 (4.2.3) | `AI/BehaviorTree/Tasks/BTTask_Patrol.h/cpp` | 巡逻任务节点：随机导航点移动+到达等待 |
| **BTTask_ChasePlayer** | ✅ 完成 (4.2.4) | `AI/BehaviorTree/Tasks/BTTask_ChasePlayer.h/cpp` | 追击任务节点：持续跟随目标，支持最大追击距离 |
| **BTTask_Attack** | ✅ 完成 (4.2.5) | `AI/BehaviorTree/Tasks/BTTask_Attack.h/cpp` | 攻击任务节点：前摇-攻击-后摇状态机 |
| **BTDecorator_DistanceCheck** | ✅ 完成 (4.2.6) | `AI/BehaviorTree/Decorators/BTDecorator_DistanceCheck.h/cpp` | 距离检查装饰器：4种比较模式 |
| **BTService_UpdateTarget** | ✅ 完成 (4.2.7) | `AI/BehaviorTree/Services/BTService_UpdateTarget.h/cpp` | 目标更新服务：定期更新目标位置/距离/方向 |

**推荐的行为树结构**:
```
[Root]
  └─ [Selector] "AI根节点"
       ├─ [Sequence] "战斗逻辑" [Decorator: TargetActor IsSet]
       │    └─ [Selector] "选择攻击或追击" [Service: UpdateTarget]
       │         ├─ [Sequence] "攻击" [Decorator: Distance < 250]
       │         │    └─ [BTTask_Attack]
       │         └─ [BTTask_ChasePlayer]
       │
       ├─ [Sequence] "返回逻辑" [Decorator: TargetActor IsNotSet]
       │    └─ [BTTask_MoveTo] HomeLocation
       │
       └─ [Sequence] "巡逻逻辑"
            └─ [BTTask_Patrol]
```

**黑板必需Key**:
| Key | 类型 | 用途 |
|-----|------|------|
| TargetActor | Object(Actor) | 当前目标（玩家） |
| HomeLocation | Vector | 出生点位置 |
| PatrolCenter | Vector | 巡逻中心点 |
| PatrolRadius | Float | 巡逻半径 |
| TargetLocation | Vector | 目标位置（自动更新） |
| TargetDirection | Vector | 目标方向（自动更新） |

**测试文档**: `TestGuide_AI_BehaviorTree.md`

**4.2.1 完成功能**:
- ✅ 继承AAIController的EnemyAIController基类
- ✅ 自动从EnemyDataAsset加载BehaviorTree和Blackboard
- ✅ 配置AIPerception（视觉感知：半径3000，角度200°）
- ✅ 黑板Key管理（TargetActor, HomeLocation, PatrolRadius等）
- ✅ 目标自动发现（通过"Player"标签识别）
- ✅ 与状态机联动（发现目标→StartChase，丢失目标→StartReturn）
- ✅ 完整测试指南（含黑板/行为树创建步骤）

**4.2.4 完成功能**:
- ✅ 追击任务节点 BTTask_ChasePlayer
- ✅ 从黑板获取TargetActor并持续追击
- ✅ 支持最大追击距离限制（超出返回失败）
- ✅ 进入攻击范围且**目标静止**时返回成功（防止震荡）
- ✅ 定期重新寻路（RepathInterval，默认0.5秒）
- ✅ 追击起始位置记录（用于计算追击距离）
- ✅ 目标丢失检测（返回失败）
- ✅ **目标移动检测**：使用 LastTargetLocation 检测目标是否静止

**技术要点**:
- 使用 `AIController->MoveToActor` 持续跟随移动目标
- 移动状态检测：`AIController->GetMoveStatus() == EPathFollowingStatus::Type::Idle`
- 距离检查：目标在AttackRange内→成功，超出MaxChaseDistance→失败

**4.2.5 完成功能**:
- ✅ 攻击任务节点 BTTask_Attack
- ✅ 攻击状态机：前摇(WindingUp) → 攻击判定(Attacking) → 后摇(Recovering)
- ✅ 攻击前可配置面向目标旋转（RotationSpeed）
- ✅ 支持攻击动画蒙太奇播放，任务等待动画完成才结束
- ✅ 伤害判定使用IDamageableInterface接口
- ✅ 攻击范围检查（距离+角度锥形检查）
- ✅ 目标不在范围内时返回失败
- ✅ 攻击动画与任务同步（后摇阶段等待动画结束+最小后摇时间）
- ✅ **攻击任务等待冷却结束**：Recovering阶段同时检查动画+后摇+冷却
- ✅ **攻击完成返回Succeeded**：让行为树正确执行后续逻辑
- ✅ **防止行为树震荡**：冷却期间不返回，防止目标被清除

**可配置参数**:
| 参数 | 默认值 | 说明 |
|------|--------|------|
| AttackWindUpTime | 0.3s | 攻击前摇时间（可面向目标，bUseAnimationProgress=false时有效） |
| AttackRecoveryTime | 0.5s | 最小后摇时间（即使动画结束也要等待此时间） |
| AttackDamage | 20.0f | 攻击伤害值 |
| AttackRange | 200.0f | 攻击范围 |
| AttackAngle | 60.0f | 攻击角度（前方锥形） |
| RotationSpeed | 5.0f | 面向目标旋转速度 |
| bUseAnimationProgress | true | 使用动画进度触发伤害 |
| AttackDamageTriggerPoint | 0.3 | 动画播放到30%时触发伤害（0.0-1.0） |

**EnemyDataAsset 配置**:
| 参数 | 推荐值 | 说明 |
|------|--------|------|
| MinAttackInterval | 1.0f | 最小攻击间隔（秒） |
| MaxAttackInterval | 2.0f | 最大攻击间隔（秒，实际冷却在这之间随机） |

**攻击流程**:
1. 检查攻击冷却（CanAttack）→ 冷却中则进入 Recovering 阶段等待
2. 开始攻击 → 播放攻击动画蒙太奇，启动攻击冷却
3. 前摇阶段 → 面向目标旋转
4. 触发伤害 → 根据动画进度或时间触发
5. 后摇阶段(Recovering) → **同时等待**：
   - 动画播放完成
   - 最小后摇时间结束
   - **攻击冷却结束**（关键！防止行为树切换）
6. 全部条件满足 → 返回Succeeded

**追击任务流程**:
1. 获取目标Actor → 开始追击
2. Tick更新 → 持续移动向目标
3. 检测目标是否静止（移动距离<10cm）
4. 静止且进入攻击范围 → 停止移动，返回Succeeded
5. 目标仍在移动 → 继续追击，保持InProgress

**4.2.6 完成功能**:
- ✅ 距离检查装饰器 BTDecorator_DistanceCheck
- ✅ 4种比较模式：小于/大于/范围内/范围外
- ✅ 支持2D/3D距离切换
- ✅ 可配置的Blackboard键绑定
- ✅ 详细的静态描述（编辑器中显示检查条件）

**使用示例**:
```
[Sequence]
  [Decorator: 距离检查 < 攻击距离]
    [BTTask_Attack]
  [Decorator: 距离检查 > 最大追击距离]
    [BTTask_ReturnHome]
```

**4.2.7 完成功能**:
- ✅ 目标更新服务 BTService_UpdateTarget
- ✅ 定期更新目标位置到黑板
- ✅ 可选更新目标方向向量
- ✅ 可选更新与目标的距离
- ✅ 支持2D/3D距离计算

**技术要点**:
- 服务默认间隔：0.5秒（Interval）
- 随机偏差：0.1秒（RandomDeviation）避免多个AI同时更新

**4.2.3 完成功能**:
- ✅ 巡逻任务节点 BTTask_Patrol
- ✅ 从黑板获取巡逻中心和半径（支持动态配置）
- ✅ 在巡逻半径内生成随机目标点（10次尝试找到导航网格有效点）
- ✅ 使用AIController->MoveToLocation进行路径移动
- ✅ 到达目标点后等待指定时间（WaitTimeAtPoint可配置）
- ✅ TickTask监控移动状态，自动进入等待阶段
- ✅ OnTaskFinished清理（停止移动）
- ✅ 详细日志输出便于调试

**技术要点**:
- 依赖模块：`NavigationSystem`（需在Build.cs中添加）
- 随机点生成：`FMath::RandPointInCircle` + `NavSystem->ProjectPointToNavigation`
- 移动状态检测：`AIController->GetMoveStatus() == EPathFollowingStatus::Type::Idle`
- Blackboard键访问：`PatrolCenterKey.SelectedKeyName`（注意不是GetSelectedKeyID()）

**Bug修复记录** (2026-03-07):
1. **感知组件初始化失败**: 原代码依赖父类GetAIPerceptionComponent()返回nullptr，改为显式创建AIPerceptionComponent并SetPerceptionComponent注册
2. **HomeLocation初始化失败**: 添加SpawnLocation为零的容错处理，使用当前位置作为备选
3. **黑板Key基类陷阱**: 文档说明Object类型Key必须设置BaseClass为Actor，否则MoveTo任务会忽略
4. **NavigationSystem链接错误**: 添加NavigationSystem模块到Build.cs的PublicDependencyModuleNames
5. **攻击只执行一次**: 攻击任务返回Succeeded后Selector不再重新评估，导致无法连续攻击。修复：攻击任务完成后返回Failed（而非Succeeded），强制Selector重新评估攻击条件，实现循环攻击

**Bug修复记录** (2026-03-08):
6. **攻击任务与追击任务震荡**: 
   - 攻击任务返回 Failed 导致 Selector 频繁重新评估
   - 修复：攻击完成后返回 Succeeded，让行为树正常执行
7. **攻击冷却期间行为树震荡**: 
   - 攻击冷却期间返回 Failed 导致 Selector 反复切换任务
   - 修复：攻击任务在 Recovering 阶段等待动画+后摇+冷却都结束才返回
8. **追击任务目标移动时返回成功**: 
   - 目标移动时追击任务立即返回成功，导致攻击任务频繁进入退出
   - 修复：添加目标移动检测，目标静止时才返回成功
9. **AttackRange默认值不一致**: 
   - BTTask_ChasePlayer 和 BTTask_Attack 默认值分别为 200 和 200
   - 修复：统一为 250
10. **攻击完成后目标被清除**:
    - 攻击完成后行为树立即切换导致目标清除
    - 修复：攻击任务等待冷却结束才返回，防止行为树切换

**代码重构记录** (2026-03-07 Phase 2-3):

| 问题 | 解决方案 | 影响文件 |
|------|----------|----------|
| **硬编码EnemyAIController依赖** | 移除`#include "AI/EnemyAIController.h"`，使用通用`AAIController` + 常见目标键名数组 | `EnemyStateMachineComponent.cpp` |
| **LogTemp不规范** | 定义专用日志类别：`LogAIAttack`, `LogAIPatrol`, `LogAIChase` | `BTTask_Attack.cpp`, `BTTask_Patrol.cpp`, `BTTask_ChasePlayer.cpp` |
| **距离计算不一致** | 统一使用`FVector::Dist2D`（水平距离），添加`bUse3DDistance`配置项 | `BTDecorator_DistanceCheck.cpp`, `BTTask_ChasePlayer.cpp`, `BTTask_Attack.cpp` |
| **动画回调未绑定** | 在`StartAttack`中使用`Montage_SetEndDelegate`绑定`OnAttackMontageEnded` | `BTTask_Attack.cpp` |
| **BlackboardKeySelector未初始化** | 在构造函数中调用`AddObjectFilter`/`AddVectorFilter` | 所有BT任务类 |
| **攻击任务与动画不同步** | 修改后摇阶段逻辑，等待动画播放完成+满足最小后摇时间后才返回Failed（强制Selector重新评估） | `BTTask_Attack.cpp` |
| **攻击与追击任务震荡** | 攻击任务返回 Succeeded（不是 Failed） | `BTTask_Attack.cpp` |
| **攻击任务等待冷却** | Recovering 阶段同时检查动画+后摇+冷却，全部结束才返回 | `BTTask_Attack.cpp` |
| **追击任务目标移动检测** | 添加 LastTargetLocation 成员变量，目标静止时才返回成功 | `BTTask_ChasePlayer.h/cpp` |
| **AttackRange默认值统一** | BTTask_ChasePlayer 和 BTTask_Attack 的 AttackRange 统一为 250 | `BTTask_ChasePlayer.h`, `BTTask_Attack.h` |

**日志类别使用规范**:
```cpp
// AI行为树日志
DEFINE_LOG_CATEGORY(LogAIAttack);   // BTTask_Attack.cpp
DEFINE_LOG_CATEGORY(LogAIPatrol);   // BTTask_Patrol.cpp
DEFINE_LOG_CATEGORY(LogAIChase);    // BTTask_ChasePlayer.cpp

// 使用示例
UE_LOG(LogAIAttack, Log, TEXT("开始攻击: %s"), *TargetName);
UE_LOG(LogAIChase, Warning, TEXT("目标丢失"));
```

**关键修复代码**:
```cpp
// 显式创建感知组件
AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
// ... 配置 ...
SetPerceptionComponent(*AIPerceptionComponent);  // 关键：注册到AI系统
AIPerceptionComponent->Activate();  // 关键：激活组件
```

**依赖关系**:
```
EnemyAIController (4.2.1) ✅
    ↓ 依赖
EnemyBase + EnemyStateMachineComponent (4.1) ✅
    ↓ 依赖
BehaviorTree + Blackboard (UE5内置)
```

### 4.3 敌人生成器 (🚧 基础功能已完成 - 2026-03-08)

**已完成**：
- ✅ **EnemySpawnPoint** - 生成点组件
  - 配置敌人类型、生成半径、高度偏移
  - 冷却时间管理、最大存活数限制
  - **AutoSpawn 独立生成模式**（无需Spawner）
- ✅ **EnemySpawner** - 生成器管理器
  - 触发区域（BoxComponent）
  - 多波次配置（FSpawnWaveInfo）
  - 波次切换、最大存活数控制
  - 自动生成点收集（20米范围）

**双模式支持**：
| 模式 | 说明 |
|------|------|
| AutoSpawn | 勾选后独立工作，持续自动生成敌人 |
| 波次模式 | 通过Spawner管理，玩家触发/分波次生成 |

**待开发**：
- ⏳ 波次配置数据资产（独立DataAsset）
- ⏳ 生成特效（粒子系统/黑雾动画）
- ⏳ 动态难度调整
- ⏳ 稀有敌人刷出逻辑

**测试文档**：`TestGuide_EnemySpawner.md`（三种测试模式：快速/标准/压力）

### 4.4 敌人死亡与掉落系统

- 死亡动画与布娃娃物理切换
- 金币掉落（瞬时掉落/自动吸附范围）
- 装备掉落判定（真随机算法 + 保底计数器）
- 神格碎片掉落（BOSS必掉/精英概率掉/普通低概率）

---

## Phase 5 - 装备与背包系统

**目标**: 角色成长核心

### 5.1 装备数据架构

**装备静态数据**:
- ID / 名称 / 描述 / 模型 / 基础属性 / 词条池 / 套装ID
- 装备品质枚举：白/蓝/金/暗金/神话

**装备运行时数据**:
- 当前耐久 / 当前词条实例 / 强化等级

### 5.2 随机词条与套装系统

- 词条库设计（前缀库/后缀库/数值范围曲线）
- 随机生成算法（权重抽取/冲突词条互斥规则）
- 套装效果管理器（2件/4件/6件效果触发检测）
- 装备比较系统（悬浮提示显示与当前装备属性差值）

### 5.3 装备槽位与背包系统

**槽位管理**:
- 主手 / 副手 / 头盔 / 护甲 / 护腿 / 饰品1 / 饰品2
- 背包网格系统（固定格子数/分类筛选/整理排序）
- 装备穿戴/卸下逻辑（属性重新计算/外观更换）
- 快捷装备栏（战斗中的轮盘切换）

### 5.4 装备耐久与经济互动

- 耐久度消耗（受击/使用次数）
- 破损状态（属性下降/无法装备）
- 分解系统（转换为强化材料/金币）
- 修理系统（金币消耗公式：耐久损失比例 × 装备价值）

---

## Phase 6 - 关卡与流程

**目标**: 游戏关卡逻辑

### 6.1 房间/节点基类系统

**房间类型枚举**:
- Combat（战斗） / Event（事件） / Shop（商店） / Skill（技能） / Boss（BOSS） / Start（起点）

**房间配置数据**:
- 场景预制体 / 背景音乐 / 环境特效

**房间状态管理**:
- 未进入 / 进行中 / 已完成 / 已跳过

### 6.2 地图生成器

- 章节配置（节点层数/分支密度/必经BOSS节点）
- 随机路线生成算法（确保起点到终点有至少3条路径）
- 节点连接可视化（中式卷轴地图上的路径连线）
- 已通关节点标记与不可回退机制

### 6.3 节点交互逻辑

- 节点选择界面（摄像机移动到节点位置/显示预览信息）
- 路线选择锁定（选择后其他路径变灰）
- 跨房间数据保持（当前血量/金币/装备/神格碎片）

### 6.4 具体房间类型实现

**战斗房**: 普通/精英/BOSS三种难度配置
**事件房**: 随机事件库（增益神谕/诅咒装备/金币祭坛/神秘商人）
**商店房**: 商品生成逻辑（基于当前章节的商品池）
**技能房**: 神格碎片兑换/技能重置服务

---

## Phase 7 - 经济与成长系统

**目标**: 角色成长与资源管理

### 7.1 金币经济系统

**金币获取**:
- 击杀掉落 / 宝箱 / 事件 / 出售装备

**金币消耗**:
- 商店购买 / 修理 / 复活 / 重置技能

**经济曲线控制**:
- 章节金币产出预期与装备价格通胀

### 7.2 神格碎片系统

- 碎片类型：通用碎片 / 三流派专属碎片
- 碎片UI：背包内展示 / 获得时的特殊特效
- 碎片使用入口：仅在技能房或营地可使用

### 7.3 技能树系统

**树形结构数据**:
- 节点ID / 前置条件 / 消耗碎片数 / 效果类型

**三流派设计**:
- **武财神（攻击流）**: 暴击强化 / 吸血 / 武器精通 / 狂暴状态
- **文财神（防御流）**: 护盾 / 反伤 / 减CD / 复活 / 金币护甲转化
- **辅助流**: 钩索强化 / 移速 / 掉落率 / 商店折扣

**节点激活逻辑**:
- 依赖检查 / 资源扣除 / 效果应用到属性系统
- 重置功能（成本递增曲线）

### 7.4 局内成长系统

- 临时Buff系统（三选一祝福界面 / 持续至本局结束）
- 装备即时更换（战斗中打开背包暂停游戏 / 更换装备）

---

## Phase 8 - UI 与体验

**目标**: 游戏界面与引导

### 8.1 漫画风格UI框架

- 动态漫画框（对话框弹出速度/文字打印效果/立绘表情切换）
- 水墨/金箔材质Shader（UI边框动态流动效果）
- 屏幕震动与定格（暴击时画面停顿0.2秒）

### 8.2 核心游戏界面

**HUD**:
- 血条 / 神力（魔法）条 / 金币数 / 神格碎片数 / 迷你地图
- 连招提示UI（当前连招阶段高亮 / 下一击提示）
- 伤害数字（浮动文字 / 暴击大字体 / 金币掉落"+XX"）

### 8.3 功能界面

- **背包与装备界面**: 左侧人物模型 / 右侧背包网格 / 装备槽位高亮
- **技能树界面**: 神话树状图 / 连线动画 / 解锁预览
- **地图界面**: 中式卷轴展开 / 节点详情弹窗
- **商店界面**: 商品列表 / 购买确认 / 刷新商品功能

### 8.4 玩家引导系统

- 引导触发器（首次进入某类房间 / 首次获得某物品 / 首次死亡）
- 引导步骤配置（高亮UI / 强制点击 / 文字提示 / 时间限制）
- 新手章节（线性教学关卡，后续解锁自由选择）

---

## Phase 9 - 美术表现与特效

**目标**: 视觉表现

### 9.1 视觉特效系统

- 武器轨迹（刀光剑影的Mesh生成 / 颜色区分武器）
- 受击特效（血液 / 火花 / 金币爆开粒子）
- 环境特效（神格碎片发光 / 装备品质光柱 / BOSS领域效果）

### 9.2 角色与场景表现

- 财神外观系统（破布衫→华丽神装的形态变化，与装备挂钩）
- 场景破坏（可击碎的石柱 / 箱子 / 动态阴影）
- 漫画风格渲染（Toon Shader / 描边 / 多层级光照）

---

## Phase 10 - 平衡与配置

**目标**: 数值平衡与调试

### 10.1 数值配置工具

- 敌人数值表（血量/攻击/防御随章节成长曲线）
- 装备数值表（白装到神话装的属性区间）
- 经济数值表（章节金币产出/装备价格/修理成本）
- 概率配置表（掉落率/暴击率/事件触发权重）

### 10.2 调试与平衡工具

**作弊指令**:
- 无敌 / 秒杀 / 给金币 / 跳关

**实时数据监控**:
- DPS统计 / 生存时间 / 金币获取速率

**平衡性报表**:
- 自动生成各流派通关率 / 平均伤害

---

## 关键依赖关系

```
Phase 4(敌人) ────────┐
                      ▼
Phase 3(战斗系统) ──► Phase 5(装备) ──► Phase 7(经济)
       │                   │
       ▼                   ▼
Phase 6(关卡) ──────► Phase 8-10(UI/美术/平衡)
       ▲
       │
Phase 2(场景管理器)
```

**依赖提醒**:
- Phase 4（敌人）依赖 Phase 3（战斗系统）的基础伤害接口
- Phase 5（装备）依赖 Phase 1（属性系统）的修饰器接口
- Phase 6（关卡）依赖 Phase 2（场景管理器）的加载机制
- Phase 7（经济）依赖 Phase 5（装备）的分解/修理逻辑

---

## 核心系统文件位置

```
Source/niuniu555/
├── SaveSystem/
│   ├── NiuniuSaveGame.h/cpp
│   ├── SaveGameSubsystem.h/cpp
│   ├── SaveEncryptionSubsystem.h/cpp
│   └── SaveEncryptionUtil.h/cpp
├── InputSystem/
│   ├── InputBufferSystem.h/cpp
│   ├── InputDeviceManager.h/cpp
│   ├── InputPriorityManager.h/cpp
│   └── ComboInputSystem.h/cpp
├── GameFlow/
│   ├── GameFlowController.h/cpp
│   ├── GameStateMachine.h/cpp
│   └── SceneManager.h/cpp
├── ObjectPool/
│   ├── ObjectPoolBase.h/cpp
│   ├── ActorObjectPool.h/cpp
│   └── ObjectPoolManager.h/cpp
├── AttributeSystem/
│   ├── GameAttributeTypes.h/cpp
│   ├── AttributeComponent.h/cpp
│   └── AttributeModifier.h/cpp
└── CharacterSystem/
│   ├── AdvancedMovementComponent.h/cpp
│   ├── CameraControllerComponent.h/cpp
│   ├── DodgeComponent.h/cpp
│   └── HookshotComponent.h/cpp
└── WeaponSystem/
│   ├── WeaponTypes.h
│   ├── WeaponInterface.h/cpp
│   ├── WeaponBase.h/cpp
│   ├── WeaponDataAsset.h/cpp
│   ├── WeaponManagerComponent.h/cpp
│   ├── ComboTypes.h
│   ├── ComboDetectorComponent.h/cpp
│   ├── WeaponComboIntegration.h/cpp
│   ├── ComboInputBuffer.h/cpp
│   └── ComboWindowController.h/cpp
└── CombatSystem/
    ├── DamageTypes.h
    ├── DamageCalculator.h/cpp
    ├── DamageableInterface.h
    └── HitReactionComponent.h/cpp
└── EnemySystem/
│   ├── EnemyTypes.h
│   ├── EnemyDataAsset.h/cpp
│   ├── DropTableDataAsset.h/cpp
│   ├── EnemyStateMachineComponent.h/cpp
│   ├── EnemyBase.h/cpp
│   └── EnemyAIController.h/cpp          【4.2.1 新增】
└── AI/
    └── BehaviorTree/
        ├── Tasks/
        │   ├── BTTask_Patrol.h/cpp        【4.2.3 新增】
        │   ├── BTTask_ChasePlayer.h/cpp   【4.2.4 新增】
        │   └── BTTask_Attack.h/cpp        【4.2.5 新增】
        ├── Decorators/
        │   └── BTDecorator_DistanceCheck.h/cpp  【4.2.6 新增】
        └── Services/
            └── BTService_UpdateTarget.h/cpp  【4.2.7 新增】
```

---

## 碰撞通道配置

1. **项目设置** → **碰撞** → **Trace Channels**
2. 创建名为 **"Hookshot"** 的自定义通道
3. 敌人/可拉取物体的碰撞设置：
   - `Hookshot` 通道响应设为 **Block**
4. 钩索组件设置：
   - `HookshotTraceChannel` 选择 **Hookshot**
   - `PullableActorTag` 设为 **Enemy**

---

## 技术备注

- **编译命令**: `E:\UE_5.6\UE_5.6\Engine\Build\BatchFiles\Build.bat niuniu555Editor Win64 Development -Project="C:\Users\Win10\Desktop\niuniu555\niuniu555.uproject" -Progress`
- **UE 版本**: 5.6
- **编译器**: Visual Studio 2022
- **目标平台**: Win64

---

## 如何恢复开发进度

在新对话中恢复开发，请提供以下信息：

### 方式 1：提供项目路径（推荐）

告诉 AI：
> "继续开发我的UE5项目，路径是 `C:\Users\Win10\Desktop\niuniu555`，请先读取 AGENTS.md 文件了解当前状态。"

### 方式 2：上传 AGENTS.md

直接上传本文件内容，AI 会自动了解项目状态。

### 方式 3：简要说明

> "继续开发财神末法冒险项目，目前 Phase 2 已完成（角色基础系统），准备进入 Phase 3 战斗核心开发。"

---

*最后更新: 2026-03-07*  
*开发阶段: Phase 4 基本完成（敌人与AI系统 - 4.2行为树系统完成）*


---

## Bug 修复记录

### 2026-03-06 - 武器系统 BlueprintNativeEvent 调用修复

**问题描述**: 
UE5 崩溃报错 "Do not directly call Event functions in Interfaces. Call Execute_XXX instead."

**根本原因**: 
`BlueprintNativeEvent` 标记的接口函数必须通过 `Execute_` 前缀调用，不能直接作为普通C++函数调用。

**修复文件**: 
- `WeaponManagerComponent.cpp`: 修复13处接口函数调用
- `WeaponBase.cpp`: 修复4处内部接口函数调用

**修复详情**:
```cpp
// 错误写法
CurrentWeapon->CanAttack();
NewWeapon->OnEquipped(OwnerCharacter);

// 正确写法  
IWeaponInterface::Execute_CanAttack(CurrentWeapon);
IWeaponInterface::Execute_OnEquipped(NewWeapon, OwnerCharacter);
```

**经验总结**: 
所有 `BlueprintNativeEvent` 接口函数，无论在类外部还是类内部调用，都必须使用 `IInterface::Execute_函数名(对象, 参数)` 形式。

### 2026-03-07 - EnemyBase 血量显示 0/0 Bug 修复

**问题描述**: 
测试时敌人头顶显示 `HP:0/0`，但数据资产中已设置生命值100。

**根本原因**: 
`EnemyBase.cpp` 中的 `UpdateAttributeStats()` 函数里，属性设置代码被注释：
```cpp
// AttributeComp->SetMaxHealth(Health);
// AttributeComp->SetCurrentHealth(Health);
```
导致 AttributeComponent 没有从数据资产获取初始值。

**修复文件**: 
- `EnemyBase.cpp`: 
  - `UpdateAttributeStats()`: 启用 `SetBaseValue()` 调用，正确设置 MaxHealth 和 CurrentHealth
  - `GetCurrentHealth_Implementation()`: 启用 `AttributeComp->GetHealth()`
  - `GetMaxHealth_Implementation()`: 启用 `AttributeComp->GetMaxHealth()`
  - `GetDefense_Implementation()`: 改用 `AttributeComp->GetDefense()`
  - `TakeInterfaceDamage_Implementation()`: 启用 `ModifyHealth()` 应用伤害
  - `Heal_Implementation()`: 启用 `ModifyHealth()` 应用治疗

**修复详情**:
```cpp
// 设置最大生命值（基础值）
AttributeComp->SetBaseValue(EAttributeType::MaxHealth, Health);
// 设置当前生命值 = 最大生命值（满血初始化）
AttributeComp->SetBaseValue(EAttributeType::Health, Health);
```

**经验总结**: 
C++开发时临时注释的代码在提交前必须恢复，或使用 `#if 0` 明确标记为临时禁用。

### 2026-03-07 - 敌人受击反馈系统修复（最终版）

**问题描述**: 
1. 攻击命中后血量不减少（始终显示HP:100/100）
2. 状态机未进入Hit状态（无受击硬直）
3. 无闪白、无击退、无音效（HitReactionComponent未触发）
4. **根因**: `IsAlive()` 检查 `GetCurrentHealth() > 0`，但 CurrentHealth 初始值为 0，导致 `TakeInterfaceDamage` 提前返回

**根本原因**: 
1. `AttributeComp->GetHealth()` 初始返回 0（未正确初始化 CurrentHealth）
2. `IsAlive()` 返回 false，攻击逻辑在入口处直接 return
3. 未执行 `ModifyHealth`、`PlayHitReaction`、`StartHit`

**修复文件**: 
- `EnemyBase.cpp`:
  - `BeginPlay()`: 添加**强制血量初始化**逻辑（关键修复！）
  - 如果 CurrentHealth <= 0，强制设置为 MaxHealth（默认100）
  - 添加详细的血量检查日志

**修复详情**:
```cpp
// ===== 关键修复：确保血量不为0 =====
if (AttributeComp)
{
    float CurrentHealth = AttributeComp->GetHealth();
    float MaxHealth = AttributeComp->GetMaxHealth();
    
    // 如果当前血量为0或未初始化
    if (CurrentHealth <= 0.0f)
    {
        if (MaxHealth <= 0.0f)
        {
            MaxHealth = 100.0f;
            AttributeComp->SetBaseValue(EAttributeType::MaxHealth, MaxHealth);
        }
        
        // 关键：设置当前血量 = 最大生命值
        AttributeComp->SetBaseValue(EAttributeType::Health, MaxHealth);
        UE_LOG(LogTemp, Warning, TEXT("[EnemyBase] 血量强制初始化为:%.0f"), MaxHealth);
    }
}
// ====================================
```

**验证日志**（修复后预期输出）:
```
[EnemyBase] XXX 血量检查 - 当前:0 最大:0      ← 发现未初始化
[EnemyBase] XXX 血量强制初始化为:100          ← 强制修复
[EnemyBase] XXX 初始化完成 - 最终血量:100/100  ← 验证成功
[EnemyBase] TakeDamage被调用, 原始伤害:171   ← 攻击命中
[EnemyBase] 血量变化: 100 -> 70               ← 扣血成功！
[EnemyStateMachine] Idle -> Hit               ← 状态切换成功！
```

**经验总结**: 
1. **防御性编程**: 在关键检查点（如IsAlive）前确保数据已初始化
2. **详细日志**: 在初始化流程中添加检查点日志，快速定位问题
3. **根因分析**: 从现象（不扣血）→ 检查条件（IsAlive）→ 追踪数据来源（GetHealth）

### 2026-03-07 - 敌人受击反馈系统修复（紧急最终版）

**问题描述**: 
1. 攻击命中后血量不减少（始终显示HP:100/100）
2. 状态机未进入Hit状态（无受击硬直）
3. 无闪白、无击退、无音效（HitReactionComponent未触发）
4. **关键证据**: `[EnemyBase] TakeDamage被调用` **日志完全未出现**

**最终根因**: 
**蓝图覆盖了接口函数！** `TakeInterfaceDamage` 声明为 `BlueprintNativeEvent`，`BP_TestEnemy` 蓝图中实现了空版本，导致 C++ 的 `_Implementation` 函数**永不执行**。

```
调用链断裂：
WeaponBase::Attack → IDamageableInterface::Execute_TakeInterfaceDamage
                              ↓
                    BP_TestEnemy.TakeInterfaceDamage (蓝图空实现)
                              ↓
                         直接返回
                              ↓
                    C++ TakeInterfaceDamage_Implementation 被跳过！
```

**修复方案**（采用方案A-强制C++执行）：
- `EnemyBase.h`: `BlueprintNativeEvent` → `BlueprintCallable`（禁止蓝图覆盖）
- `EnemyBase.cpp`: 删除 `_Implementation` 后缀，改为普通函数实现
- 添加 **【强制日志】** 作为函数第一行，确保能追踪调用

**修复详情**:
```cpp
// EnemyBase.h - 禁止蓝图覆盖，强制走C++
UFUNCTION(BlueprintCallable, Category = "伤害接口")
float TakeInterfaceDamage(const FHitInfo& HitInfo);

// EnemyBase.cpp - 第一行添加强制日志
float AEnemyBase::TakeInterfaceDamage(const FHitInfo& HitInfo)
{
    UE_LOG(LogTemp, Warning, TEXT("【EnemyBase扣血函数进入】%s 伤害:%.0f"), ...);
    // ...
}
```

**替代方案**（如果必须保留蓝图扩展性）：
在 `BP_TestEnemy` 蓝图中：
```
Event Take Interface Damage (Hit Info)
  ↓
// 必须调用父类！否则C++逻辑不执行
Parent: Take Interface Damage(Hit Info)
  ↓
// 蓝图自定义逻辑...
```

**验证日志**（修复后预期输出）:
```
【EnemyBase扣血函数进入】BP_TestEnemy 伤害:110, 当前血量:100, IsAlive:1
【EnemyBase】BP_TestEnemy 准备扣血: 100 - 110
【EnemyBase】BP_TestEnemy 血量变化: 100 -> -10
【EnemyBase】BP_TestEnemy 进入受击状态
[EnemyStateMachine] BP_TestEnemy - 状态转换: Idle -> Hit
```

**经验总结**: 
1. **BlueprintNativeEvent 陷阱**: 允许蓝图覆盖的接口函数，如果蓝图实现为空或不调用父类，C++实现会被完全跳过
2. **强制日志验证**: 在关键函数第一行添加 `UE_LOG(Warning)`，用于验证函数是否被调用
3. **接口设计原则**: 对于核心逻辑（如扣血、死亡），优先使用 `BlueprintCallable` 禁止覆盖，或通过 `Execute_` 强制调用C++实现

### 2026-03-07 - Phase 4.2 AI行为树系统完成

**完成功能**:
- ✅ **EnemyAIController** (4.2.1): AI控制器基类，感知组件配置，黑板Key管理，目标自动发现
- ✅ **BTTask_Patrol** (4.2.3): 巡逻任务节点，随机导航点移动，到达等待
- ✅ **BTTask_ChasePlayer** (4.2.4): 追击任务节点，持续跟随目标，最大追击距离限制
- ✅ **BTTask_Attack** (4.2.5): 攻击任务节点，前摇-攻击-后摇状态机
- ✅ **BTDecorator_DistanceCheck** (4.2.6): 距离检查装饰器，4种比较模式
- ✅ **BTService_UpdateTarget** (4.2.7): 目标更新服务，定期更新目标位置/距离/方向

**修复问题**:
1. **BTDecorator_DistanceCheck.h EditCondition错误**: `EDistanceMode` → `EDistanceCheckMode`
2. **BTTask_ChasePlayer追击起始位置**: 优化为优先从黑板读取，在EnemyAIController::OnTargetChanged中统一记录
3. **BTTask_Patrol导航点生成**: 添加中心点导航投影检查作为后备
4. **EnemyStateMachineComponent返回逻辑**: 受击/眩晕后智能选择返回状态（Idle/Chase）

**Bug修复 - TickTask目标获取不一致**:
- **问题**: `TickTask`中使用`SelectedKeyType`判断，但`ExecuteTask`使用3种后备方案获取目标，导致TickTask中判定"目标丢失"而ExecuteTask成功
- **修复**: 统一`BTTask_ChasePlayer`和`BTTask_Attack`的`TickTask`目标获取逻辑，使用与`ExecuteTask`相同的3种后备方案
- **关键代码**:
```cpp
// 方法1：使用配置KeyName直接获取
if (!TargetActorKey.SelectedKeyName.IsNone())
{
    TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
}

// 方法2：如果方法1失败，尝试用KeyID获取
if (!TargetActor && TargetActorKey.GetSelectedKeyID() != FBlackboard::InvalidKey)
{
    TargetActor = Cast<AActor>(Blackboard->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID()));
}

// 方法3：后备方案，直接查找"TargetActor"
if (!TargetActor)
{
    TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(FName(TEXT("TargetActor"))));
}
```

**Bug修复 - 距离计算不一致导致无法进入攻击**:
- **问题**: 追击任务使用`FVector::Dist`(3D距离)，行为树装饰器使用2D距离，当玩家和敌人Z轴有差距时，追击任务判定"进入范围"返回成功，但装饰器判定"不满足条件"，导致攻击任务不执行，AI反复在追击任务中循环
- **修复**: 统一使用`FVector::Dist2D`(2D距离，忽略Z轴)，与行为树装饰器保持一致
- **修改文件**: `BTTask_ChasePlayer.cpp`(IsInAttackRange, ShouldAbortChase), `BTTask_Attack.cpp`(IsTargetInAttackRange)
- **关键代码**:
```cpp
// 使用2D距离（忽略Z轴），与行为树装饰器保持一致
float DistanceToTarget = FVector::Dist2D(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());
return DistanceToTarget <= AttackRange;
```

**Bug修复 - TickTask未被调用导致攻击无法执行**:
- **问题**: `BTTask_Attack`继承自`UBTTask_BlackboardBase`，`ExecuteTask`返回`InProgress`后`TickTask`从未被调用，导致攻击状态机无法推进
- **根本原因**: `UBTTask_BlackboardBase`可能不支持`TickTask`或其实现覆盖了子类的`TickTask`
- **修复**: 将基类改为`UBTTaskNode`，并添加`InitializeFromAsset`强制启用`bNotifyTick`
- **修改文件**: `BTTask_Attack.h`, `BTTask_Attack.cpp`
- **关键代码**:
```cpp
// 头文件：改为继承自 UBTTaskNode
class NIUNIU555_API UBTTask_Attack : public UBTTaskNode

// CPP文件：强制启用 TickTask
void UBTTask_Attack::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    bNotifyTick = true;  // 强制启用
}
```

**Bug修复 - ExecuteTask未被调用问题排查**:
- **问题**: `InitializeFromAsset`被调用但`ExecuteTask`未被调用，攻击任务完全不执行
- **诊断方法**: 添加了详细的诊断日志（类名、实例指针），帮助确认：
  1. 构造函数是否被调用（确认类被实例化）
  2. InitializeFromAsset的类名和实例指针
  3. ExecuteTask是否被调用
- **常见原因**: 
  - 行为树中使用了蓝图子类而不是C++类
  - 节点连接错误（Task不是Sequence的直接子节点）
  - 装饰器条件始终不满足
- **排查步骤**: 详见`TestGuide_AI_BehaviorTree.md`的"攻击任务执行问题排查"章节

**新增文件**:
```
Source/niuniu555/AI/
├── EnemyAIController.h/cpp
└── BehaviorTree/
    ├── Tasks/
    │   ├── BTTask_Patrol.h/cpp
    │   ├── BTTask_ChasePlayer.h/cpp
    │   └── BTTask_Attack.h/cpp
    ├── Decorators/
    │   └── BTDecorator_DistanceCheck.h/cpp
    └── Services/
        └── BTService_UpdateTarget.h/cpp
```

**测试文档**: `TestGuide_AI_BehaviorTree.md`
- 详细的黑板Key配置说明
- 行为树结构搭建步骤
- 玩家角色配置（Player标签 + AIPerceptionStimuliSource）
- 配置检查清单和常见错误排查

---

## 开发日志

### 2026-03-06 - 武器切换致命 Bug 修复
**问题:** 用户反馈武器切换后显示的还是旧武器类型，连招段数也没更新。

**根因分析:**
- `TickComponent` 中的武器切换计时器到期后，调用了 `FinishWeaponSwitch()` 而不是 `DoWeaponSwitch()`
- `FinishWeaponSwitch()` 只广播事件，没有执行实际的武器切换逻辑
- 导致 `CurrentWeapon` 没有被更新，仍然是旧武器

**修复:**
- `WeaponManagerComponent::TickComponent`: 计时器到期后调用 `DoWeaponSwitch()` 执行实际切换
- `WeaponManagerComponent::DoWeaponSwitch`: 移除条件判断，总是调用 `FinishWeaponSwitch()` 完成流程

**Bug代码:**
```cpp
// 错误（第52行）
if (WeaponSwitchTimer <= 0.0f)
{
    FinishWeaponSwitch();  // 只广播事件，没有实际切换
}

// 正确
if (WeaponSwitchTimer <= 0.0f)
{
    DoWeaponSwitch();  // 执行卸下旧武器、创建新武器、设置CurrentWeapon
}
```

**测试验证:**
修复后武器切换流程正确：
1. 调用 `EquipWeapon(PanguAxe)`
2. 启动 0.5 秒切换计时器
3. 计时器到期 → 调用 `DoWeaponSwitch()`
4. `DoWeaponSwitch` 执行：卸下剑 → 创建斧头 → 设置 `CurrentWeapon = 斧头`
5. `FinishWeaponSwitch` 广播事件
6. 连招系统重新注册新武器的连招配置

### 2026-03-06 - 武器切换问题排查（次要）
**问题:** 如果两个武器的 `WeaponType` 设置相同，缓存机制会返回错误的武器实例。

**根因分析:**
- `WeaponManager` 使用 `WeaponType` 枚举作为武器实例缓存的键
- 例如：`DA_TestAxe.WeaponType` 错误设置为 `Sword`，切换时会返回缓存的剑实例

**检查方法:**
查看日志：`[WeaponManager] 开始切换武器 - 从 X 到 Y`
- 如果 X 和 Y 相同，说明 `WeaponType` 配置错误
- 正确应该是：`从 EWeaponType::Sword 到 EWeaponType::PanguAxe`

### 2026-03-06 - 连招系统数据流修复
**问题:** `WeaponComboIntegration` 调用 `GetMaxComboStages()` 获取连招段数，但 `WeaponBase` 使用 `LightComboStages` 数组，而 `WeaponDataAsset` 使用 `LightAttackSections`。数据流不一致导致连招段数不正确。

**修复:**
- `WeaponBase`: 添加 `WeaponDataAsset` 引用和 `LoadConfigFromDataAsset()` 函数
- `WeaponComboIntegration`: 从武器的 `LightComboStages` / `HeavyComboStages` 读取配置生成连招
- 更新 `TestGuide_ComboSystem.md` 测试指南

**编译状态:** ✅ 成功

### 2026-03-06
- ✅ Phase 3 模块1: 武器系统架构完成
- ✅ 创建武器基类 AWeaponBase
- ✅ 创建武器数据资产 UWeaponDataAsset
- ✅ 创建武器管理器 UWeaponManagerComponent
- ✅ 修复 BlueprintNativeEvent 调用方式导致的崩溃问题
- ✅ 完成武器系统测试指南
- 🎉 武器系统功能测试通过
- ✅ Phase 3 模块2: 连招系统完成
- ✅ 创建连招检测器 ComboDetectorComponent
- ✅ 创建武器连招集成 WeaponComboIntegration
- ✅ 创建输入缓冲 ComboInputBuffer
- ✅ 创建窗口控制器 ComboWindowController
- ✅ 完成连招系统测试指南
- 🎉 连招系统开发完成

---

### 2026-03-06 - Phase 3 模块3 完成
**状态:** ✅ 伤害计算与受击反馈系统开发完成并通过测试

**开发原则执行:**
- ✅ 大功能拆分为小功能逐步实现
- ✅ 每个小功能开发后立即编译验证
- ✅ 所有功能编译成功后再进行下一步

**完成步骤:**
1. **DamageTypes.h** - 伤害计算参数和结果结构 ✅
2. **DamageCalculator** - 通用伤害计算器 ✅
3. **DamageableInterface** - 可受伤对象接口 ✅
4. **HitReactionComponent** - 受击反馈组件 ✅

**核心功能:**
- 伤害公式: 基础伤害 → 防御减免 → 抗性减免 → 暴击 → 浮动
- 受击反馈: 闪白/顿帧/击退/受击动画
- 接口系统: 任何Actor可实现IDamageableInterface成为可受伤目标

**Bug修复记录:**
1. **顿帧冻结游戏**: `HitStopTimeDilation = 0.0f` 完全冻结世界，改为 `0.05f` 实现子弹时间效果
2. **接口 Cast 失败**: `Cast<IDamageableInterface>` 对蓝图接口返回 nullptr，改为 `ImplementsInterface` 检测
3. **击退逻辑不可靠**: 原 `MoveSmooth` 实现改为 `LaunchCharacter`，添加 `bRunPhysicsWithNoController` 支持无Controller角色

**编译状态:** ✅ 全部成功

**测试状态:** ✅ 通过
- 基础攻击命中 ✓
- 伤害数值计算 ✓
- 暴击系统（顿帧效果）✓
- 防御减免 ✓
- 受击反馈（闪白/击退逻辑）✓

### 2026-03-06 - Phase 3 模块2 完成
**状态:** ✅ 连招系统开发完成并通过测试

**完成功能:**
- ComboTypes - 连招数据类型定义
- ComboDetectorComponent - 连招检测器，识别输入序列
- WeaponComboIntegration - 武器与连招系统集成
- ComboInputBuffer - 输入缓冲子系统
- ComboWindowController - 连招取消窗口管理

**Bug修复记录:**
1. 武器切换计时器调用错误函数（FinishWeaponSwitch → DoWeaponSwitch）
2. WeaponBase数据加载流程优化
3. 数据资产与武器实例配置同步

**测试状态:** ✅ 通过
- 基础连招触发 ✓
- 连打测试 ✓
- 连招超时中断 ✓
- 武器切换连招更新 ✓

---

*最后更新: 2026-03-06*  
*Phase 3 基本完成（武器+连招+伤害），有3项遗漏功能*  
*下一步: Phase 4 敌人与AI系统（推荐优先）或补充遗漏功能*

---

## 📋 前三阶段遗漏功能汇总

### 遗漏清单

| 阶段 | 功能 | 优先级 | 说明 |
|------|------|--------|------|
| **Phase 2** | CharacterStateMachine | 🔶 中 | 完整角色状态机（Idle/Move/Attack/Dodge/Hit/Stun/Dead），用于统一协调各组件状态 |
| **Phase 3** | 准星与锁定系统 | 🔶 中 | 软锁定/硬锁定/锁定UI，**建议等Phase 4敌人系统完成后开发** |
| **Phase 3** | 派生攻击触发器 | 🔷 低 | 特定连招（如LLH）最后一击触发特殊效果（爆炸/冰冻等） |
| **Phase 3** | 武器切换取消 | 🔷 低 | 攻击中途切换武器触发独特转场技（高级功能） |

### 优先级建议

**🔴 立即进入 Phase 4（推荐）**
- 敌人系统是当前最大缺口，有敌人才能验证完整战斗循环
- 准星/锁定系统在敌人完成后开发更有意义
- 角色状态机可在 Phase 4 开发中顺便实现

**🔵 先补充遗漏功能（可选）**
- 如果希望战斗系统更完整，可补充角色状态机
- 准星/锁定系统依赖敌人存在，不建议现在开发

### 当前可用功能

✅ **已完整可用**:
- 基础移动（跑/停/转）+ 闪避 + 钩索
- 武器系统（4种武器类型 + 切换）
- 连招系统（3段轻击/重击连打）
- 伤害计算（攻防+暴击+受击反馈）

**足够支撑 Phase 4 敌人系统开发！**

---

## 开发防坑指南

### 通用排错清单

| 症状 | 可能原因 | 解决方案 |
|------|----------|----------|
| "未找到注册武器" 类错误 | Map/Enum 的 Key 不匹配（大小写敏感） | 检查武器类型枚举与数据资产配置是否一致 |
| "Execute_XXX" 崩溃 | 未使用 Execute_ 前缀调用 BlueprintNativeEvent | 所有 BlueprintNativeEvent 接口必须使用 `IInterface::Execute_函数名(对象, 参数)` |
| "无反应但无报错" | 函数未被实际调用 | 添加 `UE_LOG` 日志确认函数调用链 |
| 修改 C++ 后无效 | 未关闭编辑器直接热重载 | **必须关闭编辑器后编译**，不能直接热重载 |

### 接口调用黄金法则

**❌ 错误：**
```cpp
// BlueprintNativeEvent 不能直接调用
DamageableTarget->TakeDamage(HitInfo);
NewWeapon->OnEquipped(Owner);
```

**✅ 正确：**
```cpp
// 必须使用 Execute_ 前缀
IDamageableInterface::Execute_TakeDamage(HitActor, HitInfo);
IWeaponInterface::Execute_OnEquipped(NewWeapon, Owner);
```

### 蓝图接口陷阱

**❌ Cast 方式失败：**
```cpp
// 对蓝图实现的接口返回 nullptr
IDamageableInterface* DamageableTarget = Cast<IDamageableInterface>(HitActor);
if (DamageableTarget) { /* 永远不会执行 */ }
```

**✅ 正确检测方式：**
```cpp
// 使用 ImplementsInterface 检查
bool bImplementsInterface = HitActor->GetClass()->ImplementsInterface(UDamageableInterface::StaticClass());
if (bImplementsInterface)
{
    IDamageableInterface::Execute_TakeDamage(HitActor, HitInfo);
}
```


---

## 开发规范与常见陷阱 (Debug Log)

### 1. C++接口与蓝图交互规范

#### ❌ 错误: 函数名不匹配导致接口调用失效

**现象**: WeaponBase调用`TakeDamage`，EnemyBase实现`TakeInterfaceDamage`，导致扣血逻辑被跳过

**规范**: 接口函数名必须**完全一致**（区分大小写），建议使用`Execute_`前缀调用时仔细检查拼写

**检查**: 日志中应出现`【扣血函数进入】`，若未出现则检查函数签名是否匹配

**修复示例**:
```cpp
// 错误 - 函数名不匹配
UFUNCTION(BlueprintCallable, Category = "伤害接口")
float TakeInterfaceDamage(const FHitInfo& HitInfo);  // 实现的是TakeInterfaceDamage

// 正确 - 与接口定义完全一致
virtual float TakeDamage_Implementation(const FHitInfo& HitInfo) override;  // 实现TakeDamage
```

---

#### ❌ 错误: BlueprintNativeEvent被蓝图空实现覆盖

**现象**: 蓝图接口面板显示`TakeDamage`为黄色闪电（已实现），但事件图表为空，导致C++逻辑永不执行

**规范**: 慎用`BlueprintNativeEvent`，如果必须使用，确保蓝图实现中调用`Parent: 函数名`

**建议**: 核心逻辑（扣血、死亡）使用纯C++实现（去掉`BlueprintNativeEvent`），通过委托事件让蓝图扩展

**修复示例**:
```cpp
// 危险 - 允许蓝图覆盖，可能被空实现
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
float TakeDamage(const FHitInfo& HitInfo);

// 安全 - 禁止蓝图覆盖，强制走C++
UFUNCTION(BlueprintCallable, Category = "伤害接口")
float TakeDamage(const FHitInfo& HitInfo);
```

---

#### ❌ 错误: 血量初始化失败 (CurrentHealth = 0)

**现象**: `IsAlive()`返回false，导致`TakeDamage`第一行就return，无法扣血

**规范**: 在`BeginPlay()`中强制初始化：`AttributeComp->SetCurrentHealth(MaxHealth)`

**验证**: 运行游戏后检查Details面板，CurrentHealth必须为正值（如100）

**修复示例**:
```cpp
void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    
    // 强制确保血量初始化（关键！）
    if (AttributeComp && AttributeComp->GetCurrentHealth() <= 0)
    {
        float MaxHP = AttributeComp->GetMaxHealth();
        if (MaxHP <= 0) MaxHP = 100.0f;
        
        AttributeComp->SetCurrentHealth(MaxHP);
        UE_LOG(LogTemp, Log, TEXT("[EnemyBase] 强制初始化血量: %.0f"), MaxHP);
    }
}
```

---

### 2. UE5编辑器操作规范

#### ❌ 错误: 创建Data Asset时误选Blueprint Class

**现象**: 创建了蓝色图标的Blueprint Class，导致Enemy Data下拉框无法识别

**规范**: 创建路径必须是**Miscellaneous → Data Asset**（白色文档图标），不是Blueprint Class

**验证**: 创建后图标应为白色📄，不是蓝色🔷

**操作步骤**:
```
右键内容浏览器 → Miscellaneous（杂项） → Data Asset（数据资产）
→ 选择类型 → 命名（如DA_TestEnemy）
→ 验证图标为白色文档
```

---

#### ❌ 错误: 重复添加继承组件

**现象**: 组件面板出现`StateMachineComp`（灰色/继承）和`EnemyStateMachine`（白色/手动）两个状态机

**规范**: 从EnemyBase继承的组件（AttributeComp/HitReactionComp/StateMachineComp）**严禁**手动添加

**识别**: 继承组件显示灰色图标，手动添加显示白色图标

**正确操作**:
```
组件面板应显示（灰色=继承）:
├── AttributeComp        【灰色】✅ 已继承，无需添加
├── HitReactionComp      【灰色】✅ 已继承，无需添加
└── StateMachineComp     【灰色】✅ 已继承，无需添加
```

---

#### ❌ 错误: Show Debug Info未启用

**现象**: 敌人头顶不显示HP和State，只能在日志看状态

**规范**: 在BP_TestEnemy的**Class Defaults → Debug**分类中勾选**Show Debug Info**

**注意**: 不是在组件细节面板，而是在类默认值（Class Defaults）中

**操作步骤**:
```
打开BP_TestEnemy → 点击Class Defaults（右上角）
→ 左侧面板搜索"show"
→ 勾选Show Debug Info
→ 编译保存
```

---

### 3. 版本控制规范

#### ❌ 错误: 提交.vs/文件夹导致GitHub拒绝推送

**现象**: 推送时报错`File .vs/xxx is 1310 MB, exceeds 100.00 MB limit`

**规范**: .gitignore必须包含:
```
.vs/
*.sln
*.vcxproj
*.vcxproj.filters
*.vcxproj.user
Binaries/
DerivedDataCache/
Intermediate/
Saved/
```

**修复步骤**:
```bash
# 1. 添加到.gitignore
echo ".vs/" >> .gitignore

# 2. 从缓存中删除已跟踪的大文件
git rm -r --cached .vs/
git rm -r --cached Binaries/
git rm -r --cached Intermediate/

# 3. 提交并推送
git add .gitignore
git commit -m "Remove large files and update .gitignore"
git push
```

---

### 4. 调试日志黄金法则

**原则**: 在关键函数第一行添加`UE_LOG(Warning)`，确保能追踪到函数是否被调用

**必加日志的位置**:
```cpp
// 接口函数入口
float AEnemyBase::TakeDamage_Implementation(const FHitInfo& HitInfo)
{
    UE_LOG(LogTemp, Warning, TEXT("【扣血函数进入】%s 伤害:%.0f"), *GetName(), HitInfo.Damage);
    // ...
}

// 初始化完成
void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[EnemyBase] %s BeginPlay 初始化完成"), *GetName());
    // ...
}

// 状态变更
void AEnemyStateMachineComponent::ForceStateChange(EEnemyState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("[StateMachine] %s: %s -> %s"), 
        *GetName(), *UEnum::GetValueAsString(CurrentState), *UEnum::GetValueAsString(NewState));
    // ...
}
```

---

### 5. 快速诊断表

| 现象 | 检查点 | 解决方案 |
|------|--------|----------|
| 扣血函数日志未出现 | 函数名匹配 | 检查`TakeDamage` vs `TakeInterfaceDamage` |
| 扣血函数进入但无后续 | IsAlive()检查 | 确认CurrentHealth > 0 |
| 血量不变 | ModifyHealth调用 | 检查AttributeComp是否为NULL |
| 无状态切换 | HitReaction类型 | 确认HitInfo.HitReaction不为None |
| 无闪白 | HitReactionComp配置 | 检查FlashDuration > 0 |
| 头顶不显示HP | Show Debug Info | 在Class Defaults中勾选 |
| 有两个同名组件 | 重复添加 | 删除白色图标的（手动添加的） |
| DA无法识别 | 创建类型错误 | 确认是Data Asset（白色图标）不是Blueprint Class |

---

*最后更新: 2026-03-07*  
*整理自: Phase 4.1 敌人系统开发Debug Log*
