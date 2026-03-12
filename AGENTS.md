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
| Phase 6 | 关卡与流程 | ✅ 完成 | 房间/节点系统、地图生成、节点交互 |
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

## Phase 6 - 关卡与流程 (基础架构完成)

### 6.1 房间系统基础架构 (已完成)

| 组件 | 状态 | 主要文件 | 功能要点 |
|------|------|----------|----------|
| **LevelTypes** | 完成 | `LevelSystem/LevelTypes.h` | 房间类型/状态枚举，结构体定义 |
| **RoomBase** | 完成 | `LevelSystem/RoomBase.h/cpp` | 房间基类，生命周期管理，事件委托 |
| **RoomManager** | 完成 | `LevelSystem/RoomManager.h/cpp` | 房间注册/切换/生成，全局子系统 |
| **LevelGenerator** | 完成 | `LevelSystem/LevelGenerator.h/cpp` | 随机地图生成，层级结构，连接关系 |
| **LevelTransitionManager** | 完成 | `LevelSystem/LevelTransitionManager.h/cpp` | 关卡加载，过渡效果，数据保持 |

**房间类型**:
| 类型 | 说明 | 颜色 |
|------|------|------|
| Start | 起点 | 绿色 |
| Combat | 战斗房 | 红色 |
| Event | 事件房 | 橙色 |
| Shop | 商店房 | 黄色 |
| Skill | 技能房 | 紫色 |
| Boss | BOSS房 | 黑色 |

**房间状态**:
- NotEntered(未进入) / InProgress(进行中) / Completed(已完成) / Skipped(已跳过) / Locked(已锁定)

### 6.2 战斗房间 (已完成)

**CombatRoom**: 波次管理，敌人生成，目标系统，奖励发放

**战斗房间类型**:
- Normal(普通) / Elite(精英) / Boss(BOSS)

**房间目标类型**:
- KillAll(击杀所有) / KillCount(击杀指定数量) / SurviveTime(存活时间) / DefendTarget(保护目标) / KillBoss(击杀BOSS)

### 6.3 事件房间 (已完成)

**EventRoom**: 随机事件，选项选择，奖励/惩罚，跳过机制

**事件类型**:
- Blessing(神谕祝福) / Curse(诅咒) / Treasure(宝藏) / Shop(神秘商人) / Gamble(赌博)

### 6.4 商店房间 (已完成)

**ShopRoom**: 商品生成，购买/出售，刷新机制，折扣系统

**功能**:
- 预设商品列表或随机生成
- 金币/神格碎片购买
- 商品刷新（价格递增）
- 触发区域自动打开商店

### 6.5 技能房间 (已完成)

**SkillRoom**: 技能升级、技能重置、学习新技能

**功能**:
- 升级已有技能（消耗神格碎片）
- 重置所有技能（返还碎片，价格递增）
- 学习新技能
- 与Phase 7技能树系统预留接口

### 6.6 章节配置 (已完成)

**ChapterConfigDataAsset**: 数据资产化章节参数

**配置内容**:
- 章节基本信息（ID/名称/描述）
- 地图生成配置（层数/房间数/分支概率）
- 房间类型概率（商店/事件/技能/精英）
- 战斗配置（难度系数/BOSS配置）
- 奖励配置（金币/经验/装备掉率）
- 环境配置（音乐/特效/光照）

### 6.7 调试命令 (已完成)

**LevelSystemCheats**: 控制台作弊命令

**可用命令**:
- `Level.GenerateMap [Seed] [Layers]` - 生成地图
- `Level.EnterRoom [RoomID]` - 进入房间
- `Level.ShowMap` - 显示地图信息
- `Level.GiveGold [Amount]` - 给予金币
- `Level.GiveFragments [Amount]` - 给予神格碎片
- `Level.ShowEventInfo` - 显示事件信息
- `Level.ChooseOption [Index]` - 选择事件选项
- `Level.SkipEvent` - 跳过事件
- `Level.ShowShopInfo` - 显示商店信息
- `Level.BuyItem [Index]` - 购买商品
- `Level.RefreshShop` - 刷新商店
- `Level.FinishShopping` - 完成购物
- `Level.ResetRoom` - 重置当前房间
- `Level.CompleteRoom` - 完成当前房间
- `Level.SkipRoom` - 跳过当前房间
- `Level.GiveGold [Amount]` - 给予金币
- `Level.GiveFragments [Amount]` - 给予神格碎片

