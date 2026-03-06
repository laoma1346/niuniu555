# 🎮 角色系统测试操作指南（含英文原名对照）

## 第二阶段：角色基础与移动

---

## 步骤2.1：角色移动组件测试

### 添加组件

1. 打开玩家角色蓝图（如 `CombatCharacter`）
2. 在 **组件(Components)** 面板点击 **添加组件(Add Component)**
3. 搜索英文：**Advanced Movement** 或 **AdvancedMovementComponent**
4. 选择 **Advanced Movement Component**
5. 保存并编译

**节点对照**：
- 中文：`添加组件` / 英文：`Add Component`
- 作用：给Actor添加功能组件

---

### 测试1：切换奔跑/行走

#### 设置奔跑状态
1. 打开关卡蓝图
2. 添加 **键盘左Shift** 键事件（Key: Left Shift）
3. **Get Player Character**（获取玩家角色）
4. 搜索英文：**Get Component** → **Get Component by Class**
   - **Component Class**: **Advanced Movement Component**
5. 从返回值拖动，调用 **Set Running**
   - **bRunning**: `true`
6. **Print String**: `"开始奔跑"`

#### 松开停止奔跑
1. 从 **Left Shift** 的 **Released** 引脚
2. 同样获取 **Advanced Movement Component**
3. 调用 **Set Running**
   - **bRunning**: `false`
4. **Print String**: `"停止奔跑"`

---

### 测试2：查看当前速度

1. 添加 **键盘1** 键事件
2. 获取玩家的 **Advanced Movement Component**
3. 调用 **Get Current Speed**
4. **Print String**: `"当前速度: " + 返回值`

---

### 测试3：查看移动状态

1. 添加 **键盘2** 键事件
2. 获取 **Advanced Movement Component**
3. 调用 **Get Movement State**
4. **Print String**: `"移动状态: " + 返回值`

**状态说明**：
- Idle = 静止
- Walking = 行走
- Running = 奔跑
- Stopping = 急停

---

### 测试4：急停测试

1. 添加 **键盘空格** 键事件
2. 获取 **Advanced Movement Component**
3. 调用 **Perform Stop**（执行急停）
4. **Print String**: `"急停！"`

---

### 测试5：查看是否在移动

1. 在 **Event Tick** 中：
2. 获取 **Advanced Movement Component**
3. 调用 **Is Moving**
4. 如果是 **True**，打印 `"正在移动"`

---

## 📋 测试检查清单

### 基础功能
- [ ] 能添加 Advanced Movement Component
- [ ] 按住 Shift 切换到奔跑状态
- [ ] 松开 Shift 回到行走状态
- [ ] 速度值随奔跑/行走变化
- [ ] 急停功能正常工作
- [ ] 转向平滑（有插值效果）

### 参数调整（在组件Details面板）
- [ ] **Walk Speed**: 行走速度（默认300）
- [ ] **Run Speed**: 奔跑速度（默认600）
- [ ] **Rotation Speed**: 转向速度（默认10）
- [ ] **Acceleration**: 加速度（默认1500）
- [ ] **Deceleration**: 减速度（默认800）

---

## 🔍 常见问题与节点对照

### 问题1：找不到 Advanced Movement Component
**解决**：
- 搜索英文：**AdvancedMovementComponent**
- 或搜索：**Advanced Movement**

### 问题2：Get Component by Class 找不到
**解决**：
- 搜索英文：**Get Component by Class**
- 作用：从Actor获取指定类型的组件
- 注意：需要先获取玩家角色（Get Player Character）

### 问题3：速度不变化
**解决**：
- 检查是否连接了 Set Running
- 检查 Walk Speed 和 Run Speed 是否设置正确
- 确保组件已添加到角色蓝图并编译

### 问题4：转向太快/太慢
**解决**：
- 调整 **Rotation Speed** 参数
- 值越大转向越快，值越小越平滑

---

## ✅ 测试成功标志

