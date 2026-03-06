# Combo System 测试指南

## 系统架构说明

连招系统由以下组件协同工作：

```
角色蓝图
├── WeaponManagerComponent      # 管理武器（装备/卸下/攻击）
├── ComboDetectorComponent      # 检测输入序列，触发连招事件
├── WeaponComboIntegration      # 连接武器和连招系统（从武器自动生成连招配置）
└── ComboWindowController       # 管理连招取消窗口（可选）
```

**工作原理：**
1. `WeaponBase` 从 `WeaponDataAsset` 加载配置到 `LightComboStages` 数组
2. `WeaponComboIntegration` 从武器的 `LightComboStages` 生成连招序列
3. 玩家输入 → `ComboDetectorComponent` 检测序列匹配
4. 匹配成功 → 触发 `OnComboStageAdvanced` 事件
5. `WeaponComboIntegration` 监听事件并调用 `WeaponManager` 执行攻击

---

## 测试前准备

### 1. 测试场景选择

**可以使用现有地图！** 以下地图任选一个即可：
- `ThirdPersonMap` - 第三人称示例地图
- `Combat_TestMap` - 战斗测试地图
- 或任意已有玩家起始点的地图

---

### 2. 创建武器数据资产 (WeaponDataAsset)

**步骤：**
1. 在内容浏览器中右键 → 创建基础资产 → 数据资产 (Data Asset)
2. 选择父类：`WeaponDataAsset`
3. 命名为 `DA_TestSword`
4. 双击打开配置

**关键配置项（连招相关）：**

```
【基础信息】
- WeaponID: TestSword
- WeaponType: Sword          ← 重要！必须选择正确的武器类型
- WeaponName: 测试剑
- Rarity: Common

【轻击连招配置】
- LightAttackMontage: 选择你的攻击动画蒙太奇（可选，没有也能测试）
- LightAttackSections: 添加3个元素（决定连招段数）
  - [0]: LightAttack1
  - [1]: LightAttack2
  - [2]: LightAttack3
- LightAttackDamageMultipliers: 添加3个元素
  - [0]: 1.0
  - [1]: 1.2
  - [2]: 1.5
- LightCancelWindowStart: 0.3
- LightCancelWindowEnd: 0.5

【重击连招配置】（可选）
- HeavyAttackSections: 添加2个元素
  - [0]: HeavyAttack1
  - [1]: HeavyAttack2
```

**重要说明：**
- `LightAttackSections` 数组的元素数量决定轻击连招的段数
- **`WeaponType` 字段非常重要**，武器切换的缓存机制依赖这个字段
- 连招检测器使用 `LightComboStages.Num()` 获取段数（从数据资产自动加载）

---

### 3. 创建武器蓝图

**步骤：**
1. 右键 → 创建基础资产 → 蓝图类
2. 选择父类：`WeaponBase`
3. 命名为 `BP_TestSword`
4. 双击打开

**关键配置（类默认值）：**
```
【武器配置|数据资产】
- WeaponDataAsset: DA_TestSword    ← 必须设置！
```

**可选配置：**
```
【组件】
- WeaponMesh: 设置武器模型（可选）
```

**编译并保存**

---

### 4. 修改玩家角色蓝图

给玩家角色添加连招系统组件。

**步骤：**
1. 打开你的玩家角色蓝图（如 `BP_ThirdPersonCharacter`）
2. 在组件面板中添加以下组件：

```
添加组件：
- WeaponManagerComponent
- ComboDetectorComponent  
- WeaponComboIntegration
```

**组件配置：**

**WeaponComboIntegration：**
```
- LightAttackComboID: LightAttackCombo    （默认）
- HeavyAttackComboID: HeavyAttackCombo    （默认）
- bAutoRegisterOnWeaponChange: true       （武器切换时自动重新注册连招）
```

**事件图表配置：**

在 `Event BeginPlay` 后添加：
```
BeginPlay
  │
  ├─→ Get Component by Class (WeaponManagerComponent)
  │   └─→ Promote to variable: WeaponManager
  │
  ├─→ Spawn Actor From Class (BP_TestSword)
  │   └─→ Return Value: Promote to variable "TestSword"
  │
  ├─→ WeaponManager → Equip Weapon (TestSword)
  │
  └─→ 完成！WeaponComboIntegration 会自动注册连招
```

---

### 5. 配置输入映射

打开 `项目设置` → `输入`：

**动作映射 (Action Mappings)：**
```
LightAttack: 鼠标左键
HeavyAttack: 鼠标右键
```

**在角色蓝图中处理输入：**
```
Input Action LightAttack (Pressed)
  │
  └─→ WeaponComboIntegration → Process Light Attack Input

Input Action HeavyAttack (Pressed)
  │
  └─→ WeaponComboIntegration → Process Heavy Attack Input
```

---

## 测试步骤

### 测试 1：基础连招触发

**目的：** 验证轻击连招能正常触发

**步骤：**
1. 运行游戏（Play in Editor）
2. 按下鼠标左键（轻击）