### 6.8 崩溃修复记录 (2026-03-09)

**问题**: 引擎启动时崩溃 `EXCEPTION_ACCESS_VIOLATION at 0x000000000000002c`

**根本原因**: 初始化时序竞争条件
1. RoomManager在Initialize时访问未完全初始化的World/GameInstance
2. RoomBase在BeginPlay立即访问RoomManager子系统
3. CombatRoom在BeginPlay立即查找场景中的EnemySpawner

**修复措施**:

| 文件 | 修复内容 |
|------|----------|
| RoomManager.cpp | 添加`bInitialized`标志防止重复初始化；添加GetWorld()/GetGameInstance()空检查；延迟0.1秒创建LevelGenerator |
| RoomBase.cpp | 延迟0.2秒后注册到RoomManager；使用`RoomManagerPtr`缓存管理器引用；EndPlay中使用缓存指针注销 |
| CombatRoom.cpp | 延迟1.0秒后查找EnemySpawner；添加`IsValid(World)`检查；提供`RegisterEnemySpawner`手动注册接口 |
| RoomManager.h/cpp | 所有UObject*访问前添加`IsValid()`检查；数组遍历过滤无效对象；添加边界检查 |

**防御性编程原则**:
1. **延迟初始化**: 子系统/场景Actor在BeginPlay后0.1-1.0秒再访问
2. **空指针检查**: 所有GetWorld()/GetGameInstance()/GetSubsystem()调用后检查nullptr
3. **有效性验证**: UObject指针使用`IsValid()`而非`!= nullptr`（检测GC回收状态）
4. **缓存引用**: 频繁访问的管理器指针在有效时缓存，避免重复查找

**编译状态**: ✅ 成功 (2026-03-09 23:50)

**变量名统一**: 将`RoomManager`统一改为`RoomManagerPtr`，避免与成员函数名混淆

**第一次修复文件变更**:
- `RoomBase.h`: `URoomManager* RoomManager` → `URoomManager* RoomManagerPtr = nullptr`
- `RoomBase.cpp`: BeginPlay中缓存RoomManagerPtr，EndPlay中使用缓存指针注销
- `RoomManager.cpp`: RegisterRoom和UnregisterRoom中使用RoomManagerPtr

**第二次崩溃修复** (2026-03-09 23:50):

**问题**: 新的崩溃 `EXCEPTION_ACCESS_VIOLATION at 0x00007f000000000c`，发生在后台线程

**根本原因**: Lambda定时器回调中捕获原始`this`指针，如果对象在定时器触发前被销毁，访问`this`会导致崩溃

**修复方案 - 使用弱指针**：
```cpp
// 错误 - 不安全
GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this](){
    if (IsValid(this)) { ... }  // this可能已经是野指针
}, 0.1f, false);

// 正确 - 使用弱指针
TWeakObjectPtr<URoomManager> WeakThis(this);
GetWorld()->GetTimerManager().SetTimer(DelayHandle, [WeakThis](){
    if (WeakThis.IsValid()) { ... }  // 安全地检查对象是否仍然存在
}, 0.1f, false);
```

**修复文件**:
- `RoomManager.cpp`: Lambda使用`TWeakObjectPtr<URoomManager>`捕获
- `RoomBase.cpp`: Lambda使用`TWeakObjectPtr<ARoomBase>`捕获

**作弊命令修复**:

**问题**: `Level.ShowMap`命令显示`Command not recognized`

**根本原因**: `ULevelSystemCheats`继承自`UCheatManager`，需要在PlayerController中设置`CheatClass`才能生效

**解决方案**: 添加全局控制台命令（`FAutoConsoleCommandWithWorldAndArgs`），不依赖CheatClass设置

