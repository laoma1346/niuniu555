# 修复 InventoryComp 为 None 的问题

## 问题
蓝图运行时错误："尝试读取Lvl_Combat_C中的(real)属性InventoryComp时，结果为'无'。"

## 原因
关卡蓝图中的 InventoryComp 变量为 None，导致 AddItem 调用失败。

## 解决方案

### 方法1：每次使用前重新获取（推荐）

不要缓存 InventoryComp 为变量，每次使用时重新获取：

```
[Spawn Equipment From Data Asset]
    │
    ▼
[Is Valid] (检查 SpawnedEquipment 是否有效)
    │
    ▼ (True)
[Get Player Pawn] (重新获取玩家)
    │
    ▼
[Get Component by Class] (获取背包组件)
    ├── Component Class: Inventory Component
    │
    ▼
[Is Valid] (检查获取到的组件是否有效)
    │
    ▼ (True)
[Add Item] (调用函数)
    ├── Target: 连接 Get Component by Class 的返回值（不要提升为变量）
    ├── Item: SpawnedEquipment
    └── Return Value: 连接到 Branch
```

### 方法2：使用 Try Get Pawn Owner

如果关卡蓝图是在 Level 中，使用：

```
[Get Player Controller]
    │
    ▼
[Get Controlled Pawn]
    │
    ▼
[Get Component by Class]
    ├── Component Class: Inventory Component
```

### 方法3：在玩家角色蓝图中处理

不要在关卡蓝图中处理背包逻辑，而是在玩家角色蓝图中定义一个事件：

**玩家角色蓝图（BP_PlayerCharacter）:**
```
[自定义事件: AddItemToInventory]
    ├── 输入: Item (AEquipmentBase)
    │
    ▼
[Get Component by Class] (Inventory Component)
    │
    ▼
[Add Item]
    ├── Target: 组件返回值
    └── Item: 输入参数
    │
    ▼
[Return Node] (返回是否成功)
```

**关卡蓝图:**
```
[Spawn Equipment From Data Asset]
    │
    ▼
[Get Player Pawn]
    │
    ▼
[Cast to BP_PlayerCharacter]
    │
    ▼
[AddItemToInventory] (调用玩家角色中的事件)
    ├── Item: SpawnedEquipment
```

## 立即修复步骤

1. 打开关卡蓝图
2. 找到获取 InventoryComp 的部分
3. 删除 `提升为变量` 的 InventoryComp 变量
4. 改为直接从 `Get Component by Class` 连接到 `Add Item` 的 Target
5. 在 `Get Component by Class` 和 `Add Item` 之间添加 `Is Valid` 检查

## 为什么不要缓存为变量

- 玩家角色可能会被销毁和重新生成（如死亡重生）
- 组件可能在某些情况下被重新创建
- 关卡蓝图的生命周期与玩家角色不一致

## 调试技巧

在 Add Item 之前添加 Print String 检查：

```
[Get Component by Class]
    │
    ▼
[Print String]
    └── In String: "InventoryComp: " + [Is Valid] 的结果
    │
    ▼
[Branch] (Is Valid)
    ├── True → [Add Item]
    └── False → [Print String] "错误：找不到背包组件！"
```
