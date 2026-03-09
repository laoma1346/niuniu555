# TargetingSystem（准星与锁定系统）测试指南

## 功能概述

准星与锁定系统提供以下功能：
- **硬锁定** - 手动锁定目标，摄像机跟随目标
- **软锁定** - 自动吸附到最近的敌人
- **准星扩散** - 移动/射击时的准星动态变化
- **目标筛选** - 基于距离、角度、可见性

## 测试前准备

### 1. 添加组件到玩家角色

1. 打开玩家角色蓝图（如 `BP_CombatCharacter`）
2. 添加组件 → 搜索 "Targeting" 或 "准星"
3. 选择 **准星与锁定系统 (TargetingSystemComponent)**
4. 重命名为 `TargetingComp`

### 2. 配置参数

选中组件，在细节面板中配置：

```
配置|锁定:
├── 最大锁定距离: 1500.0
├── 最大锁定角度: 45.0
└── 目标高度偏移: 100.0

配置|准星:
├── 准星扩散恢复速度: 2.0
└── 移动扩散: 0.2

配置|筛选:
└── 可锁定标签: Enemy

调试:
└── Show Debug: true (测试时开启，显示锁定范围)
```

### 3. 给敌人添加标签

确保场景中的敌人有 `"Enemy"` 标签：
1. 选中敌人蓝图
2. 在细节面板找到 **Tags**
3. 添加元素：`Enemy`

---

## 测试步骤

### 步骤1：硬锁定基础测试

**目的**：测试锁定/解锁功能

**蓝图设置**：
```
[按键事件 F] (锁定键)
    │
    ▼
[Get Component by Class] (TargetingSystemComponent)
    │
    ▼
[Toggle Lock On]
```

**操作**：
1. 运行游戏，靠近敌人（距离 < 1500）
2. 确保敌人在视野内（角度 < 45°）
3. 按 **F** 键

**预期结果**：
- 屏幕显示绿色圆锥（调试显示），表示锁定范围
- 锁定成功后，被锁定敌人显示红色球体标记
- 输出日志：`[TargetingSystem] 锁定目标: [敌人名]`

---

### 步骤2：切换锁定目标测试

**目的**：测试在多个敌人间切换

**蓝图设置**：
```
[按键事件 鼠标滚轮上] (或 Q键)
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Switch To Next Target]

[按键事件 鼠标滚轮下] (或 E键)
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Switch To Previous Target]
```

**操作**：
1. 锁定第一个敌人
2. 滚动鼠标滚轮（或按Q/E）

**预期结果**：
- 锁定目标切换到另一个敌人
- 输出日志：`[TargetingSystem] 切换到下一个目标: [新目标名]`

---

### 步骤3：软锁定测试

**目的**：测试摄像机自动吸附

**蓝图设置**：
```
[Event Tick]
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Get Soft Lock Target]
    │
    ▼
[Is Valid] (检查是否有效)
    │
    ▼
[Print String] (每帧显示，可选)
    └── In String: "软锁定目标: [目标名]"
```

**操作**：
1. 不按下锁定键，仅将准星对准敌人
2. 确保敌人在中心区域（角度 < 15°）

**预期结果**：
- 当准星靠近敌人时，摄像机会轻微吸附
- 获取到软锁定目标

---

### 步骤4：准星扩散测试

**目的**：测试准星动态扩散

**蓝图设置**：
```
[按键事件 鼠标左键] (射击)
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Add Crosshair Spread]
    └── Amount: 0.3

[Event Tick]
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Get Crosshair Spread]
    │
    ▼
[Print String] (显示扩散值)
```

**操作**：
1. 连续点击鼠标左键（模拟射击）
2. 观察准星扩散值

**预期结果**：
- 每次射击扩散增加
- 停止射击后，扩散逐渐恢复（恢复速度 2.0/s）
- 扩散值范围：0.0 - 1.0

---

### 步骤5：与摄像机集成测试

**目的**：测试锁定时摄像机跟随目标