- [x] Shift键能切换奔跑/行走
- [x] 速度值正确显示
- [x] 移动状态正确切换
- [x] 急停功能正常
- [x] 转向平滑自然

**角色移动组件测试完成！** 🎉

---

## 步骤2.2：摄像机系统测试

### 添加摄像机控制组件

1. 打开玩家角色蓝图
2. **添加组件(Add Component)**
3. 搜索英文：**Camera Controller** 或 **CameraControllerComponent**
4. 选择 **Camera Controller Component**
5. 保存并编译

---

### 测试1：切换锁定视角

#### 按Q键切换锁定
1. 打开关卡蓝图
2. 添加 **键盘Q** 键事件
3. **Get Player Character**（获取玩家角色）
4. **Get Component by Class** → **Camera Controller Component**
5. 调用 **Toggle Lock On**（切换锁定）
6. **Print String**: `"切换锁定视角"`

---

### 测试2：查看当前摄像机模式

1. 添加 **键盘E** 键事件
2. 获取 **Camera Controller Component**
3. 调用 **Get Camera Mode**
4. **Print String**: `"摄像机模式: " + 返回值`

**模式说明**：
- Free = 自由视角
- LockOn = 锁定视角
- Fixed = 固定视角

---

### 测试3：查看是否锁定

1. 添加 **键盘R** 键事件
2. 获取 **Camera Controller Component**
3. 调用 **Is Lock On**
4. **Print String**: `"是否锁定: " + 返回值`

---

### 测试4：获取锁定目标

1. 添加 **键盘T** 键事件
2. 获取 **Camera Controller Component**
3. 调用 **Get Lock On Target**
4. **Is Valid**（检查是否有效）
5. 如果有效，打印 `"锁定目标: " + 目标名称`

---

### 测试5：鼠标控制摄像机（自由视角）

1. 在关卡蓝图的 **Event Tick** 中：
2. 获取 **Camera Controller Component**
3. 调用 **Add Camera Input**
   - **Yaw**: **Get Input Mouse Delta** 的 X × 0.5
   - **Pitch**: **Get Input Mouse Delta** 的 Y × 0.5
4. 这样可以用鼠标控制摄像机旋转

**节点对照**：
- 中文：`获取输入鼠标增量` / 英文：`Get Input Mouse Delta`
- 作用：获取鼠标移动的偏移量

---

### 测试6：清除锁定

1. 添加 **键盘F** 键事件
2. 获取 **Camera Controller Component**
3. 调用 **Clear Lock On Target**
4. 调用 **Set Camera Mode** → **Free**
5. **Print String**: `"清除锁定"`

---

## 📋 摄像机系统测试检查清单

- [ ] 能添加 Camera Controller Component
- [ ] Q键能切换锁定/自由视角
- [ ] 锁定模式下角色朝向目标
- [ ] 能正确显示当前摄像机模式
- [ ] 能获取锁定目标
- [ ] 清除锁定后回到自由视角

---

## 🔍 常见问题

### 问题1：找不到锁定目标
**解决**：
- 确保场景中有带"Enemy"标签的Actor
- 或Actor名称包含"Enemy"
- 检查锁定距离是否足够

### 问题2：锁定后视角不跟随
**解决**：
- 检查组件Tick是否启用
- 检查LockOnRotationSpeed参数

### 问题3：鼠标控制不灵敏
**解决**：
- 调整AddCameraInput的乘数
- 或调整FreeLookSpeed参数

---

## ✅ 摄像机系统测试完成标志

- [x] Q键能切换锁定视角
- [x] 锁定后角色自动朝向目标
- [x] 能正确显示摄像机模式
- [x] 能获取和清除锁定目标
- [x] 自由视角下能用鼠标控制

**摄像机系统测试完成！** 🎉

---

## 步骤2.3：闪避系统测试

### 添加闪避组件

1. 打开玩家角色蓝图
2. **添加组件(Add Component)**
3. 搜索英文：**Dodge** 或 **DodgeComponent**
4. 选择 **Dodge Component**
5. 保存并编译

