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
| Phase 4 | 敌人与 AI | ⏳ 待开发 | 敌人基类、行为树、生成器、掉落系统 |
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

### 4.1 敌人基础架构

- 敌人基类（继承于统一Entity基类，与Player共享属性接口）
- 敌人状态机（Spawn/Idle/Patrol/Chase/Attack/Hit/Return/Dead）
- 敌人配置数据（ScriptableObject：模型/技能组/掉落表/AI参数）

### 4.2 AI行为树系统

- 行为树节点库（Selector/Sequence/Parallel/Decorator）
- 感知系统（视觉锥形检测 / 听觉半径 / 伤害来源记忆）
- 战术AI（包夹/躲避/技能冷却管理/优先级目标选择）
- 精英与BOSS专用AI（阶段转换/狂暴模式/特殊机制）

### 4.3 敌人生成器

- 生成点管理器（触发区域/波次配置/最大同时存在数）
- 动态难度调整（根据玩家当前强度实时调整敌人数值/种类）
- 生成特效（魔物降临粒子/黑雾动画）
- 稀有敌人刷出逻辑（发光精英怪概率控制）

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

*最后更新: 2026-03-06*  
*开发阶段: Phase 3 进行中（战斗核心系统）*


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
