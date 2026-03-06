# 武器系统测试指南 (WeaponSystem Test Guide) - 更新版

## 📋 功能概述

武器系统架构已完成，包含以下核心组件：

| 组件 | 文件路径 | 功能说明 |
|------|----------|----------|
| 武器类型定义 | `WeaponSystem/WeaponTypes.h` | 武器枚举、伤害结构体、连招信息 |
| 武器接口 | `WeaponSystem/WeaponInterface.h` | IWeaponInterface 接口定义 |
| 武器基类 | `WeaponSystem/WeaponBase.h/cpp` | AWeaponBase 武器基类 |
| 武器数据资产 | `WeaponSystem/WeaponDataAsset.h/cpp` | UWeaponDataAsset 蓝图配置数据 |
| 武器管理器 | `WeaponSystem/WeaponManagerComponent.h/cpp` | UWeaponManagerComponent 管理组件 |

**重要技术要点：**
- 所有 `BlueprintNativeEvent` 接口函数必须使用 `IWeaponInterface::Execute_` 前缀调用
- 数据与逻辑分离：武器属性通过 `UWeaponDataAsset` 在蓝图中配置

---

## 🎯 测试步骤

### 步骤1：创建武器蓝图

1. **打开内容浏览器** (`Content Browser`)
   - 路径：`Content/`
   - 右键 → 新建文件夹 → 命名为 `Weapons`（武器文件夹）

2. **创建武器蓝图类**
   - 在 `Weapons` 文件夹中右键
   - 选择 `Blueprint Class`（蓝图类）
   - 在 `All Classes`（所有类）搜索框中输入 `WeaponBase`
   - 选择 `AWeaponBase` 作为父类
   - 命名为 `BP_TestSword`（测试剑）

3. **配置武器基础属性**
   - 双击打开 `BP_TestSword`
   - 在 `Details`（细节）面板中找到 `武器配置|基础信息` 部分：
     - `Weapon Type`（武器类型）: 选择 `轩辕剑-近战` (XuanyuanSword)
     - `Weapon Name`（武器名称）: 输入 `"测试轩辕剑"`
     - `Weapon Description`（武器描述）: 输入 `"一把测试用的轩辕剑"`
   
   - 在 `武器配置|属性` 部分配置基础属性：
     - `Base Stats` → `Base Attack`（基础攻击）: `20.0`
     - `Base Stats` → `Attack Speed`（攻击速度）: `1.0`
     - `Base Stats` → `Attack Range`（攻击范围）: `150.0`
     - `Base Stats` → `Cooldown Time`（冷却时间）: `0.5`

4. **配置连招阶段**（轻击连招）
   - 找到 `武器配置|连招|轻击` 部分：
   - `Light Combo Stages`（轻击连招阶段）：点击 `+` 添加3个元素
   - 每个元素配置：
     - `Stage Index`（阶段索引）: 0, 1, 2
     - `Stage Name`（阶段名称）: `Attack1`, `Attack2`, `Attack3`
     - `Damage Multiplier`（伤害倍率）: 1.0, 1.2, 1.5
     - `b Can Cancel`（能否取消）: 勾选
     - `Cancel Window Start`（取消窗口开始）: `0.3`
     - `Cancel Window End`（取消窗口结束）: `0.5`

5. **编译并保存**
   - 点击 `Compile`（编译）按钮
   - 点击 `Save`（保存）保存蓝图

---

### 步骤2：创建武器数据资产（可选但推荐）

1. **创建数据资产**
   - 在 `Content/Weapons` 文件夹中右键
   - 选择 `Miscellaneous`（杂项）→ `Data Asset`（数据资产）
   - 在 `All Classes` 中搜索 `WeaponDataAsset`
   - 选择 `UWeaponDataAsset`
   - 命名为 `DA_TestSwordData`（测试剑数据）

2. **配置武器数据**
   - 双击打开 `DA_TestSwordData`
   - 配置以下关键属性：

   **基础信息 (WeaponBasic)**：
   - `Weapon ID`: `TestSword`
   - `Weapon Type`: `XuanyuanSword`（轩辕剑）
   - `Weapon Name`: `测试轩辕剑`
   - `Weapon Description`: `一把测试用的轩辕剑`
   - `Rarity`（稀有度）: `Common`（普通-白色）

   **属性 (WeaponStats)**：
   - `Base Attack`: `20.0`
   - `Attack Speed`: `1.0`
   - `Critical Rate`: `0.05`（5%暴击率）
   - `Critical Damage`: `1.5`（暴击1.5倍伤害）
   - `Attack Range`: `150.0`（攻击范围150厘米）
   - `Stamina Cost`: `10.0`（每次攻击消耗10点耐力）
   - `Cooldown Time`: `0.5`（冷却0.5秒）