**期望结果：**
- 输出日志：`从数据资产加载配置: TestSword`
- 输出日志：`生成轻击连招: 3段`
- 输出日志：`开始连招: LightAttackCombo, 阶段: 0`
- 再次按键后：`连招阶段推进: LightAttackCombo -> 1`
- 再次按键后：`连招阶段推进: LightAttackCombo -> 2`
- 最后：`连招完成: LightAttackCombo, 完成阶段: 3`

**如何观察：**
- 打开输出日志窗口（Window → Developer Tools → Output Log）
- 搜索 `[WeaponComboIntegration]` 或 `[ComboDetector]`

---

### 测试 2：连打测试

**目的：** 验证快速连续输入能被正确处理

**步骤：**
1. 运行游戏
2. 快速连续点击鼠标左键 3 次

**期望结果：**
- 3次输入都被记录
- 连招正常完成
- 日志显示阶段从 0 → 1 → 2

**失败情况排查：**
- 如果只看到阶段 0：输入间隔太短，在连招开始前就发了多个输入
- 如果连招中断：输入间隔太长，超过 `InputToleranceTime`（默认0.5秒）

---

### 测试 3：连招超时中断

**目的：** 验证连招会在超时后正确中断

**步骤：**
1. 按下轻击键（开始连招）
2. 等待超过 0.5 秒（默认容忍时间）
3. 再次按下轻击键

**期望结果：**
- 输出日志：`连招中断: LightAttackCombo, 原因: 输入间隔过长`
- 新的输入会开始新连招（从阶段 0 开始）

---

### 测试 4：武器切换连招更新

**目的：** 验证切换武器后连招自动更新

**步骤：**
1. **创建第二种武器数据资产：**
   - 创建 `DA_TestAxe` 数据资产
   - **重要**：`WeaponType` 字段选择 `PanguAxe`（或不同于剑的任何类型）
   - `LightAttackSections` 只配置 **2个元素**（与剑的3段不同）
   - `LightAttackDamageMultipliers` 配置2个元素

2. **创建斧头武器蓝图：**
   - 创建 `BP_TestAxe`，父类 `WeaponBase`
   - 设置 `WeaponDataAsset` 为 `DA_TestAxe`
   - **重要**：确保 `WeaponType` 属性（从数据资产加载）与剑不同
   - 编译保存

3. **在角色蓝图中添加武器切换逻辑：**
```
BeginPlay
  │
  ├─→ Register Weapon (Sword, BP_TestSword, DA_TestSword)
  ├─→ Register Weapon (PanguAxe, BP_TestAxe, DA_TestAxe)
  ├─→ Equip Weapon (Sword)        // 先装备剑
  └─→ 保存引用：SwordRef, AxeRef

按键 1 (Pressed)  // 切换到剑
  └─→ WeaponManager → Equip Weapon (Sword)

按键 2 (Pressed)  // 切换到斧头
  └─→ WeaponManager → Equip Weapon (PanguAxe)
```

4. **测试流程：**
   - 运行游戏，确认当前是剑（3段连招）
   - 按下按键2切换到斧头

**期望日志输出：**
```
[WeaponManager] 开始切换武器 - 从 EWeaponType::Sword 到 EWeaponType::PanguAxe
[WeaponComboIntegration] 清空所有连招
[WeaponManager] 武器切换执行 - 新武器: 测试斧, 类型: EWeaponType::PanguAxe
[WeaponBase] 从数据资产加载配置: TestAxe
[WeaponComboIntegration] 生成轻击连招: 2段    ← 应该是2段！
[WeaponComboIntegration] 注册轻击连招: L-L
[WeaponManager] 武器切换完成 - 当前武器类型: EWeaponType::PanguAxe, 武器名称: 测试斧
```

**如果日志显示：**
```
[WeaponComboIntegration] 生成轻击连招: 3段    ← 错误！应该是2段
[WeaponManager] 武器切换完成 - 当前武器: EWeaponType::Sword    ← 错误！类型没变
```

**检查清单：**
- [ ] `DA_TestAxe` 的 `WeaponType` 字段是否设置为 `PanguAxe`（不是 `Sword`）
- [ ] `BP_TestAxe` 的 `WeaponDataAsset` 是否指向 `DA_TestAxe`
- [ ] 武器是否正确注册到 `WeaponManager`

---

## 调试方法

### 1. 控制台命令

```
log LogTemp Log          # 启用详细日志
log LogTemp Verbose      # 启用最详细日志
```

### 2. 蓝图调试

在 `ComboDetectorComponent` 上：
- 绑定 `OnComboStageAdvanced` 事件 → Print String
- 绑定 `OnComboBroken` 事件 → Print String

在 `WeaponManagerComponent` 上：
- 绑定 `OnCurrentWeaponChanged` 事件 → Print String（显示新武器名称）

### 3. 运行时检查变量

选中角色，在 Details 面板查看：
- `WeaponComboIntegration` → `IsInCombo` 状态
- `ComboDetector` → `Runtime State` 下的变量
- `WeaponManager` → `CurrentWeapon` 引用

