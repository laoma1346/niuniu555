# 财神末法冒险 (niuniu555) - 开发状态文档

## 项目概述

- **项目名称**: 财神末法冒险
- **类型**: 3D ARPG Roguelike
- **引擎**: Unreal Engine 5.6
- **项目路径**: `C:\Users\Win10\Desktop\niuniu555`

---

## 开发工作规范（重要）

### Git工作流程

| 操作 | 权限 | 说明 |
|------|------|------|
| `git status` / `git log` | ✅ AI可执行 | 查看状态，不涉及修改 |
| `git add` | ⚠️ 需用户确认 | 暂存文件前需告知用户 |
| `git commit` | ⚠️ 需用户确认 | 提交前需告知用户提交信息 |
| `git push` | ❌ **必须用户授权** | **严禁AI自主推送到远程仓库** |
| `git pull/fetch` | ⚠️ 需用户确认 | 可能引发冲突 |
| `git merge/rebase` | ❌ 需用户授权 | 分支操作必须由用户决定 |

**重要规则**:
1. **Git提交到远程仓库的操作必须由用户明确授权后才能执行**
2. AI不得自主、自动、未经用户同意就提交备份上传到Git仓库
3. 每次Git操作前，AI必须询问用户："是否需要执行git [操作]？"
4. 用户明确说"提交"、"push"、"保存到git"等指令后，AI才能执行相应操作

### 文件修改规范

1. 修改文件前告知用户修改范围和原因
2. 重大修改（删除文件、修改配置）必须经用户确认
3. 保留.bak备份文件需告知用户

---

## 开发路线图（10个阶段）

### 阶段总览

| 阶段 | 名称 | 状态 | 核心内容 |
|------|------|------|----------|
| Phase 1 | 基础架构与数据层 | 完成 | 存档、输入、状态机、对象池、属性系统 |
| Phase 2 | 角色基础与移动 | 基本完成 | 移动、闪避、钩索、角色状态机(待开发) |
| Phase 3 | 战斗核心系统 | 基本完成 | 武器、连招、伤害、准星/锁定(待开发) |
| Phase 4 | 敌人与AI | 完成 | 敌人基类、行为树、生成器、掉落系统 |
| Phase 5 | 装备与背包系统 | 完成 | 装备数据、套装系统、背包、属性集成 |
| Phase 6 | 关卡与流程 | 待开发 | 房间/节点系统、地图生成、节点交互 |
| Phase 7 | 经济与成长系统 | 待开发 | 金币、神格碎片、技能树、局内成长 |
| Phase 8 | UI 与体验 | 待开发 | 漫画风格UI、HUD、引导系统 |
| Phase 9 | 美术表现与特效 | 待开发 | VFX、角色外观、场景表现 |
| Phase 10 | 平衡与配置 | 待开发 | 数值配置、调试工具、平衡报表 |

### Phase 4 开发状态细分
- **4.1 敌人基础架构**: 完成 (EnemyBase + EnemyStateMachineComponent + EnemyDataAsset)
- **4.2 AI行为树系统**: 完成 (EnemyAIController + BTTasks + BTDecorators + BTServices)
- **4.3 敌人生成器**: 完成 (EnemySpawnPoint + EnemySpawner)
- **4.4 敌人死亡与掉落系统**: 完成 (金币、神格碎片、装备)

---

## Phase 1 - 基础架构与数据层 (100% 完成)

| 系统 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **SaveSystem** | 完成 | `SaveSystem/` 文件夹 | 本地存档加密/解密、跨局持久化数据、多存档槽位管理 |
| **InputSystem** | 完成 | `InputSystem/` 文件夹 | 输入设备抽象、战斗输入缓冲、连招输入序列缓存 |
| **GameStateMachine** | 完成 | `GameFlow/` 文件夹 | 游戏循环状态机、异步场景加载、内存管理 |
| **ObjectPool** | 完成 | `ObjectPool/` 文件夹 | 全局对象池管理器（特效/伤害数字/掉落物） |
| **AttributeSystem** | 完成 | `AttributeSystem/` 文件夹 | 基础属性结构体、属性修饰器接口、属性变化事件分发 |

