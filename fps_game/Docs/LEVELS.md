# 关卡设计文档

五张手工生成的竞技场关卡：`/Game/Maps/Ch01` – `/Game/Maps/Ch05`。全部用 `/Engine/BasicShapes` 网格 + 自定义 PBR 材质拼装，导航为动态生成（`RecastNavMesh`，`RuntimeGeneration=Dynamic`），每张图都带 `NavMeshBoundsVolume`，僵尸可在整张图内寻路。

## 地图链

```
Ch01（失联信号）→ Ch02（地下回声）→ Ch03（锈水）→ Ch04（静默之塔）→ Ch05（黎明协议）
```

- 默认启动图：`/Game/Maps/Ch01`（`Config/DefaultEngine.ini` 的 `GameDefaultMap` / `EditorStartupMap`）。
- 清完最后一波后，`StoryDirector` 在 `TravelDelay` 秒后 `OpenLevel` 切到下一章；第五章 `next_map` 为空，停在终章卡。

## 通用配置

- 每图都有：`PlayerStart`、`DirectionalLight`、`SkyLight`、`SkyAtmosphere`、`ExponentialHeightFog`、`PostProcessVolume`、`NavMeshBoundsVolume`、`RecastNavMesh`、`StoryDirector`、`ZombieWaveSpawner`（放原点附近 `(0,0,60)`）、2 个 `StoryNote`、1 个 `StoryTrigger`。
- 材质：世界投影主材质 `/Game/Levels/Materials/M_WorldAligned` + 9 个材质实例（`MI_Concrete`、`MI_DirtyConcrete`、`MI_CrackedConcrete`、`MI_GarageFloor`、`MI_MetalPlate`、`MI_RustyMetal`、`MI_WoodPlanks`、`MI_Asphalt`、`MI_ForestGround`）。贴图来自 Poly Haven CC0 的 2K PBR 套图（原图在 `Content/Textures/<set>/`，导入为 `/Game/Levels/Textures/T_<set>_D/_N/_ARM`）；警示灯/发光体仍用 `M_Hazard`、`M_EmissiveCyan/Red/Amber`、`M_Lamp` 等简单材质。
- 环境音：每张图放一个 `ALevelAmbience`（`(0,0,200)`），循环播放风声：Ch01/Ch04/Ch05 = `SW_wind_on_the_top_0`（音量 0.55/0.5/0.35，音高 1.0/0.7/0.6），Ch02 = `SW_wind_hole_tower_loop_01_0`（0.45/0.9），Ch03 = `SW_wind_height_high_00_0`（0.5/0.8）。
- 无线电语音：`Content/Audio/Voices/CHxx_0x.wav` → 导入 `/Game/Story/Voices`；`DA_Campaign` 中无线电类 beat 的 `Voice` 指到对应 SoundWave，字幕出现时自动播放（带电台带限/底噪处理）。
- 演出：进入关卡时黑场淡入（2.4s）；终局后淡出到黑场（1.1s）再切图。
- 波次参数（`ZombieWaveSpawner`）：

| 地图 | 波数 | 首波数量 | 每波递增 | 刷怪半径 | 波间隔 |
| --- | --- | --- | --- | --- | --- |
| Ch01 | 2 | 4 | +2 | 750–1250 | 8s |
| Ch02 | 3 | 4 | +2 | 800–1400 | 7s |
| Ch03 | 3 | 5 | +2 | 800–1400 | 7s |
| Ch04 | 3 | 5 | +3 | 800–1500 | 7s |
| Ch05 | 3 | 5 | +3 | 900–1600 | 8s |

- 僵尸类：`/Game/Code/Zombie/BP_Zombie.BP_Zombie_C`。生成后会延迟 0.35s（最多重试 8 次）重启 AI 控制器的 StateTree，确保运行时生成的僵尸能正常寻路追击。
- 手感调优：僵尸步行速度由 `ZombieWaveSpawner` 统一覆盖（不改蓝图）：基础 290 cm/s，每波 +7%，个体随机 ±10%；后段波次压迫感更强。玩家沿用 `BP_FPSPlayer` 默认移动（600 cm/s）。

## 光影与可视度

- 灯带不再只是自发光贴片：每条灯带下方额外挂一盏 PointLight（强度 4200、半径 1050，离地 40cm），颜色随灯带材质（白/琥珀/绿/青/红），真正照亮墙面与地面。
- 每张图额外两盏天光补光（`DirectionalLight` 不可用时作为主氛围）：前场 `15000 / 3600`，后场 `9000 / 2800`，颜色冷白 `(0.85,0.90,1.0)`。
- `SkyLight` 关闭 `lower_hemisphere_is_black`，让地面方向也有一点天空反弹，避免脚下死黑。
- `PostProcessVolume`：Bloom 0.5、Vignette 0.22、最低亮度 1.0 / 最高 2.4、自动曝光补偿 +1.2、饱和度 1.06，整体更亮更透。
- 每章天空改为更亮的暖调/冷调方向光（Ch01 暖白 1.15；Ch02 冷蓝 0.55 室内；Ch03 阴绿 1.0；Ch04 夜蓝 0.95；Ch05 暗金 0.60），雾密度同步下调。
- 外围加了一圈 2.8 倍 arena 的 `OuterTerrain` 地面 + 巨石，避免出图后看到空底。