**新增全局命令**:
| 命令 | 功能 |
|------|------|
| `Level.GenerateMap [Seed] [Layers]` | 生成随机地图 |
| `Level.ShowMap` | 显示当前地图信息 |
| `Level.EnterRoom [RoomID]` | 进入指定房间 |
| `Level.CompleteRoom` | 完成当前房间 |
| `Level.ResetRoom` | 重置当前房间 |
| `Level.GiveGold [Amount]` | 给予金币 |
| `Level.GiveFragments [Amount]` | 给予神格碎片 |
| `Level.ShowEventInfo` | 显示当前事件信息 |
| `Level.ChooseOption [Index]` | 选择事件选项 |
| `Level.SkipEvent` | 跳过当前事件 |
| `Level.ShowShopInfo` | 显示商店商品列表 |
| `Level.BuyItem [Index]` | 购买商品 |
| `Level.RefreshShop` | 刷新商店商品 |
| `Level.FinishShopping` | 完成购物离开商店 |

**第三次崩溃修复** (2026-03-09 23:58):

**问题**: `Array has changed during ranged-for iteration!` - 数组遍历时被修改

**根本原因**: `ClearAllRooms()` 中遍历 `RegisteredRooms` 时调用 `Room->Destroy()`，触发 `EndPlay` → `UnregisterRoom()`，后者从数组中移除元素，导致遍历出错

**修复方案 - 数组复制**：
```cpp
// 修复前 - 遍历时修改原数组
for (ARoomBase* Room : RegisteredRooms)
{
    Room->Destroy();  // 触发 UnregisterRoom，修改 RegisteredRooms
}

// 修复后 - 遍历副本
TArray<ARoomBase*> RoomsToDestroy = RegisteredRooms;  // 复制
for (ARoomBase* Room : RoomsToDestroy)
{
    if (IsValid(Room)) Room->Destroy();
}
```

**修复文件**: `RoomManager.cpp` - `ClearAllRooms()` 函数

**第四次崩溃修复** (2026-03-10 01:14):

**问题**: `EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000010`，发生在后台线程

**根本原因**: 
1. `CombatRoom` 中的定时器回调虽然使用了弱指针，但在回调内部对 `GroupCopy.EnemyClass` 等成员的访问仍可能出现问题
2. `TArray<FTimerHandle>` 被错误地标记为 `UPROPERTY()`（FTimerHandle不是UObject）
3. 波次延迟定时器句柄未保存，无法在EndPlay中清理
4. 定时器回调中缺乏足够的有效性检查

**修复方案**：
1. 移除 `SpawnTimerHandles` 的 `UPROPERTY()` 标记
2. 添加 `WaveDelayTimerHandle` 成员变量保存波次延迟定时器
3. 在 `EndPlay()` 中清理所有定时器（包括波次延迟定时器）
4. 强化定时器回调的有效性检查（双重检查弱指针和原始指针）
5. 在定时器回调中使用局部变量缓存获取的对象指针，避免重复解引用

**修复文件**:
- `CombatRoom.h`: 移除 `UPROPERTY()` 从 `SpawnTimerHandles`，添加 `WaveDelayTimerHandle`
- `CombatRoom.cpp`: 强化所有定时器回调的安全性，完善 `EndPlay()` 清理逻辑

**关键代码变更**:
```cpp
// 定时器回调安全性增强
GetWorldTimerManager().SetTimer(SpawnHandle, [WeakThis, GroupCopy, LocationCopy]()
{
    // 双重检查
    if (!WeakThis.IsValid()) return;
    
    ACombatRoom* Room = WeakThis.Get();
    if (!IsValid(Room)) return;
    
    UWorld* World = Room->GetWorld();
    if (!IsValid(World)) return;
    
    // 使用局部变量，避免重复解引用
    AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(...);
    if (IsValid(Enemy) && IsValid(Room))
    {
        Room->RegisterSpawnedEnemy(Enemy);
    }
}, DelayTime, false);
```

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
├── EquipmentSystem/
└── LevelSystem/
    ├── LevelTypes.h
    ├── RoomBase.h/cpp
    ├── RoomManager.h/cpp
    ├── CombatRoom.h/cpp
    ├── EventRoom.h/cpp
    ├── ShopRoom.h/cpp
    ├── SkillRoom.h/cpp
    ├── LevelGenerator.h/cpp
    ├── LevelTransitionManager.h/cpp
    ├── ChapterConfigDataAsset.h/cpp
    └── LevelSystemCheats.h/cpp
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

### 2026-03-09 - Phase 6 关卡系统Bug修复

