# OpenXR 架构重构设计说明

## 重构目标

将原本集中在 `OpenXRTutorial` 类中的复杂渲染逻辑进行解耦，提高代码的可维护性和可扩展性。

## 新的架构设计

### 1. 职责划分

#### OpenXRTutorial
- **职责**: 应用程序生命周期管理
- **负责**: 
  - OpenXR 系统初始化和清理
  - 主循环控制
  - 平台事件处理
  - 组件协调

#### OpenXRRenderer  
- **职责**: OpenXR 渲染流程管理
- **负责**:
  - 渲染循环管理 (WaitFrame/BeginFrame/EndFrame)
  - 视图矩阵计算
  - 渲染状态设置 (viewport, scissor, attachments)
  - 交换链管理
  - 场景渲染器协调

#### SceneRenderer (抽象基类)
- **职责**: 场景渲染抽象接口
- **负责**:
  - 定义场景渲染接口
  - 提供通用着色器加载功能
  - 管理图形资源生命周期

#### TableFloorScene (具体实现)
- **职责**: 具体场景实现
- **负责**:
  - 桌子和地板几何体渲染
  - 场景特定的着色器和缓冲区
  - 场景参数配置

#### XRMathUtils (工具类)
- **职责**: 数学计算封装
- **负责**:
  - 投影矩阵计算
  - 视图矩阵计算
  - 模型矩阵计算
  - 组合矩阵计算

### 2. 数据流

```
OpenXRTutorial
    ↓ 初始化
OpenXRRenderer ← 设置 ← TableFloorScene
    ↓ 每帧
XRMathUtils → 矩阵计算 → OpenXRRenderer → TableFloorScene.Render()
```

### 3. 主要改进

#### 问题解决
1. **矩阵计算抽离**: 投影/视图矩阵计算从 `OpenXRTutorial` 移至 `OpenXRRenderer` 和 `XRMathUtils`
2. **渲染逻辑分离**: `RenderLayer()` 的复杂逻辑分解到 `OpenXRRenderer` 的多个方法中
3. **职责清晰化**: 每个类都有明确的单一职责
4. **代码复用**: 数学计算和渲染流程可被其他场景复用

#### 架构优势
1. **可扩展性**: 新的场景类型只需继承 `SceneRenderer`
2. **可维护性**: 各部分职责清晰，便于调试和修改
3. **可测试性**: 各组件可以独立测试
4. **代码复用**: 渲染流程和数学计算可被多个场景共享

### 4. 使用示例

```cpp
// 在 OpenXRTutorial 中的新用法
m_scene = new TableFloorScene(apiType);
m_renderer = new OpenXRRenderer(apiType);
m_renderer->SetSceneRenderer(m_scene);
m_renderer->SetViewHeight(m_viewHeightM);

// 主循环中只需要
m_renderer->RenderFrame();
```

### 5. 未来扩展

要添加新的场景类型，只需：
1. 继承 `SceneRenderer`
2. 实现必要的虚函数
3. 在 `OpenXRTutorial` 中替换场景实例

要修改渲染流程，只需：
1. 修改 `OpenXRRenderer` 中的相关方法
2. 所有场景都会自动受益

这种设计符合单一职责原则和开闭原则，使代码更加模块化和可维护。
