# Kimi Code UE5 快速配置包

## 📦 包含文件

```
快速配置包/
├── .kimi/
│   ├── ue5-expert.toml          # 专家配置文件（核心）
│   └── project-context.md       # 项目上下文说明
├── start-kimi.bat               # 一键启动脚本
└── Kimi常用命令.txt             # 命令速查表
```

## 🚀 快速开始

### 步骤1：复制文件到项目

把 `快速配置包` 文件夹里的所有内容复制到你的项目根目录：
```
C:\Users\Win10\Desktop\niuniu555\
```

复制后结构应该是：
```
C:\Users\Win10\Desktop\niuniu555\
├── .kimi\
│   ├── ue5-expert.toml
│   └── project-context.md
├── start-kimi.bat
├── Kimi常用命令.txt
├── Source\
│   └── niuniu555\
└── ...其他文件
```

### 步骤2：登录Kimi（首次使用）

在VS Code终端中执行：
```powershell
kimi login
```
按提示打开浏览器完成授权。

### 步骤3：启动Kimi

**方法1：双击启动（推荐新手）**
- 双击 `start-kimi.bat` 文件

**方法2：命令行启动**
```powershell
cd C:\Users\Win10\Desktop\niuniu555
.\start-kimi.bat
```

### 步骤4：开始使用

浏览器会自动打开Kimi Web界面，你可以开始提问了！

---

## 💡 新手使用技巧

### 每次会话开始时

在Kimi Web界面粘贴这段提示，帮助它快速了解项目：
```
请基于以下文件快速了解当前项目状态：
1. AGENTS.md - 查看开发路线图
2. .kimi/project-context.md - 查看项目快速参考
当前问题：[描述你的问题]
```

### 遇到反复修不好的Bug时

```
这个问题已经反复修改多次仍未解决，请：
1. 仔细分析根本原因
2. 提供完整的修复代码
3. 说明修复原理
4. 提供测试验证步骤
```

### 要求代码解释

```
请解释这段代码的每一行作用，以及为什么要这样写：
```cpp
[粘贴代码]
```
```

---

## 📝 常用提示词模板

### 代码审查
```
请审查这段代码，检查：
1. 内存安全问题（UPROPERTY, 空指针检查）
2. UE5宏使用是否正确（UCLASS, UPROPERTY, UFUNCTION）
3. 是否有潜在bug
```

### 调试帮助
```
我的代码编译报错，错误信息如下：
[粘贴错误信息]
请帮我分析原因并给出修复方案。
```

### Bug修复
```
我的[功能]出现以下问题：
[问题描述]
相关代码：
```cpp
[粘贴代码]
```
请帮我找出原因并修复。
```

---

## ⚙️ 配置文件说明

### ue5-expert.toml

这是Kimi的核心配置文件，包含：
- **模型设置**: 使用kimi-k2-5最强模型
- **温度设置**: 0.2（更确定性的代码生成）
- **自动加载文件**: 自动读取项目关键文件作为上下文
- **安全设置**: 自动批准安全操作，禁止危险操作
- **系统提示**: 定义Kimi的行为规范和代码风格

### project-context.md

项目快速参考文档，包含：
- 项目结构说明
- 当前开发阶段
- 关键类说明
- 常见Bug及解决方案
- 编译命令和调试快捷键

---

## 🔧 故障排除

### 问题1：启动时提示"需要登录"

**解决**：执行 `kimi login` 重新登录

### 问题2：启动后浏览器没打开

**解决**：手动在浏览器中打开显示的URL地址

### 问题3：Kimi说找不到文件

**解决**：确保配置文件路径正确，检查 `.kimi\ue5-expert.toml` 是否存在

---

## 📚 进阶配置

### 修改模型

编辑 `.kimi\ue5-expert.toml`，修改 `[model]` 部分：
```toml
[model]
name = "kimi-k2-5"    # 可选: kimi-k2-5, kimi-k1.5, kimi-k1
```

### 添加自动加载文件

编辑 `.kimi\ue5-expert.toml`，在 `[context]` 部分添加：
```toml
[context]
files = [
    "Source/niuniu555/**/*.h",
    "Source/niuniu555/**/*.cpp",
    "niuniu555.uproject",
    "AGENTS.md",
    "你的新文件路径"    # 添加这里
]
```

---

## 📞 需要帮助？

如果遇到问题：
1. 查看 `Kimi常用命令.txt` 速查表
2. 检查配置文件路径是否正确
3. 尝试重新登录 `kimi login`

---

*配置包版本: 1.0*
*适用: UE5游戏开发新手*