## 武器系统与手感

- 运行时由 C++ `UFpsWeaponComponent` 接管武器（`UStorySubsystem` 每 2 秒确保玩家 Pawn 上有该组件）：蓝图里的 `BP_VIRTUS` 不销毁，而是隐藏、关碰撞、关 Tick、弹匣清零（保留 `CurrentWeapon` 引用，避免蓝图图表读到空值报错），再挂载 4 把武器，避免蓝图武器与 C++ 武器同时开火。
- 武器表：

| 槽位 | 名称 | 弹匣 / 备弹 | 伤害 | 射速 (RPM) | 模式 | 特点 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | VIRTUS 突击步枪 | 30 / 150 | 24 | 640 | 全自动 | 均衡，散布 0.9° |
| 2 | K-9 冲锋枪 | 36 / 180 | 15 | 950 | 全自动 | 高射速，散布 1.8° |
| 3 | M-90 霰弹枪 | 6 / 42 | 11×9 | 75 | 半自动 | 9 弹丸，散布 4.5° |
| 4 | DMR-7 精确步枪 | 10 / 60 | 58 | 180 | 半自动 | 低散布 0.22°，开镜 45° FOV |

- 命中判定：从相机做 `ECC_Visibility` 线迹，骨骼名含 `head` 时伤害 ×2.2；命中僵尸走 `ApplyPointDamage`，死亡自毁仍由蓝图处理，波次统计不受影响。
- 手感：每把枪有独立后坐力（俯仰/水平随机）、枪口闪光点光（70000 强度、45ms）、开火/命中音效、换弹计时与 HUD 弹药面板；开镜会收紧 FOV 并把枪模右移。
- 手电：`F` 开关，SpotLight 14000 强度、3200 半径、16/30 度内锥，适合室内（Ch02/Ch03）。
- 蓝图 CDO 未暴露血量/弹药字段，因此数值调优集中在移动、波次与这套武器参数上（`FpsWeaponComponent.cpp` 的 `BuildLoadout`）。
- 枪模：4 把武器使用 CC0 低模（OpenGameArt「Low Poly Weapon Pack」by byzmod3d，m16 / stg44 / 12_gauge / sniper），源文件在 `Content/Models/Guns/`，经 `Content/Python/bake_guns.py` 统一烘焙成「枪管朝 +X、单位 cm、按目标长度缩放」后导入为 `/Game/Models/Guns/SM_gun_virtus|k9|m90|dmr`，运行时按槽位给材质（金属/锈/木）；若网格缺失自动回落到基础几何拼装。`Content/Python/import_guns.py` 可重复导入。

## 各章布局

### Ch01 · 失联信号（5200×4400×700，前哨站）

- 白天、暖灰光照。中央空地 + 外围围墙；北侧通信塔（四腿 + 平台），东南角有一间三面墙的办公小屋（`OfficeBack/Left/Right/Roof`）。
- 掩体：两只集装箱（锈红、金属）、三只板条箱、两道黄色路障。
- 笔记：*值班日志* 在 `(900,-1150,230)`，*撤离名单* 在 `(1900,-1300,100)`（就在办公小屋内）。
- 触发区：`(-900,-700)` 附近 → `CH01_TRIG`。
- 灯光：白色灯带两处、通信塔平台琥珀色灯带。

### Ch02 · 地下回声（5600×5000×420，停车场 B2）

- 室内、低矮天花板（局部 `Ceiling` 板），9 根方柱网格，三排停靠车辆（`Car_A/B`，金属/锈色交替），北侧一段斜坡台阶，顶部管道。
- 阴影重，只有两排白色灯带，靠车体与柱子形成走位空间。
- 笔记：*断电通知* 在 `(0,-1150,140)`，*车主留言* 在 `(-1300,800,200)`。
- 触发区：`(1200,-1600)` → `CH02_TRIG`。

### Ch03 · 锈水（5200×4800×800，污水处理厂）

- 三座圆柱形污水池（`PoolGreen` 材质，直径约 840–1040），两根架空管道，南侧一条贯通走道（`Catwalk` + 四根立柱）。
- 东侧泵机组（`Pump`），散落黄色 / 锈色板条箱。
- 偏绿雾感，绿色灯带 + 琥珀色灯带各一。
- 笔记：*水质检测单* 在 `(-700,500,190)`，*泵房交接记录* 在 `(1500,1500,260)`。
- 触发区：`(-1500,-1700)` → `CH03_TRIG`。

