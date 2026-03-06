# 🎮 属性系统测试操作指南（含英文原名对照）

## 📌 重要说明 - 节点英文原名对照

在UE引擎中搜索节点时，如果中文翻译找不到，请尝试搜索英文原名：

| 中文翻译（可能不准确） | 英文原名 | 作用说明 |
|------------------------|----------|----------|
| 获取组件 | **Get Component** | 从Actor获取组件 |
| 添加组件 | **Add Component** | 添加组件到Actor |
| 构造对象 | **Construct Object** | 创建UObject对象 |
| 获取游戏实例 | **Get Game Instance** | 获取GameInstance |

---

## 测试方法

### 步骤1：给角色添加属性组件

#### 添加组件
1. 打开玩家角色蓝图（如 `CombatCharacter`）
2. 在 **组件(Components)** 面板点击 **添加组件(Add Component)**
3. 搜索：**Attribute** 或 **属性组件**
4. 选择 **Attribute Component**（英文原名）
5. 保存并编译

**节点对照**：
- 中文：`添加组件` / 英文：`Add Component`
- 作用：给Actor添加功能组件

---

### 测试2：获取属性值

#### 在关卡蓝图中获取玩家属性
1. 打开关卡蓝图
2. 添加 **键盘1** 键事件
3. **Get Player Pawn**（获取玩家角色）
4. 从 **Return Value** 拖动，搜索 **Get Component**
5. 选择 **Get Component by Class**
   - **Component Class**: **Attribute Component**
6. 从返回值拖动，调用 **Get Health**（获取生命值）
7. **Print String**: `"生命值: " + 返回值`

**节点对照**：
- 中文：`按类获取组件` / 英文：`Get Component by Class`
- 作用：从Actor获取指定类型的组件

---

### 测试3：修改生命值

#### 治疗/伤害
1. 添加 **键盘2** 键事件
2. 获取玩家的 **Attribute Component**（同上）
3. 调用 **Modify Health**（修改生命值）
   - **Delta**: `-20`（负数为伤害）
4. **Print String**: `"受到20点伤害"`

#### 治疗
1. 添加 **键盘3** 键事件
2. 调用 **Modify Health**
   - **Delta**: `+30`（正数为治疗）
3. **Print String**: `"恢复30点生命"`

---

### 测试4：生命值变化事件

#### 绑定生命值变化事件
1. 在 **Event Begin Play** 中：
2. 获取玩家的 **Attribute Component**
3. 从返回值拖动，选择 **Assign On Health Changed**（分配生命值变化事件）
4. 连接 **Print String**：
   - 打印：`"生命变化: 当前" + Current Health + "/" + Max Health`

**节点对照**：
- 中文：`分配 On Health Changed` / 英文：`Assign On Health Changed`
- 作用：当生命值变化时触发回调

---

### 测试5：添加Buff（攻击力加成）

#### 添加修饰器组件
1. 打开玩家角色蓝图
2. **添加组件** → **Attribute Modifier**（属性修饰器组件）
3. 保存并编译

#### 添加攻击力Buff
1. 在关卡蓝图中，添加 **键盘4** 键事件
2. 获取玩家的 **Attribute Modifier Component**
3. 调用 **Add Attack Buff**（添加攻击Buff）
   - **Value**: `+20`（增加20攻击力）
   - **Duration**: `10`（持续10秒）
   - **Buff ID**: `Buff_Attack_01`
4. **Print String**: `"攻击力+20，持续10秒"`

**节点对照**：
- 中文：`添加攻击增益` / 英文：`Add Attack Buff`
- 作用：临时增加攻击力

---

### 测试6：添加移动速度Buff

#### 添加移速Buff
1. 添加 **键盘5** 键事件
2. 获取 **Attribute Modifier Component**
3. 调用 **Add Speed Buff**
   - **Value**: `0.5`（增加50%移速）
   - **Duration**: `5`（持续5秒）
   - **Buff ID**: `Buff_Speed_01`
4. **Print String**: `"移动速度+50%，持续5秒"`

---

### 测试7：查看属性

#### 打印所有属性
1. 添加 **键盘6** 键事件
2. 获取 **Attribute Component**
3. 依次调用并打印：
   - **Get Health** / **Get Max Health**
   - **Get Attack**
   - **Get Defense**
   - **Get Move Speed**

---

### 测试8：死亡事件

#### 绑定死亡事件
1. 在 **Event Begin Play** 中：
2. 获取 **Attribute Component**
3. 选择 **Assign On Death**（分配死亡事件）
4. 连接 **Print String**: `"玩家死亡！"`

---

## 📋 测试检查清单

### 基础属性
- [ ] 能获取当前生命值
- [ ] 能修改生命值（伤害/治疗）
- [ ] 生命值变化事件触发正常
- [ ] 生命值不超过最大值
- [ ] 生命值为0时触发死亡事件

### Buff系统
- [ ] 能添加攻击力Buff
- [ ] Buff持续时间结束后自动移除
- [ ] 攻击力正确增加
- [ ] 能添加移动速度Buff
- [ ] 能移除指定Buff

### 事件系统
- [ ] 生命值变化事件正确触发
- [ ] 死亡事件正确触发

---

## 🔍 常见问题与节点对照

### 问题1：找不到 Attribute Component
**解决**：
- 搜索英文：**Attribute Component**
- 或搜索：**AttributeComp**

### 问题2：找不到 Get Component by Class
**解决**：
- 搜索英文：**Get Component by Class**
- 作用说明：从Actor获取指定类型的组件

### 问题3：找不到 Assign On Health Changed
**解决**：
- 先获取 **Attribute Component** 变量
- 从变量拖动，搜索 **Health Changed**
- 或搜索英文：**Assign On Health Changed**

### 问题4：Buff没有生效
**解决**：
- 确保角色有 **Attribute Modifier Component**
- 确保角色有 **Attribute Component**
- 检查 Buff ID 是否唯一

---

## ✅ 测试成功标志

- [x] 属性组件能正确添加
- [x] 能获取和修改生命值
- [x] 生命值变化事件触发
- [x] Buff能正确添加和移除
- [x] 死亡事件正确触发

**属性系统测试完成！** 🎉
