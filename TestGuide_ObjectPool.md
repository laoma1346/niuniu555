# 🎮 对象池管理器测试操作指南（最终修正版）

## 📌 关键信息

| 参数 | 应该填什么 | 说明 |
|------|-----------|------|
| **Outer** | **Get Game Instance** | 对象池的"拥有者"，用 GameInstance |
| **Class** | **Actor Object Pool** | 在列表中找到它（现在应该有了） |

---

## 测试步骤

### 步骤1：打开关卡蓝图
1. 打开 `CombatTestMap`
2. 点击 **蓝图** → **打开关卡蓝图**

---

### 测试2：创建对象池（修正方法）

#### 创建特效对象池
1. 添加 **键盘1** 键事件
2. 右键搜索 **Construct Object from Class**
3. 设置参数：
   - **Class**: **Actor Object Pool**（下拉列表中选择）
   - **Outer**: **Get Game Instance** ← 关键！用这个！
4. 返回值右键 → **Promote to Variable**，命名为 `VFXPool`
5. 从 `VFXPool` 拖动，调用 **Initialize Actor Pool**：
   - **Pool Size**: `10`
   - **Actor Class**: 选择任意 Actor 蓝图（如 `CombatDummy`）
   - **World**: **Get World**
6. **Print String**: `"对象池创建成功"`

**图示**：
```
键盘1 ──► Construct Object from Class
            ├─ Class: Actor Object Pool
            ├─ Outer: Get Game Instance  ← 用这个！
            └─ Return Value ──► Set VFXPool
```

---

### 测试3：注册到管理器

1. 继续上面，从 `VFXPool`：
2. **Get Game Instance** → **Get Object Pool Manager**
3. **Register Pool**：
   - **Pool Type**: `VisualEffect`
   - **Pool**: `VFXPool`
4. **Print String**: `"注册成功"`

---

### 测试4：从池获取Actor

1. 添加 **键盘2** 键事件
2. 从 `VFXPool` 拖动，调用 **Get Actor**
3. 返回值右键 → **Promote to Variable**，命名为 `SpawnedActor`
4. **Branch**（分支）检查 **Is Valid**（SpawnedActor是否有效）
5. 如果有效（True）：
   - **Set Actor Hidden In Game**: `false`（显示Actor）
   - **Set Actor Location**: 
     - **Get Player Pawn** → **Get Actor Location**
     - **+** **Get Actor Forward Vector** × `100.0`
6. **Print String**: `"获取Actor成功"`

---

### 测试5：归还Actor到池

1. 添加 **键盘3** 键事件
2. 从 `VFXPool` 拖动，调用 **Return Actor**
   - **Actor**: `SpawnedActor`
3. **Print String**: `"归还成功"`

---

### 测试6：查看池状态

1. 添加 **键盘4** 键事件
2. 从 `VFXPool` 拖动：
   - **Get Available Count** → 打印 "可用: X"
   - **Get In Use Count** → 打印 "使用中: X"

---

## ✅ 测试检查清单

- [ ] 能创建 Actor Object Pool（Outer 用 Get Game Instance）
- [ ] 能初始化对象池
- [ ] 能注册到 Object Pool Manager
- [ ] Get Actor 能获取到 Actor
- [ ] Return Actor 能归还 Actor
- [ ] Actor 能正确显示/隐藏
- [ ] 数量统计正确

---

## 🔍 如果还有错误

### 错误："无法构建 ActorObjectPool 对象"
**解决**：
1. 确保 Outer 用的是 **Get Game Instance**
2. 确保 Class 下拉菜单选择了 **Actor Object Pool**

### 错误："Outer 类型不匹配"
**解决**：
- 不要用 Get World 作为 Outer
- 用 **Get Game Instance**

### 找不到 Actor Object Pool 类
**解决**：
1. 编译是否成功？
2. 重启编辑器试试
3. 在 Class 下拉菜单中搜索 "Actor Object"

---

**请按此修正版测试！** 如果还有问题请截图给我 😊