### Ch04 · 静默之塔（4800×4800×1600，中央通讯塔）

- 垂直空间最大：中央塔芯（`TowerCore`，700×700×1800）+ 四根塔腿，三层环形平台（500/1100/1600 高度）。
- 地面三道混凝土矮墙作掩体；塔身四周竖向青色灯带。
- 笔记：*塔顶维护日志* 在 `(900,900,200)`，*一名士兵的便条* 在 `(-1500,-700,200)`。
- 触发区：`(0,-1900)` → `CH04_TRIG`。

### Ch05 · 黎明协议（6400×5600×1000，市指挥中枢）

- 中庭中央主控台（`ConsoleBase/ConsoleTop` + 悬浮 `Hologram` 球体，不阻挡移动），两侧副控台，六根方柱围合走廊。
- 南北两条红色警示灯带，主控台琥珀色灯带。
- 笔记：*黎明协议 · 摘要* 在 `(1400,300,200)`，*名单* 在 `(-1400,-300,200)`。
- 触发区：`(0,-2000)` → `CH05_TRIG`。

## 运行时报错修复记录

- 蓝图 `CurrentWeapon` 空引用（StopFire / Reload 节点报「结果为无」）：不再销毁蓝图武器，改为隐藏 + 关碰撞/Tick + 弹药清零，保留有效引用（`StorySubsystem.cpp` 的 `StripBlueprintWeapon`）。
- 僵尸生成瞬间的 `LogStateTree: Error: ... Could not find context actor of type Pawn`：波次生成改为延迟生成（`SpawnActorDeferred`），先把 `AutoPossessAI` 设为 `Disabled`、生成 AI Controller 并在 `FinishSpawning` 之前 `Possess`，再让控制器 BeginPlay 启动 StateTree；原有延迟 `RestartLogic` 作为兜底保留。
- `LogMaterial: Warning: Material /Game/Levels/Materials/M_WorldAligned missing bUsedWithNanite=True`：主材质已设置 `Used With Nanite`（MI 继承），重编译并保存后不再提示，避免 Nanite 网格回落到默认材质。
- 其余日志（`LogAutomationTest Condition failed`、EOS/HTTP 超时、VisionOS/Slate png 读取失败）来自引擎启动流程，与本项目无关。

## 重新生成 / 验证

```bash
# 1) 贴图与材质（首次或换贴图时执行）
... -ExecutePythonScript="<project>/Content/Python/build_textures.py" ...

# 2) 地图 + 剧情数据库
#    若 Content/Maps/Ch01..Ch05.umap 已存在，先删除这些文件再执行
#    （编辑器里对已加载关卡做 delete_asset 会崩溃）
rm Content/Maps/Ch0*.umap
"<UE>/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "<project>.uproject" \
  -ExecutePythonScript="<project>/Content/Python/build_levels.py" \
  -RenderOffscreen -windowed -unattended -nosplash -stdout -FullStdOutLogOutput

# 3) 校验（读取 DA_Campaign 与各图 actor 统计、配语音）
... -ExecutePythonScript="<project>/Content/Python/verify_levels.py" ...
```

生成器入口：`Content/Python/build_levels.py` 的 `CHAPTERS` 列表（章节文本、笔记、触发区、波次参数、环境音都在这里）；改完重跑即可整体重建。

## 配音生成流水线（可选）

1. `Content/Python/extract_radio_beats.py`：从 `CHAPTERS` 抽取无线电台词 → `Content/Audio/radio_manifest.json`。
2. `Content/Audio/synth_radio.ps1`：Windows SAPI（Microsoft Huihui Desktop）合成到 `Content/Audio/raw/`。
3. `Content/Audio/radio_effect.py`：带限 + 底噪 + 软削波 + 去直流 + 边缘淡化 → `Content/Audio/Voices/`。
4. `Content/Python/rebuild_campaign.py`：导入 `/Game/Story/Voices` 并重写 `DA_Campaign`（只重建剧情数据库，不动地图）。

## 操作方式

- 用 Unreal Editor 打开项目即进入 `Ch01`；PIE 或打包运行同理。
- 清理区域内的僵尸推进波次；最后一波清空后播放终章卡并自动切图。
- 靠近 `StoryNote`（档案夹模型 + 点光）即自动弹出档案面板显示笔记全文，提示条显示「E 阅读档案」（当前为靠近触发，无需按键；面板显示 14 秒，冷却 12 秒防重复）。
- 武器键位：`1`–`4` 直接选枪，`Q`/`E` 或滚轮切换，`R` 换弹，`右键` 开镜，`左键` 开火，`F` 手电。
