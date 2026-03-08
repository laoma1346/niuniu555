# Phase 4.2.1 EnemyAIController - 测试指南

**版本**: 1.0  
**日期**: 2026-03-07  
**功能**: 敌人AI控制器基类  
**依赖**: Phase 4.1 已完成（EnemyBase + 状态机）

---

## 📋 功能说明

EnemyAIController 是控制敌人AI行为的核心组件：
- ✅ 运行行为树（BehaviorTree）
- ✅ AI感知（视觉/听觉）
- ✅ 目标管理（发现/追击/丢失）
- ✅ 与状态机交互（自动切换Chase/Return状态）

---

## 🎯 测试准备

### 前置条件
- ✅ Phase 4.1 已完成（EnemyBase可正常工作）
- ✅ 有玩家角色（带"Player"标签）

### 创建测试资源

#### 1. 创建黑板（Blackboard）

**【中文操作】**
```
Step 1: 内容浏览器右键 → Artificial Intelligence（人工智能） → Blackboard（黑板）
Step 2: 命名为 "BB_Enemy"
Step 3: 双击打开黑板编辑器
Step 4: 添加以下Key（点击 New Key）：
```

**【黑板Key配置 - 关键步骤！】**

```
Key名称              Key类型        基类/默认值       说明
─────────────────────────────────────────────────────────────────
TargetActor         Object        Actor(必须！)     【攻击目标】玩家
HomeLocation        Vector        -                【出生点位置】
PatrolCenter        Vector        -                【巡逻中心】
PatrolRadius        Float         500.0            【巡逻半径】
AttackRange         Float         150.0            【攻击范围】
ChaseRange          Float         2000.0           【追击范围】
CanAttack           Bool          false            【能否攻击】
IsInCombat          Bool          false            【是否战斗】
```

**⚠️ UE5行为树陷阱：Object类型Key必须指定基类为Actor，否则Move To任务会过滤掉它！**

**【关键配置：TargetActor基类设置 - 必须正确！】**

**【中文操作步骤】**
```
Step 1: 点击 "New Key" 按钮
Step 2: Name: 输入 "TargetActor"
Step 3: Key Type: 选择 "Object"
Step 4: 【关键！】Base Class: 点击下拉选择 "Actor"
        ↓
        显示: Actor (继承自: UObject)
        ↓
        确定
```

**【错误配置】❌**
```
TargetActor:
├── Key Type: Object
└── Base Class: None  【❌ 错误！Move To会忽略这个Key】
```

**【正确配置】✅**
```
TargetActor:
├── Key Type: Object
└── Base Class: Actor 【✅ 正确！Move To可以识别并使用】
```

**【其他Key配置】**
```
Step 5: New Key → HomeLocation → Vector
Step 6: New Key → PatrolCenter → Vector  
Step 7: New Key → PatrolRadius → Float → Default Value: 500.0
Step 8: New Key → AttackRange → Float → Default Value: 150.0
Step 9: New Key → ChaseRange → Float → Default Value: 2000.0
Step 10: New Key → CanAttack → Bool → Default Value: false
Step 11: New Key → IsInCombat → Bool → Default Value: false
```

**【验证配置】**
```
黑板编辑器显示:
├── TargetActor (Object - Actor)  【注意显示 Actor】
├── HomeLocation (Vector)
├── PatrolCenter (Vector)
├── PatrolRadius (Float - 500.0)
├── AttackRange (Float - 150.0)
├── ChaseRange (Float - 2000.0)
├── CanAttack (Bool - false)
└── IsInCombat (Bool - false)
```

**【故障排除：Move To不工作】**
| 现象 | 原因 | 解决 |
|------|------|------|
| Move To任务执行但敌人不动 | TargetActor基类为None | 重新设置基类为Actor |
| Move To直接返回失败 | 导航网格未构建 | 按P键检查绿色区域 |
| Move To卡住 | 目标Actor不可移动 | 确保目标是玩家角色 |

**【保存】**
- 点击保存按钮（Ctrl+S）
- 关闭黑板编辑器

---

#### 2. 创建行为树（BehaviorTree）

**【中文操作】**
```
Step 1: 内容浏览器右键 → Artificial Intelligence（人工智能） → Behavior Tree（行为树）
Step 2: 命名为 "BT_Enemy"
Step 3: 双击打开行为树编辑器
Step 4: 在Root节点属性中设置 Blackboard Asset 为 "BB_Enemy"
```

**【简单测试行为树】**
```
Root
└── Selector（选择器）          【按顺序执行子节点，一个成功就返回】
    ├── Sequence（序列）          【优先级1：有目标就追击】
    │   ├── Decorator: Blackboard 条件判断
    │   │   └── Key: TargetActor, 判断: Is Set（已设置）
    │   └── Task: Move To（移动到）
    │       └── Blackboard Key: TargetActor
    │
    └── Sequence（序列）          【优先级2：无目标就巡逻】
        └── Task: Wait（等待）
            └── Wait Time: 2.0
```