---

### 测试1：执行闪避

#### 按空格键闪避
1. 打开关卡蓝图
2. 添加 **键盘空格** 键事件
3. **Get Player Character**
4. **Get Component by Class** → **Dodge Component**
5. 调用 **Perform Dodge**
   - **Direction**: **Auto**（自动）
6. **Print String**: `"闪避！"`

**说明**：体力足够时执行闪避，体力不足时无法闪避

---

### 测试2：查看体力

1. 添加 **键盘1** 键事件
2. 获取 **Dodge Component**
3. 调用 **Get Stamina Percent**
4. **Print String**: `"体力: " + 返回值 × 100 + "%"`

---

### 测试3：查看是否无敌

1. 在 **Event Tick** 中：
2. 获取 **Dodge Component**
3. 调用 **Is Invincible**
4. 如果是 **True**，打印 `"无敌中！"`

---

### 测试4：查看闪避状态

1. 添加 **键盘2** 键事件
2. 获取 **Dodge Component**
3. 调用 **Get Dodge State**
4. **Print String**: `"状态: " + 返回值`

**状态说明**：
- Ready = 准备就绪
- Dodging = 闪避中
- Cooldown = 冷却中
- Exhausted = 体力耗尽

---

### 测试5：绑定闪避事件

#### 闪避开始事件
1. 在 **Event Begin Play** 中：
2. 获取 **Dodge Component**
3. 选择 **Assign On Dodge Started**
4. 连接 **Print String**: `"闪避开始！"`

#### 无敌帧开始事件
1. 继续上面，从 **Dodge Component**
2. 选择 **Assign On Invincibility Started**
3. 连接 **Print String**: `"无敌开始！"`

#### 无敌帧结束事件
1. 选择 **Assign On Invincibility Ended**
2. 连接 **Print String**: `"无敌结束！"`

#### 体力变化事件
1. 选择 **Assign On Stamina Changed**
2. 连接 **Print String**: `"体力变化: " + Current Stamina`

---

### 测试6：连续闪避测试

1. 快速多次按 **空格键**
2. 观察体力消耗
3. 体力耗尽后无法闪避
4. 等待体力恢复（自动恢复）

---

## 📋 闪避系统测试检查清单

- [ ] 能添加 Dodge Component
- [ ] 空格键能执行闪避
- [ ] 闪避消耗体力
- [ ] 体力不足时无法闪避
- [ ] 能正确显示体力百分比
- [ ] 无敌帧期间 Is Invincible 返回 True
- [ ] 闪避状态正确切换
- [ ] 体力自动恢复
- [ ] 闪避事件正确触发

---

## 🔍 常见问题

### 问题1：无法闪避
**解决**：
- 检查体力是否足够（默认需要25点）
- 检查是否在冷却中
- 检查 MaxStamina 是否设置正确

### 问题2：闪避没有位移
**解决**：
- 检查 Dodge Distance 参数
- 确保角色有 CharacterMovementComponent

### 问题3：无敌帧太短/太长
**解决**：
- 调整 Invincibility Duration 参数
- 调整 Invincibility Start Time 参数

---

## ✅ 闪避系统测试完成标志

- [x] 空格键能执行闪避
- [x] 闪避消耗体力
- [x] 体力不足时无法闪避
- [x] 体力自动恢复
- [x] 无敌帧正常工作
- [x] 闪避事件正确触发

**闪避系统测试完成！** 🎉

---

## 步骤2.4：钩索系统测试

### 添加钩索组件

1. 打开玩家角色蓝图
2. **添加组件(Add Component)**
3. 搜索英文：**Hookshot** 或 **HookshotComponent**
4. 选择 **Hookshot Component**
5. 保存并编译

---

### 测试1：发射钩索

#### 按F键发射
1. 打开关卡蓝图
2. 添加 **键盘F** 键事件
3. **Get Player Character**
4. **Get Component by Class** → **Hookshot Component**
5. 调用 **Fire Hookshot**
6. **Print String**: `"钩索发射！"`