---

## 常见问题排查

### 问题 1：按下攻击键没有反应

**检查清单：**
- [ ] 武器蓝图中设置了 `WeaponDataAsset`（不是类默认值中的属性，而是实例变量）
- [ ] 角色蓝图添加了 `WeaponComboIntegration` 组件
- [ ] 角色蓝图添加了 `ComboDetectorComponent` 组件
- [ ] `WeaponManager` 已成功装备武器（`EquipWeapon` 被调用）
- [ ] 输入映射正确配置（LightAttack 绑定到鼠标左键）
- [ ] 角色蓝图中正确调用了 `ProcessLightAttackInput`

**调试方法：**
```cpp
// 在 Output Log 中搜索
[WeaponBase] 从数据资产加载配置    # 确认配置加载成功
[WeaponComboIntegration] 生成轻击连招  # 确认连招生成
[ComboDetector] 开始连招           # 确认连招检测
```

### 问题 2：连招只有1段或2段（不是3段）

**检查清单：**
- [ ] `DA_TestSword` 的 `LightAttackSections` 数组有3个元素
- [ ] 武器蓝图正确引用了 `DA_TestSword`
- [ ] 日志中显示 `生成轻击连招: 3段`

**原因：**
如果显示 `生成轻击连招: 0段`，说明 `WeaponDataAsset` 没有正确加载到武器中。

### 问题 3：武器切换后连招段数没变（还是旧武器的段数）

**这是最可能的问题！**

**根本原因：**
`WeaponManager` 使用 `WeaponType` 枚举作为武器实例缓存的键。如果两个武器的 `WeaponType` 设置成相同的值，就会从缓存返回错误的武器实例。

**检查清单：**
1. 打开 `DA_TestSword`，检查 `WeaponType` 字段 → 应该是 `Sword`
2. 打开 `DA_TestAxe`，检查 `WeaponType` 字段 → 应该是 `PanguAxe`（不是 `Sword`）
3. 确保两个数据资产的 `WeaponType` 不同

**验证方法：**
查看日志中的武器切换日志：
```
[WeaponManager] 开始切换武器 - 从 EWeaponType::Sword 到 EWeaponType::PanguAxe
```
如果显示 `从 EWeaponType::Sword 到 EWeaponType::Sword`，说明 `DA_TestAxe` 的 `WeaponType` 设置错了。

### 问题 4：连招中断过早

**检查清单：**
- [ ] `ComboDetector` 的 `DefaultInputToleranceTime` 配置（默认0.5秒）
- [ ] 输入间隔是否合理

### 问题 5：武器切换后显示的还是旧武器名称

**检查清单：**
1. 检查 `DA_TestAxe` 的 `WeaponType` 是否与 `DA_TestSword` 不同
2. 检查 `BP_TestAxe` 的 `WeaponDataAsset` 是否指向 `DA_TestAxe`
3. 检查武器是否正确注册到 `WeaponManager`

---

## 数据流向图

```
配置阶段：
DA_TestSword (数据资产)
  ├── WeaponType: Sword           ← 重要！用于缓存键
  ├── LightAttackSections: [3个元素] ──┐
  └── LightAttackDamageMultipliers    │
                                      ↓
BP_TestSword (武器蓝图实例化时)
  ├── WeaponDataAsset = DA_TestSword  │
  └── BeginPlay()                     │
       └── LoadConfigFromDataAsset() ─┘
            ├── WeaponType = Sword
            └── LightComboStages [3个元素]
                     ↓
WeaponComboIntegration (BeginPlay时)
  └── GenerateLightAttackComboFromWeapon()
       └── FWeaponComboSequence (3段连招)
```

**武器切换时的缓存机制：**
```
UnequipCurrentWeapon()
  └── WeaponInstanceCache[Sword] = 剑实例

EquipWeapon(Axe)
  └── CreateWeaponInstance(Axe)
       └── 检查 WeaponInstanceCache[Axe] 
            ├── 如果存在 → 返回缓存的斧头实例
            └── 如果不存在 → 生成新实例
```

**如果 `DA_TestAxe.WeaponType` 错误地设置为 `Sword`：**
```
EquipWeapon(Axe)  // 请求斧头
  └── 但 DA_TestAxe.WeaponType 是 Sword
       └── CreateWeaponInstance 查找 Sword 的缓存
            └── 返回缓存的剑实例！
```

---

## 通过标准

| 测试项 | 通过条件 |
|--------|----------|
| 基础连招触发 | 能看到连招阶段推进日志，完成3段 |
| 连打测试 | 快速点击能完成完整连招 |
| 连招超时 | 等待后再次输入会开始新连招 |
| 武器切换 | 切换武器后连招段数更新，武器类型正确 |

---

**文档版本:** 2.3  
**对应系统版本:** Combo System Phase 3  
**Bug修复:** 武器切换时 `TickComponent` 调用 `FinishWeaponSwitch` 而不是 `DoWeaponSwitch`  
**最后更新:** 2026-03-06