3. **保存数据资产**
   - 点击 `Save` 保存

---

### 步骤3：给角色添加武器管理器

1. **打开角色蓝图**
   - 找到你的玩家角色蓝图（例如 `BP_CombatCharacter`）
   - 双击打开

2. **添加武器管理器组件**
   - 在 `Components`（组件）面板中点击 `Add Component`（添加组件）
   - 搜索 `Weapon Manager`（武器管理器）
   - 选择 `Weapon Manager Component`
   - 命名为 `WeaponManager`

3. **配置武器管理器参数**
   - 选中 `WeaponManager` 组件
   - 在 `Details`（细节）面板中找到 `武器管理器|配置`：
     - `Default Weapon Type`（默认武器类型）: 选择 `None`（我们先手动注册）
     - `b Auto Equip Default Weapon`（自动装备默认武器）: **取消勾选**
     - `Weapon Switch Time`（武器切换时间）: `0.5`

---

### 步骤4：在角色蓝图中注册武器（关键步骤）

1. **打开 Event Graph**（事件图表）

2. **添加注册武器逻辑**
   - 找到 `Event BeginPlay`（事件开始游戏）节点
   - 按下图连接节点：

```
[Event BeginPlay]
       |
       v
[Get Weapon Manager] --(Target)--> [Register Weapon]（注册武器）
                                          |
                                   [Weapon Type]: XuanyuanSword
                                   [Weapon Class]: BP_TestSword
                                   [Weapon Data]: DA_TestSwordData（可选）
                                          |
                                          v
                                   [Equip Weapon]（装备武器）
                                          |
                                   [Weapon Type]: XuanyuanSword
```

3. **详细节点配置说明**：

   | 节点 | 中文显示名 | 配置值 |
   |------|-----------|--------|
   | `Get Weapon Manager` | 获取武器管理器 | 拖拽 `WeaponManager` 组件到图表 |
   | `Register Weapon` | 注册武器 | 从 `WeaponManager` 引脚拖出搜索 |
   | `Weapon Type` | 武器类型 | 选择 `XuanyuanSword`（轩辕剑）|
   | `Weapon Class` | 武器类 | 选择 `BP_TestSword`（你的武器蓝图）|
   | `Weapon Data` | 武器数据 | 选择 `DA_TestSwordData`（可选）|
   | `Equip Weapon` | 装备武器 | 从 `WeaponManager` 引脚拖出搜索 |

4. **连接执行引脚**：
   - `BeginPlay` → `Register Weapon` → `Equip Weapon`

5. **编译并保存**
   - 点击 `Compile`（编译）
   - 点击 `Save`（保存）

---

### 步骤5：添加攻击输入（测试攻击功能）

1. **在角色蓝图中添加输入事件**
   - 在 `Event Graph` 中右键搜索 `Input`（输入）
   - 添加 `Left Mouse Button`（鼠标左键）事件

2. **连接轻击逻辑**

```
[Left Mouse Button]（鼠标左键按下）
       |
       v
[Get Weapon Manager] --(Target)--> [Perform Light Attack]（执行轻击）
       |
       v
[Branch]（分支）检查返回值
       |
   [True] -> [Print String]: "轻击成功"
   [False] -> [Print String]: "轻击失败 - 可能正在冷却或无法攻击"
```

3. **添加重击输入**（可选）
   - 添加 `Right Mouse Button`（鼠标右键）事件
   - 连接 `Perform Heavy Attack`（执行重击）节点

---

### 步骤6：运行测试

1. **点击 Play**（播放）按钮运行游戏

2. **观察输出日志**（Window → Developer Tools → Output Log）

   **预期输出日志**：
   ```
   [WeaponManager] 开始切换武器 - 从 None 到 XuanyuanSword, 切换时间: 0.50秒
   [WeaponManager] 武器切换执行 - 新武器: 测试轩辕剑
   [WeaponManager] 武器切换完成 - 当前武器: XuanyuanSword
   ```

3. **测试攻击功能**
   - 按下鼠标左键
   - 查看输出日志：
   ```
   [BP_TestSword] 开始轻击 - 阶段: 0, Section: Attack1
   [WeaponManager] 执行轻击 - 阶段: 0
   ```

4. **测试连招**
   - 快速连续按下鼠标左键
   - 观察连招阶段的变化：
   ```
   [BP_TestSword] 开始轻击 - 阶段: 0
   [BP_TestSword] 开始轻击 - 阶段: 1
   [BP_TestSword] 开始轻击 - 阶段: 2
   ```

---

## 🔍 调试检查点

### 检查武器是否正确装备

在蓝图中使用以下节点检查：

