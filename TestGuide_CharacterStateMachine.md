# CharacterStateMachine 测试指南

## 功能概述

角色状态机组件统一管理角色的所有状态，包括：
- **Idle** (待机) - 站立不动状态
- **Move** (移动) - 奔跑/行走状态
- **Attack** (攻击) - 攻击动作中
- **Dodge** (闪避) - 闪避动作中
- **Hit** (受击) - 被击中状态
- **Stun** (眩晕) - 无法行动状态
- **Dead** (死亡) - 死亡状态
- **Interact** (交互) - 与物体交互中
- **Skill** (技能) - 释放技能中

## 测试前准备

### 1. 确认编译成功
确保C++代码已编译，没有错误。

### 2. 给玩家角色添加组件

1. 打开玩家角色蓝图（如 `BP_CombatCharacter`）
2. 点击 **添加组件** 按钮
3. 搜索 `CharacterStateMachine` 或 `角色状态机`
4. 选择 **角色状态机 (CharacterStateMachineComponent)**
5. 重命名为 `StateMachineComp`
6. 在细节面板中：
   - 勾选 **Enable Debug Log**（启用调试日志）
   - 勾选 **Show Debug Info**（在屏幕上显示状态）

## 测试步骤

### 步骤1：基础状态显示测试

**目的**：验证状态机正常工作，状态能在屏幕上显示

**操作**：
1. 点击 **播放** 运行游戏
2. 观察屏幕左上角

**预期结果**：
- 显示文本：`State: Idle`
- 显示文本：`Duration: x.xx`（持续时间）

**日志输出**（输出日志窗口搜索 `LogCharacterStateMachine`）：
```
[BP_CombatCharacter] 状态机初始化完成，初始状态: Idle
```

---

### 步骤2：状态切换测试（蓝图）

**目的**：测试通过蓝图切换状态

**操作**：

1. **打开关卡蓝图**（窗口 → 关卡蓝图）

2. **创建测试事件**：
```
[按键事件 1] (数字键1)
    │
    ▼
[Get Player Pawn]
    │
    ▼
[Get Component by Class]
    ├── Component Class: CharacterStateMachineComponent
    │
    ▼
[Try Change State]
    ├── New State: Attack
    └── Return Value: 连接到 [Print String]
    │
    ▼
[Print String]
    └── In String: "切换到Attack状态"
```

3. **运行游戏**，按数字键 **1**

**预期结果**：
- 屏幕左上角显示变为：`State: Attack`
- 打印字符串：`切换到Attack状态`
- 日志输出：`[BP_CombatCharacter] 状态切换: Idle -> Attack`

---

### 步骤3：状态切换限制测试

**目的**：验证状态转换规则是否生效

**操作**：

1. **从Idle切换到Dodge**（应该成功）：
```
[按键事件 2] -> Try Change State -> Dodge
```
- **预期**：切换成功，显示 `State: Dodge`

2. **从Dodge切换到Attack**（应该失败）：
```
[在Dodge状态中按按键1]
```
- **预期**：切换失败，保持 `State: Dodge`
- 日志输出：`[BP_CombatCharacter] 状态切换被拒绝: Dodge -> Attack`

3. **Dodge结束后自动回到Idle**（需要蓝图控制）：
```
[Delay 0.5秒] -> Try Change State -> Idle
```
- **预期**：回到 `State: Idle`

---

### 步骤4：动作可行性检查测试

**目的**：测试CanAttack、CanDodge等检查函数

**操作**：

1. **创建检查节点**：
```
[Event Tick] (每帧检查)
    │
    ▼
[Get Component by Class] (StateMachineComp)
    │
    ▼
[Can Attack]
    │
    ▼
[Print String] (只在CanAttack返回True时)
    └── In String: "可以攻击！"
```

2. **测试不同状态**：
   - **Idle状态**：应该显示 "可以攻击！"
   - **Attack状态**：应该显示 "可以攻击！"（可以继续连招）
   - **Dodge状态**：不显示（不能攻击）
   - **Hit状态**：不显示（不能攻击）

---

### 步骤5：与现有系统集成测试