---

## Phase 2 - 角色基础与移动 (100% 完成)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **AdvancedMovementComponent** | 完成 | `CharacterSystem/AdvancedMovementComponent.h/cpp` | 3D角色移动逻辑、移动状态机 |
| **CameraControllerComponent** | 完成 | `CharacterSystem/CameraControllerComponent.h/cpp` | 摄像机模式切换、锁定目标管理 |
| **CharacterStateMachine** | 待开发 | - | 完整角色状态机（Idle/Move/Attack/Dodge/Hit/Stun/Dead） |
| **DodgeComponent** | 完成 | `CharacterSystem/DodgeComponent.h/cpp` | 闪避动作资源管理、无敌帧时间窗口 |
| **HookshotComponent** | 完成 | `CharacterSystem/HookshotComponent.h/cpp` | 钩索瞄准射线检测、仅支持将敌人拉回 |

---

## Phase 3 - 战斗核心系统

### 3.1 武器系统架构 (已完成)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **WeaponTypes** | 完成 | `WeaponSystem/WeaponTypes.h` | 武器类型枚举、伤害结构体、连招信息结构体 |
| **WeaponInterface** | 完成 | `WeaponSystem/WeaponInterface.h/cpp` | IWeaponInterface 接口定义 |
| **WeaponBase** | 完成 | `WeaponSystem/WeaponBase.h/cpp` | AWeaponBase 武器基类 |
| **WeaponDataAsset** | 完成 | `WeaponSystem/WeaponDataAsset.h/cpp` | UWeaponDataAsset 数据资产 |
| **WeaponManagerComponent** | 完成 | `WeaponSystem/WeaponManagerComponent.h/cpp` | 武器管理器 |

**武器类型设计**:
| 武器类型 | 设计理念 | 核心机制 |
|----------|----------|----------|
| 轩辕剑 | 近战武器 | 连击系统、格挡反击 |
| 盘古斧 | 近战/远程混合 | 近战蓄力、远程投掷、召回机制 |
| 财神金杖 | 远程法术 | 弹道法术、范围效果 |
| 射日弓 | 远程物理 | 蓄力射击、贯穿、散射 |

### 3.2 连招系统 (已完成)

- ComboTypes - 连招数据类型定义
- ComboDetectorComponent - 连招检测器
- WeaponComboIntegration - 武器与连招集成
- ComboInputBuffer - 输入缓冲子系统
- ComboWindowController - 连招取消窗口管理

### 3.3 伤害计算与受击反馈 (已完成)

**伤害公式**:
```
基础伤害 = 攻击力 * 技能倍率
防御减免 = Max(基础伤害 * 0.1, 基础伤害 - 防御力)
抗性减免 = 防御减免 * (1 - 抗性)
暴击伤害 = 减免后伤害 * 暴击倍率
最终伤害 = 暴击伤害 * 随机浮动(0.9-1.1) + 固定加成
```

**受击反馈类型**:
- 材质闪白: 受击时角色模型闪红/闪白
- 顿帧(Hit Stop): 暴击或重击时游戏时间变为5%
- 击退: 根据攻击方向使用 LaunchCharacter 推开目标
- 受击动画: 轻击/重击/击倒不同动画反应

### 3.4 准星与锁定系统 (待开发)

---

## Phase 4 - 敌人与AI

