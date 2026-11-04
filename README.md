# fpsgamedemo

Unreal Engine 第一人称射击（FPS）演示工程，包含角色、武器、动画与 FPS 素材包资源。

## 目录结构

```
fps_game/
├── fps_game.uproject        # UE 工程描述文件
├── Config/                  # 工程配置（Engine / Game / Input / Editor）
└── Content/
    ├── Code/                # 蓝图与输入资产
    │   ├── Player/          # BP_FPSPlayer、GameMode、PlayerState 枚举、动画蓝图
    │   │   ├── Animation/   # ABP_FPSPlayer、BS_FPSPlayer、ABP_VIRTUS、BS_VIRTUS
    │   │   └── System/      # BP_FPSGameMode、Input（IA_Move/Look/Aim/Run、IMC_FPSInput）
    │   └── Weapon/          # BP_WeaponBase、BP_VIRTUS
    └── XL_FPSPack/          # FPS 素材包
        ├── Common/Sound/    # 风声、弹着点、脚步、物理材质音效
        ├── Player/Mesh/     # 角色网格与皮肤材质
        ├── Weapons/         # MCX-VIRTUS 武器模型与材质贴图
        └── Niagara_VFX/     # 特效与贴图
```

## 主要功能点

- **第一人称角色**：`BP_FPSPlayer` 搭配动画蓝图 `ABP_FPSPlayer`、混合空间 `BS_FPSPlayer`。
- **武器系统**：`BP_WeaponBase` 基类 + `BP_VIRTUS` 具体武器，配套武器动画蓝图与混合空间。
- **输入（Enhanced Input）**：`IMC_FPSInput` 映射 `IA_Move` / `IA_Look` / `IA_Aim` / `IA_Run`。
- **游戏框架**：`BP_FPSGameMode` 与 `E_PlayerState` 玩家状态枚举。
- **完整音效**：弹着点、脚步、滑步、跳跃、落地等分类素材。

## 打开方式

1. 安装对应版本的 Unreal Engine（工程使用源码编译版引擎，`EngineAssociation` 为 GUID）。
2. 右键 `fps_game/fps_game.uproject` → *Switch Unreal Engine version* 选择本地引擎。
3. 双击打开，首次加载会重新生成 `Intermediate/` 与 `DerivedDataCache/`（这两个目录已通过 `.gitignore` 排除）。

## 说明

- 仓库仅提交工程本体（Config + Content + .uproject），不含 `Intermediate/`、`Saved/`、`DerivedDataCache/` 等本地缓存。
- `Content/XL_FPSPack` 为第三方 FPS 素材包资源，商业使用请遵循素材原始授权。