**目的**：验证状态机与DodgeComponent、WeaponSystem协同工作

#### 与闪避系统集成：

```
[按键事件 空格] (闪避键)
    │
    ▼
[Get StateMachineComp]
    │
    ▼
[Can Dodge] (分支检查)
    │
    ├─ True ──► [Get DodgeComp] -> [Perform Dodge]
    │             │
    │             ▼
    │           [Get StateMachineComp] -> [Try Change State -> Dodge]
    │
    └─ False ──► [Print String] "现在不能闪避！"
```

**预期结果**：
- Idle/Move/Attack状态可以闪避
- Dodge/Hit/Stun状态不能闪避

#### 与武器系统集成：

```
[鼠标左键] (攻击键)
    │
    ▼
[Get StateMachineComp]
    │
    ▼
[Can Attack] (分支检查)
    │
    ├─ True ──► [Get WeaponManager] -> [Perform Attack]
    │             │
    │             ▼
    │           [Get StateMachineComp] -> [Try Change State -> Attack]
    │
    └─ False ──► [Print String] "现在不能攻击！"
```

---

### 步骤6：状态事件委托测试

**目的**：测试OnStateChanged等事件

**操作**：

1. **在玩家角色蓝图中绑定事件**：
```
[Event BeginPlay]
    │
    ▼
[Get StateMachineComp]
    │
    ▼
[Bind Event to OnStateChanged]
    └── Event: 创建自定义事件 [OnStateChangedEvent]
```

2. **创建自定义事件处理**：
```
[自定义事件: OnStateChangedEvent]
    ├── Old State: 参数
    ├── New State: 参数
    │
    ▼
[Print String]
    └── In String: "状态从 {OldState} 变为 {NewState}"
```

**预期结果**：
- 每次状态切换时，打印状态变化信息

---

## 日志搜索指南

### 输出日志窗口搜索关键词：

```
搜索: LogCharacterStateMachine
类别: LogCharacterStateMachine

搜索: 状态机初始化
结果: [BP_CombatCharacter] 状态机初始化完成，初始状态: Idle

搜索: 状态切换:
结果: [BP_CombatCharacter] 状态切换: Idle -> Attack

搜索: 状态切换被拒绝
结果: [BP_CombatCharacter] 状态切换被拒绝: Dodge -> Attack
```

---

## 常见问题排查

### 问题1：状态不显示

**症状**：屏幕左上角没有状态显示

**解决**：
1. 检查 **Show Debug Info** 是否勾选
2. 检查组件是否正确添加到角色
3. 检查日志是否有初始化信息

### 问题2：状态切换无效

**症状**：按按键后状态没有变化

**解决**：
1. 检查 Try Change State 的返回值（True/False）
2. 检查日志中的 "状态切换被拒绝" 信息
3. 确认当前状态是否允许切换到目标状态

### 问题3：CanAttack始终返回False

**症状**：无法攻击

**解决**：
1. 检查当前状态（可能是Dodge/Hit/Stun）
2. 检查状态机组件是否正确获取

---

## 状态转换规则速查表

| 当前状态 | 可切换至 | 不可切换至 |
|---------|---------|-----------|
| Idle | 任何状态 | - |
| Move | 任何状态 | - |
| Attack | Idle, Move, Dodge, Hit, Stun, Dead | Interact |
| Dodge | Idle, Move, Hit, Stun | Attack, Skill, Interact |
| Hit | Idle, Move, Stun, Dead | Attack, Dodge, Skill, Interact |
| Stun | Idle, Dead | 其他所有 |
| Dead | - | 任何状态（需要复活机制） |

---

## 测试通过标准

- [ ] 屏幕正确显示当前状态
- [ ] Idle可以切换到Attack
- [ ] Attack可以切换到Dodge
- [ ] Dodge不能切换到Attack（被拒绝）
- [ ] CanAttack在Idle时返回True
- [ ] CanAttack在Dodge时返回False
- [ ] 状态切换事件正确触发
- [ ] 与DodgeComponent集成正常
- [ ] 与WeaponSystem集成正常

---

*文档版本: 2026-03-09*  
*对应代码版本: CharacterStateMachine Component*