**修复问题清单**:

1. **控制台作弊命令无法识别**
   - 原因: Exec命令格式不匹配，需要PlayerController配置CheatClass
   - 修复: 添加蓝图可调用版本(BP_xxx)作为替代方案
   - 文件: LevelSystemCheats.h/cpp

2. **CombatRoom无法找到敌人生成器**
   - 原因: BeginPlay中立即查找，此时生成器可能未初始化
   - 修复: 添加0.5秒延迟查找 + 手动注册接口(RegisterEnemySpawner)
   - 文件: CombatRoom.h/cpp

3. **RoomManager多次初始化**
   - 修复: 添加bInitialized保护标志
   - 文件: RoomManager.h/cpp

4. **FMapNode成员未初始化警告**
   - 修复: MapPosition添加默认值 FVector2D::ZeroVector
   - 文件: LevelTypes.h

5. **SkillRoom前置声明错误**
   - 修复: 移除UPlayerSkillBase类定义，改为UObject类型
   - 文件: SkillRoom.h/cpp

6. **RoomManager OpenLevel逻辑问题**
   - 原因: OpenLevel会销毁当前世界，后续代码不会执行
   - 修复: 添加警告说明，暂时禁用OpenLevel逻辑
   - 文件: RoomManager.cpp

7. **LevelGenerator概率计算错误**
   - 修复: 重写概率分配逻辑，确保商店房优先且概率计算正确
   - 文件: LevelGenerator.cpp

8. **CombatRoom波次为空警告**
   - 修复: 在OnEnterRoom中添加警告日志
   - 文件: CombatRoom.cpp

### 2026-03-09 - Phase 6 关卡与流程系统完成
- **CombatRoom完善**: BOSS生成、敌人生成组、死亡处理、奖励发放
- **EventRoom完善**: 祝福/诅咒/宝藏/商店/赌博事件、奖励惩罚逻辑、Buff框架
- **ShopRoom完善**: 商品交易、金币系统框架、刷新机制
- **SkillRoom创建**: 技能升级/重置/学习、神格碎片系统接口
- **ChapterConfigDataAsset**: 章节配置数据资产化
- **LevelSystemCheats**: 调试控制台命令
- **编译修复**: 解决委托签名不匹配、变量名冲突等问题

### 2026-03-09 - Phase 6 关卡与流程系统基础架构完成
- 房间系统基础架构（RoomBase/RoomManager/LevelGenerator/LevelTransitionManager）
- 战斗房间（CombatRoom）- 波次管理、敌人生成、目标系统、奖励
- 事件房间（EventRoom）- 随机事件、选项选择、奖励/惩罚
- 商店房间（ShopRoom）- 商品生成、购买/出售、刷新机制
- 关卡类型定义（LevelTypes）- 统一枚举和结构体
- 房间类型：Start/Combat/Event/Shop/Skill/Boss
- 房间状态：NotEntered/InProgress/Completed/Skipped/Locked

**技术要点**:
- 使用独特命名的事件委托避免重复定义冲突
- RoomManager作为GameInstanceSubsystem全局管理房间
- LevelTransitionManager处理关卡加载和过渡效果
- 跨房间数据保持（血量/金币/神格碎片）

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

## Phase 6 - 关卡与流程系统修复记录 (2026-03-10)

### 修复1: LevelTransitionManager Lambda定时器移除

**问题**: `PlayTransition()` 函数仍使用Lambda定时器，存在崩溃风险

**修复**: 改为成员函数绑定
```cpp
// 修复前（有风险）
FTimerHandle FadeTimer;
TWeakObjectPtr<ULevelTransitionManager> WeakThis(this);
GetWorld()->GetTimerManager().SetTimer(FadeTimer, [WeakThis](){
    if (WeakThis.IsValid()) { WeakThis->OnTransitionFinished.Broadcast(); }
}, Duration, false);

// 修复后（安全）
void ExecuteFadeComplete();  // 成员函数
GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &ULevelTransitionManager::ExecuteFadeComplete, Duration, false);
```

**涉及文件**: `LevelTransitionManager.h/cpp`

---

### 修复2: CombatRoom敌人死亡事件绑定

**问题**: `RegisterSpawnedEnemy()` 中注释说绑定死亡事件，但实际未实现，导致敌人死亡时CombatRoom无法得知

