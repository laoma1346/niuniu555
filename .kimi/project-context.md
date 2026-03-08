# 财神末法冒险 - 项目快速参考

## 项目结构
```
Source/niuniu555/
├── AI/                          # AI行为树系统
│   ├── EnemyAIController.h/cpp
│   └── BehaviorTree/
│       ├── Tasks/
│       │   ├── BTTask_Attack.h/cpp
│       │   ├── BTTask_ChasePlayer.h/cpp
│       │   └── BTTask_Patrol.h/cpp
│       ├── Services/
│       │   └── BTService_UpdateTarget.h/cpp
│       └── Decorators/
│           └── BTDecorator_DistanceCheck.h/cpp
├── EnemySystem/                 # 敌人系统
│   ├── EnemyBase.h/cpp         # 敌人基类
│   ├── EnemyStateMachineComponent.h/cpp
│   ├── EnemyDataAsset.h/cpp
│   └── EnemyTypes.h
├── CombatSystem/                # 战斗系统
│   ├── DamageableInterface.h
│   ├── DamageTypes.h
│   └── DamageCalculator.h/cpp
├── WeaponSystem/                # 武器系统
├── AttributeSystem/             # 属性系统
└── ObjectPool/                  # 对象池系统
```

## 当前开发阶段
- **Phase 4**: 敌人与AI系统
- **已完成**: 敌人基类、状态机、行为树任务（巡逻/追击/攻击）
- **进行中**: AI行为树调试优化
- **待开发**: 敌人生成器、掉落系统

## 关键类说明

### EnemyBase
- 所有敌人的基类
- 整合属性系统、状态机、受击反馈
- 实现IDamageableInterface接口

### EnemyAIController
- AI控制器基类
- 管理行为树和感知系统
- 黑板Key管理

### BTTask_Attack
- 攻击任务节点
- 使用NodeMemory存储状态
- 攻击完成后返回Succeeded

## 常见Bug及解决方案

### 问题1: 攻击任务完成后卡住
**原因**: 返回Failed导致Selector重新评估
**解决**: 攻击完成返回Succeeded

### 问题2: 攻击冷却期间行为树震荡
**原因**: 冷却期间返回Failed
**解决**: 冷却期间返回InProgress等待

### 问题3: 目标丢失误判
**原因**: 攻击动画期间感知系统误判
**解决**: 添加目标丢失延迟（1.5秒）

## 编译命令
```
E:\UE_5.6\UE_5.6\Engine\Build\BatchFiles\Build.bat niuniu555Editor Win64 Development -Project="C:\Users\Win10\Desktop\niuniu555\niuniu555.uproject" -Progress
```

## 调试快捷键
- `P` - 显示导航网格
- `'` - AI调试视图
