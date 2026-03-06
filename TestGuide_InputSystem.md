# 🎮 输入系统测试操作指南

## 📌 测试目标
验证输入系统的功能：
1. 输入设备切换（键鼠/手柄）
2. 战斗输入缓冲
3. 连招输入序列检测
4. 输入优先级管理

---

## 输入系统简介

输入系统包含4个核心模块：

| 模块 | 功能 | 蓝图节点 |
|------|------|----------|
| **输入设备管理器** | 检测键鼠/手柄切换 | `Get Input Device Manager` |
| **输入缓冲系统** | 战斗输入预缓冲 | `Get Input Buffer System` |
| **连招输入系统** | 检测L/H连招序列 | `Get Combo Input System` |
| **输入优先级管理器** | 管理游戏/UI输入优先级 | `Get Input Priority Manager` |

---

## 测试方法（通过蓝图）

### 步骤1：打开关卡蓝图
1. 点击工具栏的 **蓝图(Blueprint)** → **打开关卡蓝图(Open Level Blueprint)**
2. 这将打开关卡蓝图编辑器

### 步骤2：测试输入设备管理器

#### 创建测试事件
1. 在关卡蓝图中右键，搜索 **Event Begin Play**，添加游戏开始事件
2. 从 **Begin Play** 拖动，搜索 **Get Game Instance**
3. 从 **Game Instance** 拖动，搜索 **Get Input Device Manager**

#### 测试设备切换事件
1. 从 **Input Device Manager** 拖动，选择 **Assign On Input Device Changed**
2. 连接一个 **Print String** 节点
3. 设置打印文本为：`"设备切换为: " + (New Device Type 转换为字符串)`
4. 点击 **编译(Compile)** 和 **保存(Save)**
5. 运行游戏，尝试按手柄按键或键盘按键
6. 查看输出日志是否显示设备切换

#### 检查当前设备类型
1. 添加 **键盘1** 键事件 (右键搜索 **1 Key**)
2. 连接 **Get Input Device Manager**
3. 调用 **Is Using Keyboard Mouse** 或 **Is Using Gamepad**
4. 打印结果
5. 运行游戏，按 **1** 键查看当前设备类型

---

### 步骤3：测试输入缓冲系统

#### 设置缓冲系统
1. 在关卡蓝图中获取 **Get Input Buffer System**
2. 设置缓冲时间：**Set Buffer Window** 为 `0.3`（300ms）
3. 设置最大缓冲数：**Max Buffer Size** 为 `3`

#### 添加输入到缓冲
1. 添加 **键盘Q** 键事件
2. 连接 **Buffer Input**，设置 **Action Type** 为 `Light Attack`
3. 添加 **键盘E** 键事件
4. 连接 **Buffer Input**，设置 **Action Type** 为 `Heavy Attack`
5. 添加 **键盘R** 键事件
5. 连接 **Buffer Input**，设置 **Action Type** 为 `Dodge`

#### 消费缓冲输入
1. 添加 **键盘空格** 键事件
2. 连接 **Consume Next Buffered Input**
3. 打印返回的 **Action Type**

#### 测试步骤
1. 编译保存
2. 运行游戏
3. 快速按 **Q-E-R**（模拟战斗预输入）
4. 按 **空格** 消费缓冲的输入
5. 查看输出日志，应该按顺序显示 Light Attack → Heavy Attack → Dodge

---

### 步骤4：测试连招输入系统

#### 注册连招
1. 在 **Event Begin Play** 后添加连招注册
2. 创建 **Make Combo Sequence** 节点：
   - **Combo ID**: `Combo_LLH`
   - **Combo Name**: `三连击`
   - **Input Sequence**: 数组，添加3个元素：Light, Light, Heavy
   - **Max Interval**: `0.8`
3. 调用 **Register Combo**

4. 再注册一个连招：
   - **Combo ID**: `Combo_HH`
   - **Combo Name**: `重击二连`
   - **Input Sequence**: Heavy, Heavy
   - **Max Interval**: `0.6`

#### 添加连招输入
1. 添加 **鼠标左键** 事件 → **Add Combo Input** → `Light`
2. 添加 **鼠标右键** 事件 → **Add Combo Input** → `Heavy`