**修复**: 
1. 添加 `OnEnemyDeathCallback` 回调函数
2. 在 `RegisterSpawnedEnemy()` 中绑定 `OnDeath` 委托
3. 在 `EndPlay()` 中解绑所有死亡事件
4. 添加 `BoundEnemies` 数组跟踪已绑定敌人

**涉及文件**: `CombatRoom.h/cpp`

---

### 修复3: CombatRoom波次完成判定逻辑

**问题**: `AreAllWavesComplete()` 边界条件考虑不周
- `TotalWaves = 0` 时，`TotalWaves - 1 = -1`，条件判断错误
- BOSS房没有特殊处理

**修复**:
```cpp
bool ACombatRoom::AreAllWavesComplete() const {
    // BOSS房特殊处理
    if (CombatType == ECombatRoomType::Boss) {
        return !bWaveInProgress && AliveEnemyCount == 0;
    }
    // 没有配置波次，视为完成
    if (TotalWaves == 0) return true;
    // 最后一波且不在进行中且无存活敌人
    return CurrentWave >= TotalWaves - 1 && !bWaveInProgress && AliveEnemyCount == 0;
}
```

**涉及文件**: `CombatRoom.cpp`

---

### 修复4: ActiveWaveConfigs UPROPERTY问题

**问题**: `TArray<FCombatWaveConfig> ActiveWaveConfigs` 被错误标记为UPROPERTY，但结构体包含非UObject成员

**修复**: 移除UPROPERTY标记
```cpp
// 修复前
UPROPERTY()
TArray<FCombatWaveConfig> ActiveWaveConfigs;

// 修复后
TArray<FCombatWaveConfig> ActiveWaveConfigs;
```

**涉及文件**: `CombatRoom.h`

---

### 修复5: PlayerDataSubsystem创建

**问题**: EventRoom/ShopRoom/SkillRoom中玩家数据接口都是TODO占位

**修复**: 创建新的GameInstanceSubsystem统一管理玩家数据

**新增文件**: 
- `GameFlow/PlayerDataSubsystem.h/cpp`

**功能**:
- 金币管理（AddGold/SetGold/TryDeductGold）
- 神格碎片管理（AddFragments/SetFragments/TryDeductFragments）
- 生命值/法力值管理
- 跨房间数据保持（SavePlayerData/RestorePlayerData）
- 变化事件委托（OnGoldChanged/OnFragmentsChanged等）

---

### 修复6: 商店房完成机制

**问题**: ShopRoom没有提供完成房间的机制

**修复**: 添加 `FinishShopping()` 方法
```cpp
UFUNCTION(BlueprintCallable, Category = "交互")
void FinishShopping();  // 关闭商店并完成房间
```

**涉及文件**: `ShopRoom.h/cpp`

---

### 修复7: RoomManager章节配置支持

**问题**: ChapterConfigDataAsset已定义但完全未被使用

**修复**: 添加 `GenerateMapFromConfig()` 方法
```cpp
UFUNCTION(BlueprintCallable, Category = "地图生成")
void GenerateMapFromConfig(UChapterConfigDataAsset* Config, int32 Seed = 0);
```

**涉及文件**: `RoomManager.h/cpp`

---

### 修复8: 波次检查定时器频率优化

**问题**: 波次检查每秒一次，响应有延迟

**修复**: 改为0.3秒检查一次
```cpp
// 修复前
GetWorldTimerManager().SetTimer(WaveCheckTimerHandle, this, &ACombatRoom::CheckWaveComplete, 1.0f, true);

// 修复后
GetWorldTimerManager().SetTimer(WaveCheckTimerHandle, this, &ACombatRoom::CheckWaveComplete, 0.3f, true);
```

**涉及文件**: `CombatRoom.cpp`

---

### 修复9: 作弊命令集成PlayerDataSubsystem

**修复**: `Level.GiveGold` 和 `Level.GiveFragments` 命令实际修改PlayerDataSubsystem

**涉及文件**: `LevelSystemCheats.cpp`

---

### 编译状态

✅ **编译成功** (2026-03-10)

---

## Phase 6 - 关卡与流程系统开发进度 (2026-03-10更新)