### 4.1 敌人基础架构 (已完成)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **EnemyTypes** | 完成 | `EnemySystem/EnemyTypes.h` | 敌人类型/状态/阵营枚举 |
| **EnemyDataAsset** | 完成 | `EnemySystem/EnemyDataAsset.h/cpp` | 敌人数据资产 |
| **DropTableDataAsset** | 完成 | `EnemySystem/DropTableDataAsset.h/cpp` | 掉落表数据资产 |
| **EnemyStateMachineComponent** | 完成 | `EnemySystem/EnemyStateMachineComponent.h/cpp` | 敌人状态机组件 |
| **EnemyBase** | 完成 | `EnemySystem/EnemyBase.h/cpp` | 敌人基类 |

### 4.2 AI行为树系统 (已完成)

| 子模块 | 状态 | 文件 | 功能 |
|--------|------|------|------|
| **EnemyAIController** | 完成 | `AI/EnemyAIController.h/cpp` | AI控制器基类 |
| **BTTask_Patrol** | 完成 | `AI/BehaviorTree/Tasks/BTTask_Patrol.h/cpp` | 巡逻任务节点 |
| **BTTask_ChasePlayer** | 完成 | `AI/BehaviorTree/Tasks/BTTask_ChasePlayer.h/cpp` | 追击任务节点 |
| **BTTask_Attack** | 完成 | `AI/BehaviorTree/Tasks/BTTask_Attack.h/cpp` | 攻击任务节点 |
| **BTDecorator_DistanceCheck** | 完成 | `AI/BehaviorTree/Decorators/BTDecorator_DistanceCheck.h/cpp` | 距离检查装饰器 |
| **BTService_UpdateTarget** | 完成 | `AI/BehaviorTree/Services/BTService_UpdateTarget.h/cpp` | 目标更新服务 |

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

### 4.3 敌人生成器 (已完成)

- **EnemySpawnPoint** - 生成点组件
- **EnemySpawner** - 生成器管理器
- **双模式支持**: AutoSpawn独立生成模式 / 波次模式

### 4.4 敌人死亡与掉落系统 (已完成)

- 死亡动画与布娃娃物理切换
- 金币掉落
- 装备掉落判定
- 神格碎片掉落

---

## Phase 5 - 装备与背包系统 (已完成)

### 5.1 装备数据架构

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **EquipmentTypes** | 完成 | `EquipmentSystem/EquipmentTypes.h` | 装备品质/槽位/类型枚举 |
| **EquipmentDataAsset** | 完成 | `EquipmentSystem/EquipmentDataAsset.h/cpp` | 装备数据资产 |
| **EquipmentBase** | 完成 | `EquipmentSystem/EquipmentBase.h/cpp` | 装备Actor基类 |

**装备品质枚举**:
- Common(普通/白) / Uncommon(优秀/蓝) / Rare(稀有/金) / Epic(史诗/暗金) / Legendary(传说/神话)

**装备槽位** (7个槽位):
- MainHand(主手) / OffHand(副手) / Helmet(头盔) / Chest(胸甲) / Legs(护腿) / Accessory1(饰品1) / Accessory2(饰品2)

### 5.2 装备属性与套装系统

**装备基础属性**:
- Attack(攻击力) / Defense(防御力) / MaxHealth(最大生命值)
- CriticalChance(暴击率) / CriticalDamage(暴击伤害)
- MovementSpeed(移动速度) / GoldFind(金币获取率)

**与AttributeSystem集成**:
- 装备属性使用附加值(Additive)方式应用
- 穿戴时调用 ApplyEquipmentStats
- 卸下时调用 RemoveEquipmentStats

### 5.3 装备槽位与背包系统

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **EquipmentManagerComponent** | 完成 | `EquipmentSystem/EquipmentManagerComponent.h/cpp` | 装备槽位管理 |
| **InventoryComponent** | 完成 | `EquipmentSystem/InventoryComponent.h/cpp` | 背包管理 |

**背包系统功能**:
- 固定容量背包（默认20格）
- 装备不可叠加，每个装备实例独立占用一格
- 支持按品质/类型排序

### 5.4 装备耐久与经济系统