#### 检测连招完成
1. 从 **Get Combo Input System** 获取 **On Combo Detected** 事件
2. 连接 **Print String**，打印 `"连招触发: " + Combo ID`

#### 查看当前序列
1. 添加 **键盘T** 键事件
2. 连接 **Get Current Sequence String**
3. 打印结果

#### 测试步骤
1. 编译保存
2. 运行游戏
3. 按 **鼠标左键-左键-右键**（L-L-H）
4. 应该看到输出 `"连招触发: Combo_LLH"`
5. 按 **T** 键查看当前输入序列
6. 等待1秒后，序列会自动清空（超时）

---

### 步骤5：测试输入优先级管理器

#### 切换输入模式
1. 获取 **Get Input Priority Manager**

2. 添加 **键盘M** 键事件 → **Open Menu**（打开菜单）
3. 添加 **键盘N** 键事件 → **Close Menu**（关闭菜单）
4. 添加 **键盘B** 键事件 → **Open UI**（打开UI）
5. 添加 **键盘V** 键事件 → **Close UI**（关闭UI）
6. 添加 **键盘C** 键事件 → **Enter Combat Mode**（进入战斗模式）
7. 添加 **键盘X** 键事件 → **Exit Combat Mode**（退出战斗模式）
8. 添加 **键盘P** 键事件 → **Pause Gameplay Input**（暂停输入）
9. 添加 **键盘O** 键事件 → **Resume Gameplay Input**（恢复输入）

#### 检查输入是否允许
1. 添加 **键盘I** 键事件
2. 调用 **Is Gameplay Input Allowed**
3. 打印结果

#### 测试步骤
1. 编译保存
2. 运行游戏

**测试菜单模式：**
- 按 **M** 打开菜单 → 鼠标应该出现，游戏输入被禁用
- 按 **I** 检查 → 应该显示 `False`（游戏输入不允许）
- 按 **N** 关闭菜单 → 恢复正常

**测试战斗模式：**
- 按 **C** 进入战斗模式
- 按 **M** 尝试打开菜单 → 应该可以打开（Push/Pop模式栈）
- 按 **N** 关闭菜单 → 返回战斗模式
- 按 **X** 退出战斗模式

**测试暂停：**
- 按 **P** 暂停 → 所有游戏输入禁用
- 按 **I** 检查 → 显示 `False`
- 按 **O** 恢复 → 恢复正常

---

## 📋 测试检查清单

### 输入设备管理器
- [ ] 按键盘时显示 `KeyboardMouse`
- [ ] 按手柄按键时显示 `Gamepad`
- [ ] `Is Using Keyboard Mouse` 返回正确值

### 输入缓冲系统
- [ ] 按Q/E/R后，空格消费显示对应动作
- [ ] 缓冲超过3个时，最旧的被移除
- [ ] 超过缓冲时间(0.3s)后输入被清空

### 连招输入系统
- [ ] L-L-H 触发 Combo_LLH
- [ ] H-H 触发 Combo_HH
- [ ] 超过0.8秒序列自动清空
- [ ] T键显示当前序列字符串

### 输入优先级管理器
- [ ] M打开菜单，鼠标出现，游戏输入禁用
- [ ] N关闭菜单恢复正常
- [ ] C进入战斗模式
- [ ] P暂停游戏输入
- [ ] O恢复游戏输入
- [ ] 模式栈工作正常（Push/Pop）

---

## 🔍 常见问题

### 问题1：获取不到子系统
**解决**：确保从 **Game Instance** 调用，而不是从其他对象

### 问题2：连招不触发
**解决**：
1. 检查输入序列是否正确（L-L-H）
2. 检查时间间隔是否超过 Max Interval
3. 在输出日志中查看是否有 "Combo detected" 日志

### 问题3：输入模式不生效
**解决**：
1. 确保有PlayerController存在
2. 检查是否在客户端上运行
3. 查看输出日志是否有错误

---

## ✅ 测试成功标志

- [x] 键鼠/手柄切换检测正常
- [x] 输入缓冲工作正常（提前输入有效）
- [x] 连招系统能正确识别L-L-H和H-H
- [x] 输入优先级能正确禁用/启用游戏输入
- [x] 所有蓝图节点都能正常使用

**输入系统测试完成！** 🎉
