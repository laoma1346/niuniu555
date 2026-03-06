# 🎮 游戏状态机与场景管理器测试操作指南

## 📌 测试目标
验证游戏状态流转和场景切换功能

---

## 测试方法

### 步骤1：打开关卡蓝图
1. 打开任意地图（如 `CombatTestMap`）
2. 点击 **蓝图(Blueprint)** → **打开关卡蓝图(Open Level Blueprint)**

### 步骤2：获取游戏流程控制器

在 **Event Begin Play** 后：
1. 搜索 **Get Game Instance**
2. 从 **Game Instance** 拖动，搜索 **Get Game Flow Controller**
3. 调用 **Initialize Game**

### 步骤3：测试状态机

#### 绑定状态变化事件
1. 从 **Get Game Flow Controller** → **Get State Machine**
2. 从 **State Machine** 拖动，选择 **Assign On Game State Changed**
3. 连接 **Print String**，打印：
   ```
   "状态变化: " + Transition Info.New State + " (来自: " + Transition Info.Previous State + ")"
   ```

#### 测试状态转换
添加按键测试：

| 按键 | 操作 | 预期状态 |
|------|------|----------|
| **1** | Get State Machine → Start Game | Camp |
| **2** | Get State Machine → Enter Map Selection | MapSelection |
| **3** | Get Flow Controller → From Map To Combat (LevelName="Test") | Combat |
| **4** | Get State Machine → Pause Game | Pause |
| **5** | Get State Machine → Resume Game | 返回之前状态 |
| **6** | Get Flow Controller → Toggle Pause | 切换暂停 |
| **7** | Get State Machine → Combat Settlement (bVictory=true) | Settlement |
| **8** | Get Flow Controller → Return To Main Menu | MainMenu |
| **9** | Get State Machine → Get Current State | 打印当前状态 |

#### 查询状态信息
- **Get Current State** - 获取当前状态
- **Get Previous State** - 获取上一个状态
- **Get Current Flow State** - 获取流程状态（OutOfGame/Preparation/Adventure/Combat/Settlement）
- **Get State Duration** - 获取当前状态持续时间
- **Is In Game** - 是否在游戏中
- **Can Pause** - 是否可以暂停

### 步骤4：测试场景管理器

#### 绑定场景加载事件
1. 从 **Get Flow Controller** → **Get Scene Manager**
2. 从 **Scene Manager** 拖动：
   - **Assign On Scene Loading Started** → Print String
   - **Assign On Scene Loading Complete** → Print String

#### 测试场景切换
添加按键测试：

| 按键 | 操作 | 说明 |
|------|------|------|
| **Q** | Load Main Menu | 加载主菜单 |
| **W** | Load Camp | 加载营地 |
| **E** | Load Map Selection | 加载地图选择 |
| **R** | Load Combat Scene (CombatLevelName="Test") | 加载战斗场景 |
| **T** | Reload Current Scene | 重新加载当前场景 |
| **Y** | Get Current Scene Name | 打印当前场景名 |

### 步骤5：测试流程控制器

#### 完整流程测试
按顺序按键测试完整游戏流程：

```
1. 按 1 → 进入营地
2. 按 2 → 进入地图选择  
3. 按 3 → 进入战斗
4. 按 4 → 暂停
5. 按 5 → 恢复
6. 按 7 → 战斗胜利结算
7. 按 8 → 返回主菜单
```

---

## 📋 状态流转图

```
None → MainMenu → Camp → MapSelection
                           ↓
    ┌─────────────────────┼─────────────────────┐
    ↓                     ↓                     ↓
Combat ←─────────────── Event                 Shop
    ↓                                           ↓
Settlement ──→ MapSelection ←─────────────── SkillTree
    ↓
GameOver → MainMenu
```

---

## ✅ 测试检查清单

### 状态机测试
- [ ] Start Game → 状态变为 Camp
- [ ] Enter Map Selection → 状态变为 MapSelection
- [ ] Start Combat → 状态变为 Combat
- [ ] Pause Game → 状态变为 Pause
- [ ] Resume Game → 状态恢复为之前状态
- [ ] Combat Settlement → 状态变为 Settlement
- [ ] Return To Main Menu → 状态变为 MainMenu
- [ ] 状态变化事件正确触发
- [ ] Get State Duration 显示正确的时间

### 场景管理器测试
- [ ] Load Scene 触发场景加载事件
- [ ] 场景加载完成后触发完成事件
- [ ] Get Current Scene Name 显示正确的场景名
- [ ] Reload Current Scene 重新加载当前场景

### 流程控制器测试
- [ ] Start New Game 初始化游戏
- [ ] Is In Game 在战斗中返回 true
- [ ] Can Pause 在战斗中返回 true
- [ ] Toggle Pause 正确切换暂停状态

---

## 🔍 常见问题

### 问题1：状态转换失败
**解决**：检查是否允许该状态转换（如不能直接从 MainMenu 到 Combat）

### 问题2：场景加载没反应
**解决**：检查场景名是否正确，场景文件是否存在

### 问题3：获取不到子系统
**解决**：确保从 **Game Instance** 获取，不是在构造函数或早期阶段调用

---

## ✅ 测试成功标志

- [x] 所有状态转换正常
- [x] 状态变化事件正确触发
- [x] 场景加载事件正确触发
- [x] 暂停/恢复功能正常
- [x] 流程控制器的便捷方法正常工作

**游戏状态机系统测试完成！** 🎉