**【保存】**
- 点击保存按钮
- 关闭行为树编辑器

---

## 🔧 配置敌人蓝图

### 3. 配置BP_TestEnemy的AI控制器

**【中文操作路径】** BP_TestEnemy蓝图编辑器 → Class Defaults（类默认值）

**【步骤】**
```
Step 1: 打开 BP_TestEnemy 蓝图
Step 2: 点击右上角 "Class Defaults" 按钮
Step 3: 在Details面板搜索 "AI"
Step 4: 找到 "AI Controller Class" 字段
Step 5: 下拉选择 "EnemyAIController"（如果没有，先确保C++已编译）
Step 6: 【重要】勾选 "Auto Possess AI" 设为 "Placed in World or Spawned"
```

**【验证配置】**
```
AI设置:
├── AI Controller Class: EnemyAIController    【已设置】
└── Auto Possess AI: Placed in World or Spawned  【已勾选】
```

---

### 4. 配置EnemyDataAsset关联行为树

**【中文操作】**
```
Step 1: 打开 DA_TestEnemy 数据资产
Step 2: 找到 "AI配置" 分类
Step 3: Behavior Tree: 选择 "BT_Enemy"      【拖拽或下拉选择】
Step 4: Blackboard Data: 选择 "BB_Enemy"    【拖拽或下拉选择】
Step 5: 保存（Ctrl+S）
```

**【验证】**
```
AI配置:
├── Behavior Tree: BT_Enemy    【已关联】
└── Blackboard Data: BB_Enemy  【已关联】
```

---

### 5. 编译并保存

**【操作】**
```
Step 1: 在 BP_TestEnemy 蓝图中点击 "Compile"（编译）
Step 2: 等待绿色对勾 ✅
Step 3: 点击 "Save"（保存）
Step 4: 关闭蓝图编辑器
```

---

## 🎮 场景测试

### 6. 放置敌人到场景

**【操作】**
```
Step 1: 将 BP_TestEnemy 拖拽到关卡
Step 2: 放置在玩家附近（5-10米内）
Step 3: 确保玩家角色有 "Player" 标签（EnemyAIController通过标签识别）
```

**【给玩家添加标签】**
```
如果是默认第三人称角色:
Step 1: 打开 BP_ThirdPersonCharacter
Step 2: 选中 "Mesh" 或 "Character" 根组件
Step 3: Details面板找到 "Tags"
Step 4: 点击 + 添加元素
Step 5: 输入 "Player"
Step 6: 编译保存
```

---

### 7. 运行测试

**【预期日志输出】**
```
【初始化阶段】
[EnemyAIController] EnemyAIController_C_0 BeginPlay
[EnemyAIController] EnemyAIController_C_0 OnPossess -> BP_TestEnemy
[EnemyAIController] EnemyAIController_C_0 从EnemyData加载BehaviorTree
[EnemyAIController] EnemyAIController_C_0 AI配置加载完成
[EnemyAIController] EnemyAIController_C_0 黑板Key初始化完成
[EnemyAIController] EnemyAIController_C_0 行为树启动

【运行时】（如果玩家进入视野）
[EnemyAIController] EnemyAIController_C_0 感知更新，检测到 1 个Actor
[EnemyAIController] EnemyAIController_C_0 发现目标: BP_ThirdPersonCharacter
[EnemyAIController] EnemyAIController_C_0 目标改变: BP_ThirdPersonCharacter
[EnemyBase] BP_TestEnemy 进入追击状态
[EnemyStateMachine] BP_TestEnemy - 状态转换: Idle -> Chase
```

---

## ✅ 功能检查清单

| 检查项 | 预期结果 | 验证方法 | ✓ |
|--------|----------|----------|---|
| AI控制器创建 | 日志显示 BeginPlay | 查看输出日志 | ⏳ |
| 行为树启动 | 日志显示 "行为树启动" | 查看输出日志 | ⏳ |
| 黑板初始化 | 日志显示 "黑板Key初始化完成" | 查看输出日志 | ⏳ |
| 感知检测 | 玩家进入视野后检测到 | 查看日志 "感知更新" | ⏳ |
| 目标设置 | 日志显示 "发现目标" | 查看输出日志 | ⏳ |
| 状态切换 | 进入Chase状态 | 头顶显示 State:Chase | ⏳ |
| 移动追击 | 敌人向玩家移动 | 观察敌人位置变化 | ⏳ |

---

## ⚠️ 故障排除

### 问题1：AI控制器没有创建

**症状**: 日志中没有 `[EnemyAIController]` 相关内容