- 耐久度消耗与修理
- 分解系统
- 装备生成工厂
- 与掉落系统衔接

---

## Phase 6 - 关卡与流程 (待开发)

**目标**: 游戏关卡逻辑

---

## Phase 7 - 经济与成长系统 (待开发)

**目标**: 角色成长与资源管理

---

## Phase 8 - UI 与体验 (待开发)

**目标**: 游戏界面与引导

---

## Phase 9 - 美术表现与特效 (待开发)

**目标**: 视觉表现

---

## Phase 10 - 平衡与配置 (待开发)

**目标**: 数值平衡与调试

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

---

## 核心系统文件位置

```
Source/niuniu555/
├── SaveSystem/
├── InputSystem/
├── GameFlow/
├── ObjectPool/
├── AttributeSystem/
├── CharacterSystem/
├── WeaponSystem/
├── CombatSystem/
├── EnemySystem/
├── AI/
└── EquipmentSystem/
```

---

## 技术备注

- **编译命令**: `E:\UE_5.6\UE_5.6\Engine\Build\BatchFiles\Build.bat niuniu555Editor Win64 Development -Project="C:\Users\Win10\Desktop\niuniu555\niuniu555.uproject" -Progress`
- **UE 版本**: 5.6
- **编译器**: Visual Studio 2022
- **目标平台**: Win64

---

## 如何恢复开发进度

**方式 1：提供项目路径**
告诉 AI：
> "继续开发我的UE5项目，路径是 `C:\Users\Win10\Desktop\niuniu555`，请先读取 AGENTS.md 文件了解当前状态。"

**方式 2：上传 AGENTS.md**
直接上传本文件内容，AI 会自动了解项目状态。

**方式 3：简要说明**
> "继续开发财神末法冒险项目，目前 Phase 5 已完成（装备与背包系统），准备进入 Phase 6 关卡系统开发。"

---

## Bug 修复记录

### 2026-03-09 - Phase 5 装备系统完成
- 装备数据架构（品质/槽位/属性）
- 与AttributeSystem属性集成
- 装备槽位管理（7槽位）与背包系统
- 套装效果计算框架
- 耐久度与修理/分解系统
- 装备生成工厂与掉落系统衔接

### 2026-03-09 - Phase 4.4 掉落系统Bug修复
- 修复敌人死亡后特效不消失
- 修复金币/装备掉落不生成Actor
- 修复神格碎片日志重复
- 修复掉落物模型不显示
- 修复武器攻击特效不销毁
- 修复AI攻击动画卡住

### 2026-03-08 - AI行为树系统完成
- 完成巡逻/追击/攻击任务节点
- 完成距离检查装饰器与目标更新服务
- 修复攻击与追击任务震荡问题

### 2026-03-07 - 敌人受击反馈修复
- 修复血量初始化失败
- 修复蓝图覆盖接口函数问题
- 修复攻击任务与动画不同步

### 2026-03-06 - 武器系统修复
- 修复BlueprintNativeEvent调用方式
- 修复武器切换计时器错误

---

## 开发规范速查

### 接口调用黄金法则
```cpp
// 错误：直接调用
CurrentWeapon->CanAttack();

// 正确：使用Execute_前缀
IWeaponInterface::Execute_CanAttack(CurrentWeapon);
```

### 防御性编程检查点
- 血量初始化：在BeginPlay中强制检查CurrentHealth > 0
- 空指针检查：使用ensure或check宏
- 接口检测：使用ImplementsInterface而非Cast

### Git提交规范
- 提交前检查.vs/、Binaries/、Intermediate/是否已排除
- 大文件（>100MB）必须使用Git LFS
- 阶段性打标签：git tag -a "v0.5-equipment" -m "Phase 5完成"

---

*最后更新: 2026-03-09*  
*开发阶段: Phase 5 装备与背包系统 完成*  
*下一步: Phase 6 关卡与流程系统*