### 6.1 房间系统基础架构 ✅ 完成
- [x] RoomBase - 房间基类，状态管理
- [x] RoomManager - 全局子系统，房间生命周期管理
- [x] LevelGenerator - 随机地图生成
- [x] LevelTransitionManager - 关卡过渡管理

### 6.2 战斗房间 (CombatRoom) ✅ 完成
- [x] 波次管理系统 - 多波次配置与切换
- [x] 敌人生成 - 基于配置生成敌人
- [x] 敌人死亡事件绑定 - 委托回调机制
- [x] 波次完成检测 - AliveEnemyCount正确更新
- [x] BOSS房支持 - 特殊BOSS配置
- [x] 房间奖励 - 金币/经验发放

**修复记录**:
- 修复 `UnregisterEnemy` 类型匹配问题（弱指针与原始指针比较）
- 修复敌人死亡事件委托绑定与解绑
- 添加 `Level.KillAllEnemies` 作弊命令
- 修复波次完成判定逻辑

### 6.3 事件房间 (EventRoom) ✅ 完成
- [x] 基础架构 - 事件配置与选项系统
- [x] 默认事件生成 - 祝福/诅咒/宝藏/商店/赌博
- [x] PlayerDataSubsystem集成 - 金币/碎片操作
- [x] 事件效果实现 - 实际应用奖励/惩罚
- [x] 蓝图事件处理 - Blessing/Curse等事件
- [x] 全局控制台命令 - Level.ShowEventInfo, Level.ChooseOption, Level.SkipEvent

### 6.4 商店房间 (ShopRoom) ✅ 完成
- [x] 基础架构 - 商品生成与交易
- [x] PlayerDataSubsystem集成 - 金币检查与扣除
- [x] FinishShopping功能 - 完成房间机制
- [x] 商品数据配置 - 实际物品关联
- [x] 购买物品发放 - Inventory系统集成
- [x] 全局控制台命令 - Level.ShowShopInfo, Level.BuyItem, Level.RefreshShop, Level.FinishShopping

### 6.5 技能房间 (SkillRoom) ✅ 完成
- [x] 基础架构 - 技能升级/重置/学习
- [x] PlayerDataSubsystem集成 - 神格碎片操作
- [x] 与技能树系统集成 - Phase 7预留接口

### 6.6 调试与稳定性 ✅ 完成
- [x] LevelSystemCheats - 控制台作弊命令
- [x] 崩溃修复 - Lambda定时器移除
- [x] 安全性检查 - IsValid()全面应用
- [x] 委托生命周期管理 - 敌人死亡事件
- [x] 全局控制台命令 - 所有房间类型的调试命令

### 6.7 全局控制台命令 ✅ 完成

**新增全局命令** (使用 `FAutoConsoleCommandWithWorldAndArgs`):

| 命令 | 功能 |
|------|------|
| `Level.GenerateMap [Seed] [Layers]` | 生成随机地图 |
| `Level.ShowMap` | 显示当前地图信息 |
| `Level.EnterRoom [RoomID]` | 进入指定房间 |
| `Level.CompleteRoom` | 完成当前房间 |
| `Level.ResetRoom` | 重置当前房间 |
| `Level.SkipRoom` | 跳过当前房间 |
| `Level.GiveGold [Amount]` | 给予金币 |
| `Level.GiveFragments [Amount]` | 给予神格碎片 |
| `Level.ShowPlayerData` | 显示玩家数据 |
| `Level.KillAllEnemies` | 击杀所有敌人 |
| `Level.ShowEventInfo` | 显示当前事件信息 |
| `Level.ChooseOption [Index]` | 选择事件选项 |
| `Level.SkipEvent` | 跳过当前事件 |
| `Level.ShowShopInfo` | 显示商店商品列表 |
| `Level.BuyItem [Index]` | 购买商品 |
| `Level.RefreshShop` | 刷新商店商品 |
| `Level.FinishShopping` | 完成购物离开商店 |
| `Level.UnlockAllRooms` | 解锁所有房间 |
| `Level.ResetChapter` | 重置章节 |

---

*最后更新: 2026-03-11*  
*开发阶段: Phase 6 关卡与流程系统测试通过*  
*下一步: Phase 7 经济与成长系统开发*
