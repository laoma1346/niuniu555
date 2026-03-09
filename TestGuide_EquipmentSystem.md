# 装备系统测试指南

## 目录
1. [测试前准备](#测试前准备)
2. [创建装备数据资产](#步骤1创建装备数据资产)
3. [创建装备蓝图类](#步骤2创建装备蓝图类)
4. [给玩家添加组件](#步骤3给玩家添加装备组件)
5. [测试装备生成](#步骤4测试装备生成)
6. [测试背包功能](#步骤5测试背包功能)
7. [测试装备穿戴](#步骤6测试装备穿戴)
8. [测试属性应用](#步骤7测试属性应用)
9. [日志搜索指南](#日志搜索指南)
10. [常见问题排查](#常见问题排查)

---

## 测试前准备

### 1. 确认编译成功

在编译之前，请确保：
- 关闭UE编辑器
- 使用以下命令编译项目：
```bash
E:\UE_5.6\UE_5.6\Engine\Build\BatchFiles\Build.bat niuniu555Editor Win64 Development -Project="C:\Users\Win10\Desktop\niuniu555\niuniu555.uproject" -Progress
```
- 确认没有编译错误

### 2. 确认关键文件存在

检查以下文件是否存在：
- `Source/niuniu555/EquipmentSystem/EquipmentTypes.h`
- `Source/niuniu555/EquipmentSystem/EquipmentDataAsset.h`
- `Source/niuniu555/EquipmentSystem/EquipmentBase.h`
- `Source/niuniu555/EquipmentSystem/EquipmentManagerComponent.h`
- `Source/niuniu555/EquipmentSystem/InventoryComponent.h`

### 3. 重启UE编辑器

编译完成后，重新打开UE编辑器以确保新组件正确加载。

---

## 步骤1：创建装备数据资产

### 操作步骤

1. **打开内容浏览器**
   - 在UE编辑器中，点击内容浏览器窗口
   - 选择一个文件夹（如 `Content/Weapons`）

2. **创建数据资产**
   - 右键点击内容浏览器的空白区域
   - 选择菜单：`杂项(Miscellaneous)` → `数据资产(Data Asset)`
   - 【注意】选择白色文档图标，不是蓝色蓝图类图标

3. **选择父类**
   - 在弹出的窗口中，搜索 `EquipmentDataAsset`
   - 选择 `EquipmentDataAsset` 类
   - 点击 `选择` 按钮

4. **命名资产**
   - 输入名称：`DA_TestSword`
   - 按回车确认

### 配置装备属性

双击 `DA_TestSword` 打开详情面板，按以下配置：

#### 基础信息
```
基础ID: TestSword_001
装备名称: 测试铁剑
装备描述: 一把普通的测试用剑，用于验证装备系统功能
```

#### 装备类型
```
装备类型: Weapon（武器）
装备槽位: MainHand（主手）
```

#### 装备品质
```
基础品质: Common（普通）
```

#### 属性（基础属性）
```
攻击力: 10
防御力: 0
最大生命值: 0
暴击率: 0.05
暴击伤害: 0.2
移动速度: 0
金币获取率: 0
```

#### 耐久度
```
最大耐久度: 100
```

#### 经济
```
装备价值: 100
修理成本系数: 0.1
分解材料数量: 5
```

#### 套装（可选）
```
套装ID: （留空，非套装装备）
套装名称: （留空）
```

#### 词条（预留）
```
词条池ID: （留空）
最大词条数量: 3
前缀概率: 0.5
后缀概率: 0.5
```

#### 外观（可选）
```
装备模型: （可以留空，或选择一个骨骼网格体）
装备材质: （可以留空）
装备图标: （可以留空）
```

#### 音效（可选）
```
装备音效: （可以留空）
卸下音效: （可以留空）
```

### 保存资产

- 点击 `保存` 按钮（Ctrl+S）
- 确认资产保存成功

---

## 步骤2：创建装备蓝图类

### 操作步骤

1. **创建蓝图类**
   - 在内容浏览器中右键点击空白区域
   - 选择 `创建蓝图类`

2. **选择父类**
   - 点击 `所有类` 按钮
   - 在搜索框中输入 `EquipmentBase`
   - 选择 `EquipmentBase`（不是 EquipmentDataAsset）
   - 点击 `选择` 按钮

3. **命名蓝图**
   - 输入名称：`BP_TestEquipment`
   - 按回车确认

4. **打开蓝图**
   - 双击 `BP_TestEquipment` 打开蓝图编辑器

### 查看蓝图组件

在蓝图编辑器的组件面板中，确认以下组件存在：
- **DefaultSceneRoot**（默认根组件）
- **CollisionComponent**（碰撞组件，胶囊体）
- **EquipmentMeshComponent**（装备网格组件，骨骼网格体）

### 查看可配置属性

点击 `类默认值`（Class Defaults）按钮，查看可配置属性：

```
装备数据（Equipment Data）:
└── Equipment Data: None（可以在运行时设置）

装备实例（Instance Data）:
└── 运行时生成的数据，不需要手动设置

组件:
├── Equipment Mesh Component
└── Collision Component
```

### 保存蓝图

- 点击 `编译` 按钮
- 点击 `保存` 按钮

---

## 步骤3：给玩家添加装备组件

### 操作步骤

1. **打开玩家角色蓝图**
   - 找到玩家角色蓝图（如 `BP_PlayerCharacter` 或 `BP_CombatCharacter`）
   - 双击打开蓝图编辑器

2. **添加背包组件**
   - 在组件面板中，点击 `+ 添加` 按钮
   - 在搜索框中输入 `Inventory` 或 `背包`
   - 选择 `背包组件 (Inventory Component)`
   - 【注意】如果搜索不到，尝试以下方法：
     - 直接在组件列表中滚动查找
     - 重启UE编辑器
     - 确认C++代码已编译成功
   - 添加后，在组件面板中重命名为 `InventoryComp`

3. **添加装备管理器组件**
   - 再次点击 `+ 添加` 按钮
   - 在搜索框中输入 `Equipment` 或 `装备`
   - 选择 `装备管理器 (Equipment Manager Component)`
   - 添加后，在组件面板中重命名为 `EquipmentManagerComp`

4. **确认属性面板**
   - 选中 `InventoryComp` 组件
   - 在详情面板中，确认 `Inventory Capacity`（背包容量）默认为 20
   - 可以根据需要调整背包容量

5. **编译并保存**
   - 点击 `编译` 按钮
   - 点击 `保存` 按钮

### 验证组件添加成功

- 在组件面板中确认两个组件都存在：
  - `InventoryComp`（继承自C++，显示为蓝色图标）
  - `EquipmentManagerComp`（继承自C++，显示为蓝色图标）

---

## 步骤4：测试装备生成

### 创建测试蓝图

1. **打开关卡蓝图**
   - 点击菜单栏：`蓝图` → `打开关卡蓝图`

2. **创建自定义事件**
   - 在事件图表中右键点击空白区域
   - 搜索 `添加自定义事件`
   - 命名为 `TestSpawnEquipment`

3. **连接蓝图节点**

按照以下顺序连接节点：

```
[TestSpawnEquipment] (自定义事件)
    │
    ▼
[Get Player Pawn] (获取玩家) ───────┬──────────────────────┐
    │ Player Pawn                   │                      │
    ▼                               ▼                      ▼
[Get Actor Location]    [Get Actor Rotation]    [Is Valid]
    │ Return Value          │ Return Value          │ Input Object
    │                       │                       │
    └───────────────────────┼───────────────────────┘
                            │
                            ▼
[Spawn Equipment From Data Asset] (调用静态函数)
    ├── World Context: 【无需连接或连接 Get Player Pawn】
    │                  【注意：此参数使用meta标记自动获取】
    ├── Data Asset: 选择 DA_TestSword
    ├── Spawn Location: 连接 Get Actor Location 的 Return Value
    ├── Spawn Rotation: 连接 Get Actor Rotation 的 Return Value
    └── Return Value: 右键点击，选择 `提升为变量`，命名为 `SpawnedEquipment`
                            │
                            ▼
                    [Is Valid] (检查有效性)
                            │
                            ▼
                    [Branch] (分支判断)
                            │
            ┌───────────────┴───────────────┐
            ▼                               ▼
    [Print String] "成功！"        [Print String] "失败！"
```

#### 重要说明

1. **World Context 参数**：
   - 如果节点显示 `World Context` 引脚，可以留空或连接 `Get Player Pawn`
   - 如果节点隐藏了此引脚，说明使用了 `meta = (WorldContext = "...")`，无需手动连接
   - 此参数用于获取 World 上下文以生成 Actor

2. **Spawn Rotation**：
   - 必须从 `Get Actor Rotation` 获取并连接
   - 如果不连接，装备可能生成在错误的方向

3. **有效性检查**：
   - 使用 `Is Valid` 节点检查返回的装备是否有效
   - 如果为 nullptr，说明生成失败

### 运行测试

1. 点击 `编译` 按钮
2. 关闭关卡蓝图
3. 点击 `保存` 按钮保存关卡
4. 点击 `播放` 按钮运行游戏
5. 【测试方法A - 控制台命令】
   - 按 `~` 键（波浪键）打开控制台
   - 输入 `ce TestSpawnEquipment` 并按回车
   - 【注意】如果提示 "CE command wasn't processed"，使用方法B
6. 【测试方法B - 按键绑定】
   - 在关卡蓝图中添加 `按键事件 T`（或其他按键）
   - 连接到 `TestSpawnEquipment` 事件
   - 在游戏中按 T 键测试
7. 查看屏幕上的打印信息和输出日志

### 预期结果

**成功时**：
- 屏幕上显示：`装备生成成功！` 或 `成功！`
- 输出日志中出现：
  ```
  [EquipmentBase] 开始生成装备：测试铁剑 在位置 X=... Y=... Z=...
  [EquipmentBase] 成功生成装备：测试铁剑
  [EquipmentBase] 装备初始化完成：测试铁剑 (品质: 0)
  ```

**失败时（无崩溃）**：
- 屏幕上显示：`装备生成失败！` 或 `失败！`
- 检查日志中的错误信息
- 常见问题见 [常见问题排查](#常见问题排查)

**崩溃时**：
- 立即查看日志文件（`Saved/Logs/niuniu555.log`）
- 检查是否有断言失败或空指针访问
- 确认 WorldContext 和 DataAsset 参数有效
- 重启编辑器后重试

---

## 步骤5：测试背包功能

### 修改测试蓝图

在步骤4的蓝图基础上继续添加节点：

```
[Spawn Equipment From Data Asset]
    │
    ▼
[Get Player Pawn] (重新获取玩家)
    │
    ▼
[Get Component by Class] (获取组件)
    ├── Component Class: 选择 Inventory Component
    └── Return Value: 右键点击，选择 `提升为变量`，命名为 `InventoryComp`
    │
    ▼
[Add Item] (调用函数)
    ├── Target: InventoryComp
    ├── Item: SpawnedEquipment（步骤4中的变量）
    └── Return Value: 连接到 [Branch] 分支节点
    │
    ▼
[Branch] (分支判断)
    ├── True → [Print String] "装备添加成功！"
    └── False → [Print String] "背包已满！"
```

### 运行测试

1. 重新运行 `ce TestSpawnEquipment`
2. 查看屏幕输出

### 预期结果

**成功时**：
- 屏幕上显示：`装备添加成功！`
- 输出日志中出现：
  ```
  [Inventory] 添加装备成功: 测试铁剑 到槽位 0
  ```

**失败时（背包已满）**：
- 屏幕上显示：`背包已满！`
- 需要清理背包或增加背包容量

---

## 步骤6：测试装备穿戴

### 修改测试蓝图

在步骤5的蓝图基础上继续添加节点：

```
[Add Item 成功后] (Branch的True分支)
    │
    ▼
[Get Player Pawn] (重新获取玩家)
    │
    ▼
[Get Component by Class] (获取组件)
    ├── Component Class: 选择 Equipment Manager Component
    └── Return Value: 右键点击，选择 `提升为变量`，命名为 `EquipmentManagerComp`
    │
    ▼
[Equip Item] (调用函数)
    ├── Target: EquipmentManagerComp
    ├── Equipment: SpawnedEquipment
    ├── Slot: MainHand (主手)
    └── Return Value: 连接到 [Branch] 分支节点
    │
    ▼
[Branch] (分支判断)
    ├── True → [Print String] "装备穿戴成功！"
    └── False → [Print String] "穿戴失败！"
```

### 运行测试

1. 重新运行 `ce TestSpawnEquipment`
2. 查看屏幕输出

### 预期结果

**成功时**：
- 屏幕上显示：`装备穿戴成功！`
- 输出日志中出现：
  ```
  [EquipmentBase] 测试铁剑 装备属性已应用到 BP_PlayerCharacter
  [Inventory] 装备成功: 测试铁剑 从背包槽位 0 装备到装备槽位 0
  ```

**失败时**：
- 检查装备槽位是否匹配（武器只能装备到MainHand）
- 检查装备是否已损坏（耐久度 <= 0）

---

## 步骤7：测试属性应用

### 创建属性监视蓝图

在玩家角色蓝图中添加调试节点：

1. 打开玩家角色蓝图
2. 切换到 `事件图表`
3. 添加以下节点：

```
[Event Tick] (事件Tick)
    │
    ▼
[Get Component by Class] (获取组件)
    ├── Component Class: 选择 Attribute Component
    │
    ▼
[Get Attack] (获取攻击力)
    │
    ▼
[Print String] (打印字符串)
    ├── In String: 连接 Format Text 节点
    └── Print to Screen: 勾选
    └── Print to Log: 勾选
```

### Format Text 节点配置

```
格式字符串: "当前攻击力: {Attack}"
Attack: 连接 Get Attack 的返回值	
```

### 测试属性变化

1. 运行游戏
2. 观察屏幕上的攻击力数值
3. 执行装备穿戴操作
4. 观察攻击力是否增加（基础10 + 装备10 = 20）

### 预期结果

**穿戴前**：
```
当前攻击力: 10.0
```

**穿戴后**：
```
当前攻击力: 20.0
```

**卸下后**：
```
当前攻击力: 10.0
```

---

## 日志搜索指南

### 如何查看输出日志

1. 点击菜单栏：`窗口` → `开发者工具` → `输出日志`
2. 在日志窗口的搜索框中输入关键词

### 装备系统日志类别

| 日志类别 | 用途 |
|----------|------|
| LogEquipmentSystem | 装备系统核心日志 |
| LogInventorySystem | 背包系统日志 |
| LogDropItem | 掉落物品日志 |

### 常用搜索关键词

#### 成功日志
```
搜索: 成功生成装备
结果: [EquipmentBase] 成功生成装备：测试铁剑

搜索: 装备初始化完成
结果: [EquipmentBase] 装备初始化完成：测试铁剑 (品质: 0)

搜索: 装备属性已应用
结果: [EquipmentBase] 测试铁剑 装备属性已应用到 BP_PlayerCharacter

搜索: 添加装备成功
结果: [Inventory] 添加装备成功: 测试铁剑 到槽位 0

搜索: 装备成功
结果: [Inventory] 装备成功: 测试铁剑 从背包槽位 0 装备到装备槽位 0
```

#### 错误日志
```
搜索: 没有属性组件
结果: [EquipmentBase] 目标 XXX 没有属性组件
解决: 给玩家添加 AttributeComponent

搜索: 没有背包组件
结果: [EquipmentDropItem] 玩家 XXX 没有背包组件
解决: 给玩家添加 InventoryComponent

搜索: 生成装备失败
结果: [EquipmentBase] 生成装备失败：WorldContext或DataAsset为空
解决: 检查输入参数

搜索: 开始生成装备
结果: [EquipmentBase] 开始生成装备：测试铁剑 在位置 X=... Y=... Z=...
说明: 生成流程开始，如果后面没有成功日志，可能在SpawnActor时崩溃

搜索: 初始化失败
结果: [EquipmentBase] 初始化失败：DataAsset为空
解决: 检查DataAsset是否有效

搜索: 装备失败
结果: [Inventory] 装备失败：无法装备到槽位 X
解决: 检查装备槽位是否匹配

搜索: 背包已满
结果: [Inventory] 添加装备失败：背包已满
解决: 清理背包或增加容量
```

---

## 常见问题排查

### 问题0：UE引擎崩溃（最严重）

**症状**：运行 `ce TestSpawnEquipment` 或调用生成函数时引擎突然崩溃退出

**常见原因**：
1. WorldContext 参数为 nullptr 或无效
2. DataAsset 为 nullptr 或包含无效数据
3. 组件未正确初始化就访问
4. 在构造函数中过早调用外观更新

**解决方案**：
1. 【检查】WorldContext 参数
   - 如果蓝图节点显示 `World Context` 引脚，必须连接有效的 Actor 或 World
   - 可以使用 `Get Player Pawn` 或 `Get Game Instance` 获取
   - 也可以尝试直接传入 `Get World` 的结果

2. 【检查】DataAsset 配置
   - 确认 DA_TestSword 已保存且没有空值字段
   - 确认 EquipmentName 不为空
   - 确认 Mesh/Material 引用有效（如果设置了的话）

3. 【延迟初始化】如果崩溃发生在 BeginPlay
   - 已修复代码中使用了延迟初始化（Timer）
   - 确保使用的是最新代码版本

4. 【查看日志】
   - 崩溃后查看 `Saved/Logs/niuniu555.log`
   - 搜索最后几条日志中的 "EquipmentBase" 相关内容
   - 查看是否有 "初始化失败" 或 "生成失败" 的提示

5. 【备用测试方法】如果控制台命令崩溃，尝试使用按键事件：
   ```
   [按键事件 T] -> [Spawn Equipment From Data Asset]
   ```
   直接在关卡蓝图中绑定到按键，而不是使用控制台命令

### 问题1：搜不到背包组件/装备管理器组件

**症状**：在添加组件时搜索不到 Inventory Component 或 Equipment Manager Component

**解决方案**：
1. 【确认】C++代码已编译成功（没有红色错误）
2. 【确认】UE编辑器已重启（关闭后重新打开）
3. 【尝试】直接在组件列表中滚动查找，不使用搜索框
4. 【检查】确认 `EquipmentManagerComponent.h` 中有 `BlueprintSpawnableComponent` 标记
5. 【检查】确认 `InventoryComponent.h` 中有 `BlueprintSpawnableComponent` 标记

### 问题2：属性没有变化

**症状**：穿戴装备后攻击力不变

**解决方案**：
1. 【检查】玩家角色是否有 AttributeComponent
2. 【检查】装备数据资产中的属性值是否大于0
3. 【检查】日志中是否有 "装备属性已应用" 字样
4. 【检查】是否打印的是基础值而非最终值

### 问题3：装备生成失败 / 节点连接问题

**症状**：Spawn Equipment From Data Asset 返回 None，或节点没有 World Context 引脚

**解决方案**：
1. 【检查】World Context 连接
   - 如果节点有 `World Context` 引脚，连接 `Get Player Pawn` 或 `Get World`
   - 如果节点隐藏了此引脚（使用 meta 标记），则无需连接
   - 【注意】不能留空，必须从有效的 UObject 获取

2. 【检查】Spawn Rotation 连接
   - 必须连接 `Get Actor Rotation` 节点
   - 不能直接输入默认值，必须通过节点连接

3. 【检查】Data Asset 是否有效
   - 确认已选择 DA_TestSword
   - 确认 DA_TestSword 已保存

4. 【查看】输出日志中的详细错误信息
   - 搜索 "生成装备失败" 或 "SpawnActor"

### 问题4：装备穿戴失败

**症状**：Equip Item 返回 False

**解决方案**：
1. 【检查】装备槽位是否匹配（武器只能装备到 MainHand）
2. 【检查】装备是否已损坏（Durability <= 0）
3. 【检查】装备是否已经装备（bIsEquipped = true）
4. 【检查】目标槽位是否已有其他装备（会触发替换逻辑）

### 问题5：背包添加失败

**症状**：Add Item 返回 False

**解决方案**：
1. 【检查】背包容量是否已满
2. 【检查】传入的装备实例是否为 nullptr
3. 【检查】装备实例是否有效

### 问题6：组件冲突

**症状**：有两个同名组件或组件无法添加

**解决方案**：
1. 【确认】不要重复添加继承的组件（灰色图标的是继承的）
2. 【确认】只添加白色图标的自定义组件
3. 【删除】如果有重复的组件，删除白色图标的那个

---

## 测试通过标准

| 测试项 | 通过标准 |
|--------|----------|
| 装备生成 | 能成功从 DataAsset 生成 Equipment Actor |
| 背包添加 | 装备能添加到背包并显示在物品列表中 |
| 装备穿戴 | 能成功装备到指定槽位，属性正确应用 |
| 装备卸下 | 卸下后属性正确移除，装备回到背包 |
| 耐久度消耗 | 战斗或使用消耗耐久度，耗尽自动卸下 |
| 套装效果 | 同套装装备数量正确统计 |

---

## 附加测试

### 测试装备卸下

```
[按键事件 P] (按下P键)
    │
    ▼
[Get Player Pawn]
    │
    ▼
[Get Component by Class] (Equipment Manager Component)
    │
    ▼
[Unequip Item] (调用函数)
    ├── Slot: MainHand
    └── Return Value: 连接到 [Branch]
    │
    ▼
[Branch]
    ├── True → [Print String] "卸下成功！"
    └── False → [Print String] "卸下失败！"
```

### 测试耐久度消耗

```
[按键事件 1] (按下数字键1)
    │
    ▼
[Get SpawnedEquipment] (获取装备变量)
    │
    ▼
[Consume Durability] (调用函数)
    ├── Amount: 10
    │
    ▼
[Print String]
    └── In String: 连接 Format Text "	"
```

### 测试修理装备

```
[按键事件 2] (按下数字键2)
    │
    ▼
[Get SpawnedEquipment]
    │
    ▼
[Repair Equipment] (调用函数)
    ├── Gold Amount: 50
    │
    ▼
[Print String]
    └── In String: "修理完成！"
```

---

*文档版本: 2026-03-09*  
*对应代码版本: Phase 5 - 装备系统*
