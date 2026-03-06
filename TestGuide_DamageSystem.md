# 伤害计算与受击反馈 测试指南

## 2026-03-06 Bug修复记录

### Bug 1：武器攻击不触发伤害检测（已修复）
**问题：** `PerformAttackTrace()` 函数写好了但没有被调用
**修复：** 在 `PerformLightAttack_Implementation` 和 `PerformHeavyAttack_Implementation` 中添加定时器调用攻击检测
**状态：** ✅ 已修复并编译成功

### Bug 2：接口调用方式问题（已修复）
**问题：** `FHitInfo` 前向声明缺失，可能导致接口调用失败
**修复：** 添加前向声明和空指针检查
**状态：** ✅ 已修复

### Bug 3：顿帧冻结游戏（已修复）
**问题：** 暴击时 `HitStopTimeDilation = 0.0f` 完全冻结游戏世界
**修复：** 改为 `0.05f`（极慢而非冻结），使用定时器恢复
**状态：** ✅ 已修复

### Bug 4：击退效果不生效（已修复）
**问题：** `UpdateKnockback` 使用 `MoveSmooth` 不可靠，且未正确配置
**修复：** 
- 改为 `LaunchCharacter` 实现击退
- 添加定时器自动恢复移动控制
- 更新测试指南说明必要配置
**状态：** ✅ 已修复

---

## 测试前准备

### 步骤1：确认已重新编译

**重要：** 确保你已经关闭了编辑器，并运行了以下命令重新编译：

```bash
E:\UE_5.6\UE_5.6\Engine\Build\BatchFiles\Build.bat niuniu555Editor Win64 Development -Project="C:\Users\Win10\Desktop\niuniu555\niuniu555.uproject" -Progress
```

编译成功后才能打开编辑器测试。

---

### 步骤2：创建测试目标

#### 2.1 创建蓝图类

1. 内容浏览器中 **右键** → **蓝图类**（Blueprint Class）
2. 父类选择 **Character**（⚠️ 必须是 Character，不是 Actor！HitReactionComponent 需要 CharacterMovementComponent）
3. 命名为 `BP_TestDummy`
4. 双击打开

**为什么必须是 Character？**
- HitReactionComponent 使用 `LaunchCharacter` 实现击退效果
- 这依赖 Character 的移动组件
- 如果使用 Actor 作为父类，击退功能将无法工作

#### 2.2 添加/检查组件

**默认组件检查：**
- Character 父类默认已有 `Mesh` 组件（骨骼网格体）
- 保留默认组件即可，无需额外添加 Static Mesh

**如果需要可视化网格体：**
1. 选中 `Mesh` 组件
2. 细节面板：
   - **Skeletal Mesh**：可选择 `SK_Mannequin`（或保持默认）
   - **Scale**：根据需要调整

**添加 HitReactionComponent：**
1. 点击 **添加组件**
2. 搜索 `HitReaction`，选择 **Hit Reaction**
3. 配置：
   - **Flash Duration**：`0.2`（闪白持续时间）
   - **Knockback Speed**：`800.0`（击退最大速度）
   - **Knockback Deceleration**：`2000.0`（击退减速度）

#### 2.3 添加 Damageable Interface 接口

1. 点击工具栏 **类设置**（Class Settings）
2. 细节面板找到 **实现的接口**（Implemented Interfaces）
3. 点击 **添加**
4. 选择 **Damageable Interface**

#### 2.4 创建变量

**我的蓝图** → **变量**：

| 变量名 | 类型 | 默认值 |
|--------|------|--------|
| CurrentHealth | Float | 1000.0 |
| MaxHealth | Float | 1000.0 |
| Defense | Float | 50.0 |
| PhysicalResistance | Float | 0.2 |

#### 2.5 实现接口函数

**Get Current Health：**
- 双击函数
- 拖动 **CurrentHealth** → **Return Value**

**Get Max Health：**
- 拖动 **MaxHealth** → **Return Value**