**检查**:
```
1. BP_TestEnemy → Class Defaults → AI Controller Class 是否设为 EnemyAIController
2. Auto Possess AI 是否设为 "Placed in World or Spawned"
3. C++代码是否已编译（关闭UE5 → VS编译 → 重启UE5）
```

---

### 问题2：行为树未启动

**症状**: 日志中没有 "行为树启动"

**检查**:
```
1. DA_TestEnemy 是否关联了 Behavior Tree
2. BT_Enemy 是否关联了 Blackboard Asset
3. 黑板中是否配置了必需的Key（TargetActor, HomeLocation等）
```

---

### 问题3：无法发现玩家

**症状**: 日志中没有 "发现目标"

**检查**:
```
1. 玩家角色是否有 "Player" 标签
2. 玩家是否在感知范围内（默认1500单位）
3. 感知配置中是否启用了检测敌人（Detect Enemies = true）
4. 是否有视线遮挡（需要LOS检测）
```

**调试方法**:
```
在控制台输入（游戏中按 `~`）:
ai.debug.perception 1          【显示感知调试信息】
show debug perception          【显示感知范围】
```

---

### 问题4：DA_TestEnemy看不到AI配置字段

**症状**: Details面板没有"AI配置"分类，或BehaviorTree/BlackboardData字段缺失

**检查**:
```
1. C++代码是否已编译（关闭UE5 → VS编译 → 重启UE5）
2. 面板过滤器是否清空（Details面板搜索栏）
3. 尝试右键DA_TestEnemy → Refresh（刷新）
```

**验证字段存在**:
```
打开 Source/niuniu555/EnemySystem/EnemyDataAsset.h
检查是否有:
├── UPROPERTY(EditAnywhere, Category="AI配置")
│   TObjectPtr<UBehaviorTree> BehaviorTree;
│
└── UPROPERTY(EditAnywhere, Category="AI配置")
    TObjectPtr<UBlackboardData> BlackboardData;
```

**强制刷新**:
```
Step 1: 删除 Intermediate 文件夹中的缓存
Step 2: 重新编译项目
Step 3: 重启UE5
Step 4: 重新打开DA_TestEnemy
```

---

### 问题5：发现目标但不移动

**症状**: 发现目标但State不变，敌人不动，或Move To任务执行但无效果

**检查**:
```
1. 行为树中是否有 Move To 任务
2. 导航网格（NavMesh）是否已构建
3. 检查输出日志是否有导航错误
4. 【关键】黑板TargetActor的Base Class是否为Actor
```

**验证黑板配置**:
```
打开 BB_Enemy:
├── TargetActor
│   ├── Key Type: Object
│   └── Base Class: Actor  【必须显示Actor，不是None！】
```

**修复基类**:
```
Step 1: 打开BB_Enemy
Step 2: 选中TargetActor
Step 3: Details面板找到Base Class
Step 4: 从None改为Actor
Step 5: 保存黑板
Step 6: 重启行为树（停止游戏再开始）
```

**构建导航网格**:
```
Step 1: 关卡编辑器 → 放置 → 导航网格边界体积（Nav Mesh Bounds Volume）
Step 2: 拖拽覆盖整个关卡
Step 3: 工具栏 → 构建（Build） → 构建路径（Build Paths）
Step 4: 按 P 键查看导航网格（绿色区域）
```

---

## 📋 关键节点速查（中文注释）

| 节点路径 | 中文说明 | 用途 |
|----------|----------|------|
| BT_Enemy | 【行为树资产】 | 控制AI行为逻辑 |
| BB_Enemy | 【黑板资产】 | AI内存数据存储 |
| TargetActor | 【攻击目标】 | 当前追击的玩家 |
| HomeLocation | 【出生点】 | 巡逻中心和返回点 |
| PatrolRadius | 【巡逻半径】 | 距离出生点多远开始返回 |
| EnemyAIController | 【AI控制器】 | 运行行为树，管理感知和目标 |

---

## 🎯 下一步

完成此测试后，继续开发：
- **4.2.2 AIPerceptionComponent**: 配置视觉/听觉感知细节
- **4.2.3 BTTask_Patrol**: 创建巡逻任务节点
- **4.2.4 BTTask_ChasePlayer**: 创建追击任务节点
- **4.2.5 BTTask_Attack**: 创建攻击任务节点

---

*测试指南版本: 1.2*  
*最后更新: 2026-03-07*  
*更新内容:*
- *v1.2: 修复感知组件初始化问题（显式创建AIPerceptionComponent）*
- *v1.2: 修复HomeLocation初始化（添加容错和详细日志）*
- *v1.1: 添加黑板Key基类配置详细说明（UE5 Move To陷阱）*
- *v1.0: 初始版本*