```
[键盘 1 按下]
       |
       v
[Get Weapon Manager] -> [Get Current Weapon]（获取当前武器）
                               |
                               v
                       [Is Valid]（是否有效）-> [Branch]（分支）
                               |
                   [True]  -> [Print String]: "武器已装备"
                   [False] -> [Print String]: "武器未装备"
```

### 检查武器状态

```
[Get Weapon Manager] -> [Get Current Weapon]
                               |
                               v
                       [Get Weapon State]（获取武器状态）
                               |
                               v
                       [Print String]: 显示当前状态
```

武器状态枚举：
- `Idle`（空闲）
- `Attacking`（攻击中）
- `Charging`（蓄力中）
- `Cooldown`（冷却中）
- `Switching`（切换中）

### 检查是否可以攻击

```
[Get Weapon Manager] -> [Can Attack]（能否攻击）
                               |
                               v
                       [Branch]（分支）
                           |
               [True] -> [Print String]: "可以攻击"
               [False] -> [Print String]: "不能攻击"
```

---

## ⚠️ 常见问题及解决方案

### 问题1：崩溃提示 "Call Execute_XXX instead"

**原因**：直接调用了 `BlueprintNativeEvent` 接口函数，没有使用 `Execute_` 前缀

**解决方案**：
- 这是代码层面的问题，已在 `WeaponManagerComponent` 和 `WeaponBase` 中修复
- 如果你在自己的蓝图中调用武器函数，请确保通过正确的接口调用

### 问题2：武器没有显示在角色手上

**检查点**：
1. 角色骨骼中是否有 `hand_r` 插槽 (Socket)
2. 武器蓝图中 `Weapon Mesh` 是否正确设置了骨骼网格体
3. `OnEquipped` 是否成功执行

**解决方案**：
- 在角色骨骼编辑器中添加 `hand_r` Socket
- 确保武器网格体已正确指定

### 问题3：攻击没有反应

**检查点**：
1. 输出日志中是否有 "轻击失败" 或 "无法攻击"
2. `CanAttack` 返回 `false`
3. 武器状态是否正确

**解决方案**：
- 检查武器当前状态是否为 `Idle` 或 `Attacking` (Cancel Window内)
- 检查 `CooldownTimer` 是否已完成

### 问题4：攻击检测不到敌人

**检查点**：
1. `Attack Trace Channel` 设置是否正确
2. 敌人碰撞设置是否响应该通道
3. `Attack Range` 是否足够

**解决方案**：
- 将 `Attack Trace Channel` 设置为 `Pawn`
- 确保敌人的碰撞设置中 `Pawn` 通道为 `Block`
- 增大 `Attack Range`

### 问题5：未找到类型为 XXX 的注册武器

**原因**：武器没有在 `WeaponManager` 中注册

**解决方案**：
- 确保在 `BeginPlay` 中调用了 `Register Weapon`
- 确保 `Weapon Type` 参数与装备时一致

---

## 📁 文件位置速查

| 功能 | 头文件 | 实现文件 |
|------|--------|----------|
| 武器类型定义 | `Source/niuniu555/WeaponSystem/WeaponTypes.h` | - |
| 武器接口 | `Source/niuniu555/WeaponSystem/WeaponInterface.h` | `WeaponInterface.cpp` |
| 武器基类 | `Source/niuniu555/WeaponSystem/WeaponBase.h` | `WeaponBase.cpp` |
| 武器数据资产 | `Source/niuniu555/WeaponSystem/WeaponDataAsset.h` | `WeaponDataAsset.cpp` |
| 武器管理器 | `Source/niuniu555/WeaponSystem/WeaponManagerComponent.h` | `WeaponManagerComponent.cpp` |

---

## 🎓 技术要点总结

### UE5 BlueprintNativeEvent 调用规则

| 函数类型 | 调用方式 | 示例 |
|---------|----------|------|
| `BlueprintNativeEvent` 接口函数 | 必须 `IWeaponInterface::Execute_函数名(对象, 参数)` | `IWeaponInterface::Execute_CanAttack(this)` |
| 普通 C++ 函数 | `对象->函数名()` 或 `this->函数名()` | `CanCancelAttack()` |

**注意**：即使在类内部调用自己的 `BlueprintNativeEvent` 函数，也必须使用 `Execute_` 前缀！

---

## 🎮 下一步开发

当前已完成武器系统的基础架构，下一步可以：

1. **创建具体武器实现** - 实现轩辕剑、盘古斧、财神金杖、射日弓四种武器
2. **集成连招系统** - 与现有的 `ComboInputSystem` 联动
3. **实现伤害计算** - 完整的伤害公式和抗性系统
4. **添加准星与锁定系统**

---

*测试指南版本: 2.0*  
*最后更新: 2026-03-06*  
*对应代码版本: WeaponSystem 完整修复版*