**Get Health Percent：**
- 创建 **除法**（Divide）节点
- **CurrentHealth** / **MaxHealth**
- 结果 → **Return Value**

**Is Alive：**
- 创建 **>** 节点
- **CurrentHealth** > 0
- 结果 → **Return Value**

**Can Be Damaged：**
- 勾选 **Return Value** 为 **true**

**Get Defense：**
- 拖动 **Defense** → **Return Value**

**Get Resistance：**
- 拖动 **PhysicalResistance** → **Return Value**

**Take Damage（受到伤害 - 核心）：**

1. 双击 **Take Damage**
2. 从 **Hit Info** 拖出 → **Break Hit Info**（拆解Hit Info）

**扣减生命值：**
1. 创建 **减法**（Subtract）节点：
   - **A**：拖动 **CurrentHealth**（Get）
   - **B**：Break Hit Info 的 **Damage**
2. 创建 **Set CurrentHealth**：
   - 输入：减法节点的 **结果**

**打印伤害：**
1. 创建 **Print String**
2. **In String**：输入 `"受到伤害！"`（先测试固定文本）
3. 勾选 **Print to Screen**
4. 连接到 Set CurrentHealth 之后

**判断死亡：**
1. 创建 **<=** 节点：**CurrentHealth** <= 0
2. 创建 **Branch** 节点
3. **True** → **Destroy Actor**（销毁Actor）

#### 2.6 编译保存

1. 点击 **编译**（锤子图标）
2. 等待显示 **✓ 编译成功**
3. 点击 **保存**

#### 2.7 放置到场景

将 `BP_TestDummy` 拖入场景，放置3-5个在玩家附近。

---

## 测试步骤

### 测试1：基础攻击命中

**操作：**
1. 打开 **输出日志** 窗口（窗口 → 开发者工具 → 输出日志）
2. 点击 **播放** 运行游戏
3. 攻击测试目标

**期望日志输出（按时间顺序）：**
```
[WeaponBase] 开始轻击 - 阶段: 0, Section: ...
[WeaponBase] PerformAttackTrace 被调用
[WeaponBase] 攻击检测: 起始=..., 结束=..., 范围=150, 通道=...
[WeaponBase] 攻击检测结果: 命中, 命中数量=1
[WeaponBase] 处理命中目标: BP_TestDummy_C_1
[WeaponBase] 目标 BP_TestDummy_C_1 实现 DamageableInterface: 是
[WeaponBase] 准备调用 TakeDamage，目标=BP_TestDummy_C_1, 伤害=85
```

**屏幕上显示：**
```
受到伤害！
```

**如果看到上述输出，说明系统工作正常！**

---

### 测试2：伤害数值计算

**操作：**
1. 修改 Print String 的文本为格式化文本：
   - 创建 **格式化文本**（Format Text）
   - 文本：`"伤害: {Dmg}, 剩余: {HP}"`
   - {Dmg} 连接 Break Hit Info 的 **Damage**
   - {HP} 连接 Set CurrentHealth 后的 **CurrentHealth**
2. 编译，运行游戏，攻击

**期望输出：**
```
伤害: 85, 剩余: 915
```

**计算公式验证：**
```
攻击力 100 × 倍率 1.0 = 100
防御减免：Max(100×0.1, 100-50) = 50
抗性减免：50 × (1-0.2) = 40
浮动后：40 × 0.9~1.1 = 36~44
```

---

### 测试3：暴击系统

**操作：**
1. 打开 **武器数据资产**（DA_TestSword 或你的武器数据）
2. 将 **Critical Rate** 改为 `1.0`（100%）
3. 保存，运行游戏，攻击

**期望结果：**
- 日志显示：`准备调用 TakeDamage, ..., 伤害=127`（比平常高1.5倍）
- 屏幕显示伤害数值变大
- 触发顿帧（游戏短暂冻结0.05秒）