**蓝图设置（在角色蓝图中）**：
```
[Event Tick]
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Is Locked On] (分支)
    │
    ├─ True ──► [Get Target Lock Location]
    │             │
    │             ▼
    │           [Set Control Rotation]
    │             └── New Rotation: [Look At 目标位置]
    │
    └─ False ──► (正常摄像机控制)
```

**或使用更简单的方式**：
```
[Get TargetingComp]
    │
    ▼
[Get Current Target]
    │
    ▼
[Is Valid] (如果有目标)
    │
    ▼
[Get Actor Location] (目标位置)
    │
    ▼
[Find Look at Rotation]
    ├── Start: 玩家位置
    └── Target: 目标位置 + (0, 0, 100)
    │
    ▼
[Set Control Rotation] (或使用RInterp To平滑过渡)
```

**预期结果**：
- 锁定时，摄像机自动对准敌人
- 移动时，摄像机保持跟随敌人
- 超出范围（>1500或角度>45°）自动解锁

---

### 步骤6：解锁条件测试

**目的**：测试自动解锁逻辑

**操作**：
1. 锁定一个敌人
2. 远离敌人（距离 > 1500）
3. 或者转身背对敌人（角度 > 90°）

**预期结果**：
- 自动解除锁定
- 输出日志：`[TargetingSystem] 解除锁定: [目标名]`

---

## 蓝图工具函数

### 获取最佳锁定目标（用于AI或自动瞄准）
```
[Get TargetingComp]
    │
    ▼
[Get All Lockable Targets]
    │
    ▼
[For Each Loop]
    └── 遍历所有可锁定目标
```

### 检查目标是否可锁定
```
[Get TargetingComp]
    │
    ▼
[Can Lock On Target]
    ├── Target: [指定Actor]
    └── Return Value: 是否可锁定
```

### 获取目标角度（用于UI显示）
```
[Get TargetingComp]
    │
    ▼
[Get Target Angle]
    ├── Target: [敌人]
    └── Return Value: 角度（度数）
```

---

## 事件绑定示例

### 锁定状态变化事件
```
[Event BeginPlay]
    │
    ▼
[Get TargetingComp]
    │
    ▼
[Bind Event to OnLockOnStateChanged]
    └── 创建自定义事件: OnLockStateChanged

[自定义事件: OnLockStateChanged]
    ├── bValue: 是否锁定
    │
    ▼
[Branch]
    ├─ True ──► [Print String] "已锁定目标！"
    └─ False ──► [Print String] "解锁！"
```

### 目标变化事件
```
[Bind Event to OnTargetChanged]
    └── 创建自定义事件: OnTargetChanged

[自定义事件: OnTargetChanged]
    ├── Actor: 新目标
    │
    ▼
[Print String]
    └── In String: "切换到新目标: [Actor名]"
```

---

## 调试显示说明

开启 **Show Debug** 后，屏幕上显示：
- **黄色圆锥** - 锁定检测范围（角度45°，距离1500）
- **红色球体** - 当前锁定目标
- **红色连线** - 从玩家到目标的连线

---

## 常见问题

### 问题1：无法锁定
**症状**：按F键没有反应

**检查**：
1. 敌人是否有 `"Enemy"` 标签
2. 是否在锁定范围内（距离 < 1500）
3. 是否开启 Show Debug 查看范围
4. 检查日志输出

### 问题2：锁定后摄像机不跟随
**症状**：锁定成功但摄像机不动

**解决**：需要在角色蓝图中手动实现摄像机跟随逻辑（见步骤5）

### 问题3：切换目标无效
**症状**：滚轮切换没有反应

**检查**：
1. 是否有多个敌人在范围内
2. 切换角度阈值是否设置合理

---

## 测试通过标准

- [ ] 能成功锁定视野内的敌人
- [ ] 能切换多个锁定目标
- [ ] 远离敌人自动解锁
- [ ] 准星扩散随射击增加
- [ ] 停止射击后准星扩散恢复
- [ ] 事件委托正确触发

---

*文档版本: 2026-03-09*  
*对应代码版本: TargetingSystem Component*
