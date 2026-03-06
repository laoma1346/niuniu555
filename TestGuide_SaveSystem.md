# 🎮 存档系统测试操作指南

## 📌 测试目标
验证存档系统的基本功能：创建存档、保存数据、读取数据、删除存档

---

## 步骤1：打开项目

### 1.1 启动UE5编辑器
1. 双击打开 `niuniu555.uproject` 文件
2. 等待编辑器完全加载（可能需要1-2分钟）

### 1.2 打开测试地图
1. 在内容浏览器中找到 `Content/Variant_Combat/Maps` 文件夹
2. 双击打开 `CombatTestMap` 地图
3. 等待地图加载完成

---

## 步骤2：放置测试Actor

### 2.1 打开放置Actor面板
1. 按键盘上的 **F1** 或点击菜单栏的 **窗口(Window)** → **放置Actor(Place Actors)**

### 2.2 搜索并放置测试Actor
1. 在放置Actor面板的搜索框中输入：`Save System Test`
2. 将 **SaveSystemTestActor** 拖到场景中的任意位置（比如地面上）
3. 点击选中这个Actor

### 2.3 查看Actor详情
1. 在右侧的 **细节(Details)** 面板中，可以看到这个Actor的各种功能按钮
2. 向下滚动找到 **Save System | Test** 折叠菜单，点击展开

---

## 步骤3：测试存档功能

### 3.1 测试创建新存档

#### 方法A：通过Details面板（推荐小白使用）
1. 确保选中了场景中的 **SaveSystemTestActor**
2. 在 **Details面板** 中找到 **Test Create Save** 按钮
3. 修改参数：
   - **Slot Index**: `0` （使用第1个存档槽位）
   - **Save Name**: `我的第一个存档` （自定义名称）
4. 点击 **Test Create Save** 按钮
5. **查看结果**：
   - 看屏幕右上角的弹出通知
   - 看底部的 **输出日志(Output Log)** 窗口（如果没有，按 **` 键或 ~ 键打开）

#### 方法B：运行时按键测试（更像真实游戏）
1. 选中 **SaveSystemTestActor**
2. 在Details面板中找到 **蓝图(Blueprint)** 菜单，点击 **打开关卡蓝图(Open Level Blueprint)**
3. 在关卡蓝图中右键，搜索 **Test Create Save**
4. 连接一个按键事件：
   - 右键搜索 **1** 键事件 (Keyboard Events -> 1)
   - 把 **Pressed** 引脚连接到 **Test Create Save** 节点
5. 点击 **编译(Compile)** 按钮
6. 点击 **播放(Play)** 运行游戏
7. 按键盘上的 **1** 键
8. 看输出日志显示创建结果

### 3.2 测试添加金币
1. 选中 **SaveSystemTestActor**
2. 找到 **Test Add Gold** 按钮
3. 设置 **Amount** 为 `100`
4. 点击按钮
5. 查看输出日志，应该显示添加了100金币

### 3.3 测试添加神格碎片
1. 找到 **Test Add God Fragments** 按钮
2. 设置 **Amount** 为 `5`
3. 点击按钮
4. 查看输出日志

### 3.4 测试解锁技能
1. 找到 **Test Unlock Skill** 按钮
2. 在 **Skill Id** 输入框输入：`Skill_Attack_01`
3. 点击按钮
4. 查看输出日志

### 3.5 查看当前存档信息
1. 找到 **Test Print Current Save Info** 按钮
2. 点击按钮
3. 在输出日志中查看当前存档的所有数据：
   - 金币数量
   - 神格碎片数量
   - 已解锁技能数量
   - 游戏时长

### 3.6 保存当前游戏
1. 找到 **Test Save Current Game** 按钮
2. 点击按钮
3. 等待保存完成（会有通知）

### 3.7 测试读取存档
1. 先点击 **Test Print All Save Info** 查看所有存档
2. 找到 **Test Load Save** 按钮
3. 设置 **Slot Index** 为 `0`
4. 点击按钮
5. 查看加载结果

### 3.8 测试删除存档
1. 找到 **Test Delete Save** 按钮
2. 设置 **Slot Index** 为 `0`
3. 点击按钮
4. 再次点击 **Test Print All Save Info** 确认存档已删除

---

## 步骤4：使用蓝图函数库（用于UI制作）

### 4.1 在蓝图中调用存档功能
1. 打开任意蓝图（比如角色蓝图或关卡蓝图）
2. 右键搜索以下节点测试：
   - `Get All Save Slot Infos` - 获取所有存档槽位信息
   - `Get Save Slot Info` - 获取指定槽位信息
   - `Does Save Slot Exist` - 检查存档是否存在
   - `Format Play Time` - 格式化游戏时长

### 4.2 示例：制作简单的存档列表
1. 在关卡蓝图中
2. 按 **1** 键连接 `Get All Save Slot Infos`
3. 从返回值拖动，选择 **For Each Loop**
4. 在循环体内打印每个存档的信息：
   - 右键搜索 **Print String**
   - 连接 **Slot Index** 和 **Display Name** 到打印节点

---

## 步骤5：测试自动保存组件

### 5.1 给角色添加自动保存
1. 在内容浏览器中找到玩家角色（比如 **CombatCharacter**）
2. 双击打开角色蓝图
3. 在 **组件(Components)** 面板点击 **添加组件(Add Component)**
4. 搜索 `Save Auto Save`，选择 **SaveAutoSaveComponent**
5. 在Details面板中配置：
   - **Enable Auto Save**: 勾选 ✓
   - **Auto Save Interval**: `10` （每10秒自动保存一次）
   - **Auto Save Slot Index**: `0`
6. 点击 **编译** 和 **保存**
7. 运行游戏，等待10秒，看自动保存是否触发

---

## 📋 测试检查清单

| 测试项 | 操作 | 预期结果 |
|--------|------|----------|
| ✅ 创建存档 | 点击 Test Create Save | 输出日志显示"创建成功" |
| ✅ 添加金币 | 点击 Test Add Gold | 金币数量增加 |
| ✅ 添加碎片 | 点击 Test Add God Fragments | 碎片数量增加 |
| ✅ 解锁技能 | 点击 Test Unlock Skill | 技能被添加到列表 |
| ✅ 保存游戏 | 点击 Test Save Current Game | 保存成功通知 |
| ✅ 查看存档 | 点击 Test Print All Save Info | 显示所有存档信息 |
| ✅ 加载存档 | 点击 Test Load Save | 加载成功通知 |
| ✅ 删除存档 | 点击 Test Delete Save | 存档被删除 |

---

## 🐛 常见问题

### 问题1：找不到 SaveSystemTestActor
**解决**：
1. 确保编辑器完全加载
2. 在放置Actor面板点击 **刷新(Refresh)** 按钮
3. 或者直接在内容浏览器中搜索 `BP_SaveSystemTest`

### 问题2：点击按钮没有反应
**解决**：
1. 确保选中了场景中的Actor
2. 检查输出日志是否有红色错误信息
3. 确保存档子系统已初始化（运行一次游戏即可）

### 问题3：存档文件在哪里？
**答案**：
- 位置：`C:\Users\Win10\AppData\Local\niuniu555\Saved\SaveGames\`
- 文件格式：`.sav` 文件
- 注意：这些文件是加密的，直接打开看不到明文

---

## ✅ 测试成功标志
当你能成功：
1. 创建一个存档
2. 添加金币和碎片
3. 保存游戏
4. 重新加载编辑器后读取存档
5. 看到之前的数据还在

**恭喜你！存档系统工作正常！** 🎉