**恢复：** 测试后将 **Critical Rate** 改回 `0.05`

---

### 测试4：防御减免

**操作：**
1. 选中场景中的 BP_TestDummy
2. 细节面板将 **Defense** 改为 `200`
3. 攻击，记录伤害
4. **Defense** 改为 `50`
5. 攻击，对比

**期望结果：**
- Defense=200 时伤害约 10（保底10%）
- Defense=50 时伤害约 40

---

### 测试5：受击反馈（闪白/击退）

**前提：** 
- BP_TestDummy 父类为 Character
- 已添加 HitReactionComponent
- **武器数据资产已配置击退力度**（见下方步骤）

#### 配置武器击退力度（关键步骤！）

1. 打开武器数据资产（如 `DA_TestSword`）
2. 找到 **Light Attack Knockback Forces**（轻击击退力度）
3. 展开数组，添加元素：
   - 索引 0（第1段）：`500.0`
   - 索引 1（第2段）：`600.0`
   - 索引 2（第3段）：`800.0`
4. 保存资产

**如果不配置击退力度，默认值为 0，击退效果不会触发！**

#### 配置测试假人

1. 选中 BP_TestDummy 的 HitReaction 组件
2. 配置：
   - **Flash Duration**：`0.2`
   - **Flash Color**：R=1, G=0.3, B=0.3（红色）
   - **Knockback Speed**：`800`（最大击退速度）
3. 编译保存

#### 测试操作

1. 运行游戏
2. 攻击 BP_TestDummy

**期望效果：**
- ✅ 受击时目标被击退一段距离（从攻击方向向后飞）
- ⚠️ 目标变红（闪白效果，需要材质支持）

**击退不生效的排查：**
1. 检查日志是否有 `"击退应用: 力度=..."` 输出
2. 检查武器资产的 Light Attack Knockback Forces 是否配置
3. 确认 BP_TestDummy 父类是 Character（不是 Actor）
4. 检查假人是否有 CharacterMovementComponent（默认应该有）

---

## 故障排查

### 问题1：看不到任何日志输出

**检查：**
1. 是否重新编译了项目？（必须关闭编辑器后编译）
2. 输出日志窗口是否已打开？
3. 控制台命令：`log LogTemp Log` 是否已执行？

### 问题2：看到 `[WeaponBase] 攻击检测结果: 未命中`

**原因：** 攻击检测执行了但没命中目标

**检查：**
1. 目标是否有碰撞体？（Static Mesh 的 Collision 设置）
2. 攻击范围是否足够大？（武器数据资产的 Attack Range）
3. 攻击通道是否正确？（Attack Trace Channel）

### 问题3：`实现 DamageableInterface: 否`

**原因：** 目标没有正确实现接口

**检查：**
1. BP_TestDummy → 类设置 → 实现的接口 是否有 Damageable Interface
2. 蓝图是否已编译？（必须显示绿色勾）

### 问题4：`准备调用 TakeDamage` 后没有反应

**原因：** 蓝图 Take Damage 函数有错误

**检查：**
1. Take Damage 函数中是否有断开的节点？
2. Print String 是否勾选了 Print to Screen？
3. 尝试简化：只保留一个 Print String，输入固定文本 `"测试"`

---

## 快速检查清单

| 检查项 | 通过标准 |
|--------|---------|
| 编译成功 | 无错误编译通过 |
| 攻击检测调用 | 日志出现 `PerformAttackTrace 被调用` |
| 命中目标 | 日志出现 `处理命中目标: BP_TestDummy` |
| 接口检测 | 日志出现 `实现 DamageableInterface: 是` |
| TakeDamage调用 | 日志出现 `准备调用 TakeDamage` |
| 蓝图响应 | 屏幕显示 `"受到伤害！"` |

---

**文档版本:** 3.1  
**Bug修复状态:** ✅ 攻击检测、顿帧、击退已修复  
**最后更新:** 2026-03-06