**说明**：角色会发射钩索到前方，命中后会拉回角色

---

### 测试2：开始/结束瞄准

#### 按住鼠标右键瞄准
1. 添加 **鼠标右键** 事件（Pressed）
2. 获取 **Hookshot Component**
3. 调用 **Start Aiming**
4. **Print String**: `"开始瞄准"`

#### 松开结束瞄准
1. 从 **鼠标右键** 的 **Released** 引脚
2. 调用 **End Aiming**
3. **Print String**: `"结束瞄准"`

---

### 测试3：查看钩索状态

1. 添加 **键盘G** 键事件
2. 获取 **Hookshot Component**
3. 调用 **Get Hookshot State**
4. **Print String**: `"钩索状态: " + 返回值`

**状态说明**：
- Ready = 准备就绪
- Aiming = 瞄准中
- Shooting = 发射中
- Pulling = 拉回中
- Cooldown = 冷却中

---

### 测试4：查看瞄准结果

1. 在 **Event Tick** 中（需要正在瞄准）：
2. 获取 **Hookshot Component**
3. 调用 **Get Aim Result**
4. 从返回值获取 **bHit**
5. 如果命中，打印 `"瞄准命中: " + Hit Actor`

---

### 测试5：绑定钩索事件

#### 钩索发射事件
1. 在 **Event Begin Play** 中：
2. 获取 **Hookshot Component**
3. 选择 **Assign On Hookshot Fired**
4. 连接 **Print String**: `"钩索发射！"`

#### 钩索命中事件
1. 选择 **Assign On Hookshot Hit**
2. 连接 **Print String**: `"钩索命中！"`

#### 拉回完成事件
1. 选择 **Assign On Hookshot Completed**
2. 连接 **Print String**: `"拉回完成！"`

---

### 测试6：取消钩索

1. 添加 **键盘C** 键事件
2. 获取 **Hookshot Component**
3. 调用 **Cancel Hookshot**
4. **Print String**: `"取消钩索"`

---

## 📋 钩索系统测试检查清单

- [ ] 能添加 Hookshot Component
- [ ] F键能发射钩索
- [ ] 右键能开始/结束瞄准
- [ ] 钩索命中后拉回角色
- [ ] 能正确显示钩索状态
- [ ] 瞄准结果正确
- [ ] 钩索事件正确触发
- [ ] 能取消钩索

---

## 🔍 常见问题

### 问题1：钩索没有发射
**解决**：
- 检查是否处于冷却中
- 检查前方是否有可抓取的目标
- 检查 MaxRange 参数

### 问题2：钩索发射但没有拉回
**解决**：
- 检查 FireSpeed 和 PullSpeed 参数
- 检查 CooldownTime 参数

### 问题3：瞄准没有反应
**解决**：
- 确保调用了 Start Aiming
- 检查 AimLineMaxLength 参数

---

## ✅ 钩索系统测试完成标志

- [x] F键能发射钩索
- [x] 右键能瞄准
- [x] 钩索命中后拉回角色
- [x] 状态正确切换
- [x] 事件正确触发
- [x] 能取消钩索

**钩索系统测试完成！** 🎉

---

## 🎉 第二阶段全部完成！

恭喜！角色基础与移动系统全部开发完成！

### 已完成功能：
1. ✅ 角色移动组件（奔跑/急停/转向）
2. ✅ 摄像机系统（自由/锁定视角）
3. ✅ 闪避系统（无敌帧/体力/取消后摇）
4. ✅ 钩索系统（瞄准/位移/派生攻击）

---

## 下一步预告（第三阶段：战斗核心系统）

### 2.2 摄像机系统
- 自由视角/锁定视角切换
- 摄像机朝向解耦

### 2.3 闪避系统
- 无敌帧
- 体力消耗
- 取消后摇

### 2.4 钩索系统
- 瞄准射线检测
- 钩索位移
- 派生攻击
