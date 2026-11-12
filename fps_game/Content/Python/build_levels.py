# Builds the five campaign levels, their materials, gameplay actors and the
# campaign story database. Run inside the editor:
#   UnrealEditor-Cmd.exe fps_game.uproject -ExecutePythonScript=Content/Python/build_levels.py -RenderOffscreen -windowed -unattended -nosplash
# (Do NOT use -nullrhi: spawning actors there crashes with INT_DIVIDE_BY_ZERO.)

import unreal
import math
import os
import traceback

CUBE_MESH = '/Engine/BasicShapes/Cube.Cube'
CYLINDER_MESH = '/Engine/BasicShapes/Cylinder.Cylinder'
SPHERE_MESH = '/Engine/BasicShapes/Sphere.Sphere'
MAT_ROOT = '/Game/Levels/Materials'
MAP_ROOT = '/Game/Maps'
DB_PATH = '/Game/Story/DA_Campaign'
VOICE_DIR = '/Game/Story/Voices'
AMBIENCE = {
    'wind_top': '/Game/XL_FPSPack/Common/Sound/Ambient/Wind/SW_wind_on_the_top_0',
    'wind_tower': '/Game/XL_FPSPack/Common/Sound/Ambient/Wind/SW_wind_hole_tower_loop_01_0',
    'wind_high': '/Game/XL_FPSPack/Common/Sound/Ambient/Wind/SW_wind_height_high_00_0',
}
ZOMBIE_PATHS = [
    '/Game/Code/Zombie/BP_Zombie.BP_Zombie_C',
    '/Game/Zombie/BP_Zombie.BP_Zombie_C',
]

_asset_tools = None
_actor_sys = None
_level_editor = None


def tools():
    global _asset_tools
    if _asset_tools is None:
        _asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    return _asset_tools


def actor_sys():
    global _actor_sys
    if _actor_sys is None:
        _actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return _actor_sys


def level_editor():
    global _level_editor
    if _level_editor is None:
        _level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    return _level_editor


def log(msg):
    unreal.log('[BuildLevels] ' + str(msg))


def warn(msg):
    unreal.log_warning('[BuildLevels] ' + str(msg))


def setp(obj, names, value):
    if isinstance(names, str):
        names = [names]
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return True
        except Exception:
            continue
    warn('Could not set property %s on %s' % (names, obj))
    return False


def getp(obj, names, default=None):
    if isinstance(names, str):
        names = [names]
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return default


def load_asset(path):
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        return None
    return unreal.load_asset(path)


def load_class_any(paths):
    for path in paths:
        cls = unreal.load_class(None, path)
        if cls:
            return cls
    return None


def spawn(cls, loc=(0, 0, 0), rot=(0, 0, 0), label=None):
    actor = actor_sys().spawn_actor_from_class(cls, unreal.Vector(loc[0], loc[1], loc[2]),
                                               unreal.Rotator(rot[0], rot[1], rot[2]))
    if actor is None:
        warn('Failed to spawn %s' % cls)
        return None
    if label:
        try:
            actor.set_actor_label(label)
        except Exception:
            pass
    return actor


# --------------------------------------------------------------------------------------
# Materials
# --------------------------------------------------------------------------------------

def ensure_material(name, base, roughness=0.85, metallic=0.0, emissive=None):
    path = MAT_ROOT + '/' + name
    existing = load_asset(path)
    if existing:
        return existing

    unreal.EditorAssetLibrary.make_directory(MAT_ROOT)
    mat = tools().create_asset(name, MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    if mat is None:
        warn('Could not create material ' + name)
        return None

    mel = unreal.MaterialEditingLibrary

    color = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -420, -160)
    color.set_editor_property('constant', unreal.LinearColor(base[0], base[1], base[2], 1.0))
    mel.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR)

    rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -420, 20)
    rough.set_editor_property('r', float(roughness))
    mel.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)

    metal = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -420, 120)
    metal.set_editor_property('r', float(metallic))
    mel.connect_material_property(metal, '', unreal.MaterialProperty.MP_METALLIC)

    if emissive is not None:
        em = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -420, 220)
        em.set_editor_property('constant', unreal.LinearColor(emissive[0] * emissive[3],
                                                             emissive[1] * emissive[3],
                                                             emissive[2] * emissive[3], 1.0))
        mel.connect_material_property(em, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    unreal.EditorAssetLibrary.save_asset(path)
    return mat


def build_materials():
    mats = {}
    # PBR material instances built by Content/Python/build_textures.py (world-aligned,
    # sampled from Poly Haven CC0 texture sets in Content/Textures).
    insts = (
        ('concrete', 'MI_Concrete'),
        ('concrete_dark', 'MI_DirtyConcrete'),
        ('cracked', 'MI_CrackedConcrete'),
        ('floor', 'MI_GarageFloor'),
        ('metal', 'MI_MetalPlate'),
        ('rust', 'MI_RustyMetal'),
        ('wood', 'MI_WoodPlanks'),
        ('asphalt', 'MI_Asphalt'),
        ('terrain', 'MI_ForestGround'),
    )
    for key, inst in insts:
        asset = load_asset(MAT_ROOT + '/' + inst)
        if asset is None:
            warn('Missing material instance %s (run build_textures.py); using flat fallback' % inst)
            asset = ensure_material('M_Fallback_' + key, (0.24, 0.24, 0.24), 0.9)
        mats[key] = asset

    # flat / emissive materials are still authored here
    mats['dark'] = ensure_material('M_Dark', (0.06, 0.07, 0.08), 0.7)
    mats['yellow'] = ensure_material('M_Hazard', (0.80, 0.62, 0.10), 0.6)
    mats['green'] = ensure_material('M_PoolGreen', (0.13, 0.22, 0.18), 0.55)
    mats['cyan'] = ensure_material('M_EmissiveCyan', (0.02, 0.02, 0.02), 0.4, 0.0, (0.10, 0.85, 1.0, 8.0))
    mats['red'] = ensure_material('M_EmissiveRed', (0.02, 0.02, 0.02), 0.4, 0.0, (1.0, 0.10, 0.05, 6.0))
    mats['amber'] = ensure_material('M_EmissiveAmber', (0.02, 0.02, 0.02), 0.4, 0.0, (1.0, 0.55, 0.12, 5.0))
    mats['white'] = ensure_material('M_Lamp', (0.02, 0.02, 0.02), 0.4, 0.0, (0.85, 0.92, 1.0, 10.0))
    return mats


# --------------------------------------------------------------------------------------
# Radio voice lines (SAPI-synthesised wavs under Content/Audio/Voices)
# --------------------------------------------------------------------------------------

def import_voices():
    """Imports the radio wavs and returns {beat_id: object_path} for beats with a voice."""
    voices = {}
    wanted = []
    for cfg in CHAPTERS:
        for (beat_id, kind, speaker, text, auto) in cfg['beats']:
            if '无线电' in speaker:
                wanted.append(beat_id)
    if not wanted:
        return voices

    wav_dir = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()),
                           'Audio', 'Voices')
    factory_cls = getattr(unreal, 'SoundFactory', None) or getattr(unreal, 'SoundWaveFactory', None)
    if factory_cls is None:
        warn('No sound import factory available; voice lines will stay silent.')
        return voices

    tasks = []
    for beat_id in wanted:
        wav = os.path.join(wav_dir, beat_id + '.wav')
        if not os.path.exists(wav):
            warn('Missing voice wav: ' + wav)
            continue
        task = unreal.AssetImportTask()
        task.set_editor_property('filename', wav)
        task.set_editor_property('destination_path', VOICE_DIR)
        task.set_editor_property('destination_name', beat_id)
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('automated', True)
        task.set_editor_property('save', True)
        task.set_editor_property('factory', factory_cls())
        tasks.append(task)

    if not tasks:
        return voices

    unreal.EditorAssetLibrary.make_directory(VOICE_DIR)
    tools().import_asset_tasks(tasks)

    for beat_id in wanted:
        path = VOICE_DIR + '/' + beat_id
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            voices[beat_id] = path + '.' + beat_id
        else:
            warn('Voice import failed for ' + beat_id)
    log('Imported %d radio voice lines.' % len(voices))
    return voices


# --------------------------------------------------------------------------------------
# Geometry helpers
# --------------------------------------------------------------------------------------

def box(label, center, size, material, rot_z=0.0, mesh=CUBE_MESH, block=True):
    mesh_asset = load_asset(mesh)
    if mesh_asset is None:
        warn('Missing mesh ' + mesh)
        return None
    actor = spawn(unreal.StaticMeshActor, center, (0.0, 0.0, rot_z), label)
    if actor is None:
        return None
    smc = actor.get_component_by_class(unreal.StaticMeshComponent)
    if smc is None:
        warn('No StaticMeshComponent on ' + label)
        return None
    smc.set_static_mesh(mesh_asset)
    smc.set_material(0, material)
    if not block:
        smc.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    return actor


def wall_ring(size_x, size_y, height, thickness, material, door_side=None):
    box('Wall_N', (0, size_y * 0.5, height * 0.5), (size_x + thickness, thickness, height), material)
    box('Wall_S', (0, -size_y * 0.5, height * 0.5), (size_x + thickness, thickness, height), material)
    box('Wall_E', (size_x * 0.5, 0, height * 0.5), (thickness, size_y + thickness, height), material)
    box('Wall_W', (-size_x * 0.5, 0, height * 0.5), (thickness, size_y + thickness, height), material)


def lamp_strip(label, center, size, material):
    box(label, center, size, material, block=False)
    # Emissive strips now cast real light so the arena reads even in dark sections.
    color = (1.0, 0.93, 0.82)
    name = ''
    try:
        name = material.get_name()
    except Exception:
        pass
    if 'Amber' in name:
        color = (1.0, 0.62, 0.25)
    elif 'Green' in name:
        color = (0.30, 1.0, 0.55)
    elif 'Cyan' in name:
        color = (0.40, 0.90, 1.0)
    elif 'Red' in name:
        color = (1.0, 0.25, 0.20)
    add_light_ki(True, color, 4200.0, 1050.0, (center[0], center[1], center[2] - 40.0))


def add_light_ki(dark, color, intensity, radius, loc):
    light = spawn(unreal.PointLight, loc, label='PointLight')
    if light is None:
        return
    comp = light.get_component_by_class(unreal.PointLightComponent)
    if comp is None:
        return
    try:
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    except Exception:
        pass
    setp(comp, 'intensity', intensity)
    setp(comp, 'attenuation_radius', radius)
    setp(comp, 'light_color', unreal.Color(int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), 255))


def add_atmosphere(sun_pitch, sun_yaw, sun_intensity, sun_color, sky_intensity, fog_density, fog_color,
                   extra_lights=()):
    sun = spawn(unreal.DirectionalLight, (0, 0, 1800), (sun_pitch, sun_yaw, 0), 'Sun')
    if sun:
        comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            try:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            except Exception:
                pass
            setp(comp, 'intensity', sun_intensity)
            setp(comp, 'light_color', unreal.Color(int(sun_color[0] * 255), int(sun_color[1] * 255),
                                                   int(sun_color[2] * 255), 255))
            setp(comp, 'atmosphere_sun_light', True)

    sky = spawn(unreal.SkyLight, (0, 0, 900), label='SkyLight')
    if sky:
        comp = sky.get_component_by_class(unreal.SkyLightComponent)
        if comp:
            try:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            except Exception:
                pass
            setp(comp, 'real_time_capture', True)
            setp(comp, 'intensity', sky_intensity)
            setp(comp, 'lower_hemisphere_is_black', False)

    spawn(unreal.SkyAtmosphere, (0, 0, 0), label='SkyAtmosphere')

    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), label='Fog')
    if fog:
        comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if comp:
            setp(comp, 'fog_density', fog_density)
            setp(comp, 'fog_inscattering_luminance',
                 unreal.LinearColor(fog_color[0], fog_color[1], fog_color[2], 1.0))
            setp(comp, ['volumetric_fog', 'volumetric_scattering', 'enable_volumetric_fog'], True)

    ppv = spawn(unreal.PostProcessVolume, (0, 0, 0), label='PostProcess')
    if ppv:
        setp(ppv, 'unbound', True)
        settings = ppv.get_editor_property('settings')
        try:
            settings.override_bloom_intensity = True
            settings.bloom_intensity = 0.5
            settings.override_vignette_intensity = True
            settings.vignette_intensity = 0.22
            settings.override_auto_exposure_min_brightness = True
            settings.auto_exposure_min_brightness = 1.0
            settings.override_auto_exposure_max_brightness = True
            settings.auto_exposure_max_brightness = 2.4
            settings.override_auto_exposure_bias = True
            settings.auto_exposure_bias = 1.2
            settings.override_color_saturation = True
            settings.color_saturation = (1.06, 1.06, 1.06, 1.0)
            ppv.set_editor_property('settings', settings)
        except Exception as exc:
            warn('Post process settings: %s' % exc)

    for (loc, color, intensity, radius) in extra_lights:
        add_light_ki(True, color, intensity, radius, loc)


def player_start(x, y, yaw):
    spawn(unreal.PlayerStart, (x, y, 140), (0, 0, yaw), 'PlayerStart')


def nav_bounds(size_x, size_y, height):
    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, height * 0.5), (0, 0, 0), 'NavBounds')
    if nav:
        # Default volume brush is a 200cm cube.
        nav.set_actor_scale3d(unreal.Vector(size_x / 200.0, size_y / 200.0, height / 200.0))
# --------------------------------------------------------------------------------------
# Per-level prop layouts
# --------------------------------------------------------------------------------------

def props_ch01(m, cfg):
    box('Container_A', (-1100, -900, 150), (500, 240, 300), m['rust'], rot_z=8)
    box('Container_B', (900, 1000, 150), (520, 240, 300), m['metal'], rot_z=-12)
    box('Crate_1', (850, -1200, 70), (140, 140, 140), m['concrete'])
    box('Crate_2', (1010, -1130, 70), (140, 140, 140), m['concrete'], rot_z=20)
    box('Crate_3', (930, -1060, 210), (140, 140, 140), m['metal'], rot_z=35)
    box('Barrier_1', (-500, 1200, 75), (700, 100, 150), m['yellow'])
    box('Barrier_2', (-1400, 1600, 75), (500, 100, 150), m['yellow'], rot_z=25)
    for lx in (-450.0, 450.0):
        for ly in (-450.0, 450.0):
            box('TowerLeg', (lx, ly - 1700, 230), (60, 60, 460), m['metal'])
    box('TowerDeck', (0, -1700, 480), (1000, 1000, 40), m['metal'])
    box('OfficeBack', (1900, -1500, 200), (700, 40, 400), m['concrete_dark'])
    box('OfficeLeft', (1560, -1250, 200), (40, 500, 400), m['concrete_dark'])
    box('OfficeRight', (2240, -1250, 200), (40, 500, 400), m['concrete_dark'])
    box('OfficeRoof', (1900, -1250, 400), (720, 540, 40), m['concrete_dark'])
    lamp_strip('Lamp_A', (-1500, 0, 440), (80, 40, 24), m['white'])
    lamp_strip('Lamp_B', (1500, 900, 440), (80, 40, 24), m['white'])
    lamp_strip('Lamp_C', (0, -1780, 540), (240, 24, 24), m['amber'])


def props_ch02(m, cfg):
    for gx in (-1500.0, 0.0, 1500.0):
        for gy in (-1500.0, 0.0, 1500.0):
            box('Pillar', (gx, gy, 190), (300, 300, 380), m['concrete'])
    for gy in (-1600.0, 0.0, 1600.0):
        box('Ceiling', (0, gy, 390), (5600, 1500, 40), m['dark'])
    for i, gx in enumerate((-1600.0, -800.0, 0.0, 800.0, 1600.0)):
        box('Car_A', (gx, -800, 80), (430, 200, 170), m['rust'] if i % 2 else m['metal'], rot_z=90)
        box('Car_B', (gx, 800, 80), (430, 200, 170), m['metal'] if i % 2 else m['dark'], rot_z=90)
    box('Step_1', (0, -2300, 30), (900, 120, 60), m['concrete'])
    box('Step_2', (0, -2300, 80), (900, 120, 60), m['concrete'])
    box('Pipe_1', (-400, 2000, 310), (3800, 90, 90), m['metal'])
    lamp_strip('Lamp_A', (-1500, 0, 340), (200, 24, 18), m['white'])
    lamp_strip('Lamp_B', (1500, 0, 340), (200, 24, 18), m['white'])


def props_ch03(m, cfg):
    for (x, y, diameter, height) in ((-1300.0, -1100.0, 960.0, 520.0),
                                     (1200.0, -800.0, 840.0, 480.0),
                                     (200.0, 1300.0, 1040.0, 560.0)):
        box('Tank', (x, y, height * 0.5), (diameter, diameter, height), m['green'], mesh=CYLINDER_MESH)
    box('Pipe_1', (-300, -1900, 260), (3200, 90, 90), m['rust'])
    box('Pipe_2', (1400, 400, 300), (90, 2600, 90), m['metal'])
    box('Catwalk', (0, -2300, 330), (4200, 220, 40), m['metal'])
    for gx in (-1800.0, -600.0, 600.0, 1800.0):
        box('CatwalkLeg', (gx, -2300, 165), (80, 80, 330), m['metal'])
    box('Crate_1', (-700, 400, 70), (160, 160, 160), m['yellow'])
    box('Crate_2', (-520, 430, 200), (160, 160, 160), m['rust'])
    box('Pump', (1500, 1600, 120), (400, 300, 240), m['metal'])
    lamp_strip('Lamp_A', (0, -2200, 380), (160, 24, 18), m['green'])
    lamp_strip('Lamp_B', (-1400, 1500, 460), (160, 24, 18), m['amber'])


def props_ch04(m, cfg):
    box('TowerCore', (0, 0, 900), (700, 700, 1800), m['dark'])
    for (x, y) in ((-520.0, -520.0), (520.0, -520.0), (520.0, 520.0), (-520.0, 520.0)):
        box('TowerLeg', (x, y, 900), (220, 220, 1800), m['metal'])
    for z in (500.0, 1100.0, 1600.0):
        box('Ring_N', (0, 900, z), (2000, 60, 50), m['metal'])
        box('Ring_S', (0, -900, z), (2000, 60, 50), m['metal'])
        box('Ring_E', (900, 0, z), (60, 2000, 50), m['metal'])
        box('Ring_W', (-900, 0, z), (60, 2000, 50), m['metal'])
    box('Barrier_1', (-1500, 0, 120), (1400, 100, 240), m['concrete'])
    box('Barrier_2', (1500, 600, 120), (1400, 100, 240), m['concrete'], rot_z=30)
    box('Barrier_3', (0, -1700, 120), (1600, 100, 240), m['concrete'])
    for (x, y) in ((0.0, 352.0), (0.0, -352.0), (352.0, 0.0), (-352.0, 0.0)):
        lamp_strip('Strip', (x, y, 700), (26, 26, 1300), m['cyan'])


def props_ch05(m, cfg):
    box('ConsoleBase', (0, 0, 60), (900, 500, 120), m['dark'])
    box('ConsoleTop', (0, 0, 140), (1000, 600, 40), m['metal'])
    box('Hologram', (0, 0, 340), (280, 280, 280), m['cyan'], mesh=SPHERE_MESH, block=False)
    for gx in (-2000.0, 2000.0):
        for gy in (-1600.0, 0.0, 1600.0):
            box('Column', (gx, gy, 450), (260, 260, 900), m['concrete_dark'])
    for gy in (-1200.0, 1200.0):
        lamp_strip('RedStrip', (0, gy, 8), (4600, 30, 16), m['red'])
    box('SideConsole_A', (-1400, 0, 80), (500, 300, 160), m['metal'])
    box('SideConsole_B', (1400, 0, 80), (500, 300, 160), m['metal'])
    box('Crate_A', (-1100, 900, 70), (150, 150, 150), m['rust'])
    lamp_strip('Lamp_A', (0, -2500, 900), (2000, 24, 18), m['amber'])


# --------------------------------------------------------------------------------------
# Campaign data
# --------------------------------------------------------------------------------------

def beatkind(name):
    enum_cls = unreal.StoryBeatKind
    for attr in (name.upper(), name, 'b' + name.upper()):
        if hasattr(enum_cls, attr):
            return getattr(enum_cls, attr)
    attrs = [a for a in dir(enum_cls) if a.isupper()]
    raise RuntimeError('StoryBeatKind entry %s not found, options: %s' % (name, attrs))


CHAPTERS = [
    {
        'chapter': 'CH01',
        'map': '/Game/Maps/Ch01',
        'ambience': (AMBIENCE['wind_top'], 0.55, 1.0),
        'title': '失联信号',
        'subtitle': '撤离日 · 第 41 天 — 前哨站 K-9',
        'objective': '肃清前哨站，回收信号数据',
        'arena': (5200.0, 4400.0, 700.0),
        'floor': 'asphalt', 'wall': 'concrete',
        'sky': (-42.0, 35.0, 16.0, (1.0, 0.90, 0.78), 1.15, 0.007, (0.55, 0.58, 0.62)),
        'props': props_ch01,
        'notes': [
            ('值班日志', '02:14 巡逻队未归。\n02:40 围墙外出现异常人影。\n03:05 有人在我们的频道里呼救，声音不对——它一直在笑。\n\n最后一行被划掉了。', (900, -1150, 230)),
            ('撤离名单', '已撤离：17 人\n失联：3 人\n名单最末一行写着：灰隼小队，四人，下落不明。', (1900, -1300, 100)),
        ],
        'triggers': [((-900, -700, 0), 'CH01_TRIG')],
        'spawner_loc': (0, 0, 60),
        'spawner': {
            'total_waves': 2, 'base_count': 4, 'count_per_wave': 2,
            'start_delay': 7.0, 'time_between_waves': 8.0,
            'spawn_radius_min': 750.0, 'spawn_radius_max': 1250.0,
            'start_beat_id': 'CH01_WAVE', 'wave_cleared_beat_id': 'CH01_CLEAR',
            'all_cleared_beat_id': 'CH01_END',
        },
        'next_map': '/Game/Maps/Ch02',
        'travel_delay': 10.0,
        'beats': [
            ('CH01_01', 'subtitle', '无线电 · 林默', '灰隼，这里是远望号。前哨站 K-9 的信号在十七分钟前断了。整座城只剩下你还在响应。', True),
            ('CH01_02', 'subtitle', '无线电 · 林默', '先清空院子，再去找数据终端。记住，别在同一个地方停下超过十秒。', True),
            ('CH01_03', 'subtitle', '灰隼', '……无线电里只有杂音。空气里有铁锈味，还有别的什么。', True),
            ('CH01_WAVE', 'subtitle', '无线电 · 林默', '侦测到热源正在聚集。它们已经闻到你了。', False),
            ('CH01_CLEAR', 'subtitle', '无线电 · 林默', '第一波清干净了。别放松，更多的还在路上。', False),
            ('CH01_TRIG', 'subtitle', '灰隼', '办公区的门被从里面锁上了。有人在里面待过——而且不止一个人。', False),
            ('CH01_END', 'ending', '', '第一章 · 完', False),
        ],
    },
    {
        'chapter': 'CH02',
        'map': '/Game/Maps/Ch02',
        'ambience': (AMBIENCE['wind_tower'], 0.45, 0.9),
        'title': '地下回声',
        'subtitle': '撤离日 · 第 42 天 — 西区停车场 B2',
        'objective': '恢复 B2 层的应急电源',
        'arena': (5600.0, 5000.0, 420.0),
        'floor': 'floor', 'wall': 'concrete_dark',
        'sky': (-25.0, 200.0, 2.2, (0.6, 0.7, 0.9), 0.55, 0.025, (0.20, 0.24, 0.30),
                [((0, 0, 330), (0.35, 0.75, 1.0), 2600.0, 1100.0),
                 ((-1700, -1200, 330), (1.0, 0.75, 0.4), 2200.0, 900.0),
                 ((1700, 1200, 330), (0.4, 1.0, 0.75), 1800.0, 900.0)]),
        'props': props_ch02,
        'notes': [
            ('断电通知', 'B2 层将于 22:00 断电。请勿使用电梯。\n——物业', (0, -1150, 140)),
            ('车主留言', '如果看到这张纸条，别按喇叭。它们对声音有反应。\n我把车钥匙放在驾驶座下面了。', (-1300, 800, 200)),
        ],
        'triggers': [((1200, -1600, 0), 'CH02_TRIG')],
        'spawner_loc': (0, 0, 60),
        'spawner': {
            'total_waves': 3, 'base_count': 4, 'count_per_wave': 2,
            'start_delay': 7.0, 'time_between_waves': 7.0,
            'spawn_radius_min': 800.0, 'spawn_radius_max': 1400.0,
            'start_beat_id': 'CH02_WAVE', 'wave_cleared_beat_id': 'CH02_CLEAR',
            'all_cleared_beat_id': 'CH02_END',
        },
        'next_map': '/Game/Maps/Ch03',
        'travel_delay': 10.0,
        'beats': [
            ('CH02_01', 'subtitle', '无线电 · 林默', '第二个节点在西区停车场。断电之后，那里就没再传出过任何消息。', True),
            ('CH02_02', 'subtitle', '无线电 · 林默', '车库里没有天光，回声会骗人。如果你听见自己的脚步多了一下，别回头。', True),
            ('CH02_WAVE', 'subtitle', '无线电 · 林默', '它们从坡道涌下来了。', False),
            ('CH02_CLEAR', 'subtitle', '无线电 · 林默', '电源还在。继续往里走。', False),
            ('CH02_TRIG', 'subtitle', '灰隼', '墙上有人用血写了字：别开灯。', False),
            ('CH02_END', 'ending', '', '第二章 · 完', False),
        ],
    },
    {
        'chapter': 'CH03',
        'map': '/Game/Maps/Ch03',
        'ambience': (AMBIENCE['wind_high'], 0.5, 0.8),
        'title': '锈水',
        'subtitle': '撤离日 · 第 43 天 — 第七污水处理厂',
        'objective': '重启中继泵，接通通讯中继',
        'arena': (5200.0, 4800.0, 800.0),
        'floor': 'green', 'wall': 'concrete_dark',
        'sky': (-38.0, 120.0, 9.0, (0.85, 0.92, 0.82), 1.0, 0.016, (0.30, 0.38, 0.34)),
        'props': props_ch03,
        'notes': [
            ('水质检测单', '取样日期：撤离日前 3 天\n结论：铁、锰超标。\n备注：第三号池的样本里检测到未知有机物。', (-700, 500, 190)),
            ('泵房交接记录', '夜班：一切正常。\n早班：泵房的门别开。开了就关不上了。', (1500, 1500, 260)),
        ],
        'triggers': [((-1500, -1700, 0), 'CH03_TRIG')],
        'spawner_loc': (0, 0, 60),
        'spawner': {
            'total_waves': 3, 'base_count': 5, 'count_per_wave': 2,
            'start_delay': 7.0, 'time_between_waves': 7.0,
            'spawn_radius_min': 800.0, 'spawn_radius_max': 1400.0,
            'start_beat_id': 'CH03_WAVE', 'wave_cleared_beat_id': 'CH03_CLEAR',
            'all_cleared_beat_id': 'CH03_END',
        },
        'next_map': '/Game/Maps/Ch04',
        'travel_delay': 10.0,
        'beats': [
            ('CH03_01', 'subtitle', '无线电 · 林默', '中继站泡在污水里，但还活着。第三个节点就挂在那台老泵上。', True),
            ('CH03_02', 'subtitle', '无线电 · 林默', '检测仪说水里没有活物。仪器不会撒谎——大概吧。', True),
            ('CH03_WAVE', 'subtitle', '无线电 · 林默', '水下面有东西在动。别站在池边。', False),
            ('CH03_CLEAR', 'subtitle', '无线电 · 林默', '中继恢复了。三个节点，已经连上两个。', False),
            ('CH03_TRIG', 'subtitle', '灰隼', '工人们把泵房的门焊死了。从里面焊的。', False),
            ('CH03_END', 'ending', '', '第三章 · 完', False),
        ],
    },
    {
        'chapter': 'CH04',
        'map': '/Game/Maps/Ch04',
        'ambience': (AMBIENCE['wind_top'], 0.5, 0.7),
        'title': '静默之塔',
        'subtitle': '撤离日 · 第 44 天 — 中央通讯塔',
        'objective': '登上通讯塔，上传城市数据',
        'arena': (4800.0, 4800.0, 1600.0),
        'floor': 'concrete', 'wall': 'metal',
        'sky': (-8.0, 250.0, 7.0, (0.60, 0.70, 1.0), 0.95, 0.012, (0.26, 0.34, 0.50)),
        'props': props_ch04,
        'notes': [
            ('塔顶维护日志', '信号灯每 12 秒闪烁一次。这是给活人看的。\n如果你读到这段字，说明还有活人。', (900, 900, 200)),
            ('一名士兵的便条', '我们守了这里十九天。第二十天，补给没有来。\n如果有人来换班，请告诉我们，外面还有城市吗？', (-1500, -700, 200)),
        ],
        'triggers': [((0, -1900, 0), 'CH04_TRIG')],
        'spawner_loc': (0, 0, 60),
        'spawner': {
            'total_waves': 3, 'base_count': 5, 'count_per_wave': 3,
            'start_delay': 7.0, 'time_between_waves': 7.0,
            'spawn_radius_min': 800.0, 'spawn_radius_max': 1500.0,
            'start_beat_id': 'CH04_WAVE', 'wave_cleared_beat_id': 'CH04_CLEAR',
            'all_cleared_beat_id': 'CH04_END',
        },
        'next_map': '/Game/Maps/Ch05',
        'travel_delay': 10.0,
        'beats': [
            ('CH04_01', 'subtitle', '无线电 · 林默', '最后一个节点在塔顶。上去之后，整座城的数据都会回到舰队。', True),
            ('CH04_02', 'subtitle', '无线电 · 林默', '我知道你累了。但今天之后，就不用再打了。', True),
            ('CH04_WAVE', 'subtitle', '无线电 · 林默', '塔基的守备比你想的多。', False),
            ('CH04_CLEAR', 'subtitle', '无线电 · 林默', '数据上传完成。现在……下来，去指挥中枢。', False),
            ('CH04_TRIG', 'subtitle', '灰隼', '刚刚，对讲机里有人叫了我的名字。我的名字已经三年没人叫过了。', False),
            ('CH04_END', 'ending', '', '第四章 · 完', False),
        ],
    },
    {
        'chapter': 'CH05',
        'map': '/Game/Maps/Ch05',
        'ambience': (AMBIENCE['wind_top'], 0.35, 0.6),
        'title': '黎明协议',
        'subtitle': '撤离日 · 第 45 天 — 市指挥中枢',
        'objective': '进入指挥中枢，执行「黎明协议」',
        'arena': (6400.0, 5600.0, 1000.0),
        'floor': 'dark', 'wall': 'concrete_dark',
        'sky': (-20.0, 300.0, 3.0, (0.8, 0.7, 0.58), 0.60, 0.016, (0.22, 0.18, 0.16),
                [((0, -1500, 500), (1.0, 0.45, 0.2), 3200.0, 1400.0),
                 ((0, 1500, 500), (0.3, 0.85, 1.0), 2600.0, 1400.0)]),
        'props': props_ch05,
        'notes': [
            ('黎明协议 · 摘要', '协议编号：DAWN-01\n目的：在城区清空后，重启城市核心，恢复供水、供电与医疗网络。\n执行条件：最后一个信标存活。', (1400, 300, 200)),
            ('名单', '名单第 1 行：灰隼（本名已涂黑）\n状态：存活\n权限：开启黎明。', (-1400, -300, 200)),
        ],
        'triggers': [((0, -2000, 0), 'CH05_TRIG')],
        'spawner_loc': (0, 0, 60),
        'spawner': {
            'total_waves': 3, 'base_count': 5, 'count_per_wave': 3,
            'start_delay': 8.0, 'time_between_waves': 8.0,
            'spawn_radius_min': 900.0, 'spawn_radius_max': 1600.0,
            'start_beat_id': 'CH05_WAVE', 'wave_cleared_beat_id': 'CH05_CLEAR',
            'all_cleared_beat_id': 'CH05_END',
        },
        'next_map': '',
        'travel_delay': 0.0,
        'beats': [
            ('CH05_01', 'subtitle', '无线电 · 林默', '最后一件事。指挥中枢里有一份名单，还有一道叫「黎明」的指令。', True),
            ('CH05_02', 'subtitle', '无线电 · 林默', '拿到它，然后回家。', True),
            ('CH05_WAVE', 'subtitle', '无线电 · 林默', '中庭的守卫全醒了。为了今天，它们等了很久。', False),
            ('CH05_CLEAR', 'subtitle', '无线电 · 林默', '……灰隼，谢谢你。替我们所有人。', False),
            ('CH05_TRIG', 'subtitle', '灰隼', '名单上第一个名字是我。签署日期是今天。', False),
            ('CH05_END', 'ending', '', '黎明之前', False),
        ],
    },
]


# --------------------------------------------------------------------------------------
# Level construction
# --------------------------------------------------------------------------------------

def new_level(path):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        try:
            unreal.EditorAssetLibrary.delete_asset(path)
        except Exception as exc:
            warn('Could not delete old level %s: %s' % (path, exc))
    try:
        return level_editor().new_level(path)
    except Exception:
        return unreal.EditorLevelLibrary.new_level(path)


def save_current_level():
    try:
        return level_editor().save_current_level()
    except Exception:
        return unreal.EditorLevelLibrary.save_current_level()


def build_level(cfg, mats):
    if not new_level(cfg['map']):
        warn('Could not create level ' + cfg['map'])
        return False

    sx, sy, h = cfg['arena']
    log('  step: floor+walls')
    box('Floor', (0, 0, -10), (sx, sy, 20), mats[cfg['floor']])
    wall_ring(sx, sy, h, 40, mats[cfg['wall']])

    # Outer terrain apron and boulders so the level does not end abruptly at the walls.
    log('  step: terrain')
    box('OuterTerrain', (0, 0, -95), (sx * 2.8, sy * 2.8, 150), mats['terrain'], block=False)
    for i, (bx, by) in enumerate(((0.78, 0.55), (-0.74, 0.62), (0.6, -0.78), (-0.66, -0.72))):
        rr = 280.0 + i * 45.0
        box('Boulder_%d' % i, (sx * bx, sy * by, rr * 0.55), (rr * 2.6, rr * 2.4, rr * 1.9),
            mats['concrete_dark'], rot_z=23.0 * i, mesh=SPHERE_MESH, block=False)

    log('  step: atmosphere')
    add_atmosphere(*cfg['sky'])
    # Broad fills so the play space reads clearly even when the sun is low.
    add_light_ki(True, (0.85, 0.90, 1.0), 15000.0, 3600.0, (-sx * 0.5 + 600, -300, 950))
    add_light_ki(True, (0.85, 0.90, 1.0), 9000.0, 2800.0, (sx * 0.3, sy * 0.3, 950))
    player_start(-sx * 0.5 + 600, -300, 0)
    nav_bounds(sx, sy, h)
    log('  step: props')
    cfg['props'](mats, cfg)

    log('  step: notes')
    for (title, body, loc) in cfg['notes']:
        note = spawn(unreal.StoryNote, loc, label='Note')
        if note:
            setp(note, 'note_title', title)
            setp(note, 'note_body', body)
            setp(note, 'display_seconds', 16.0)
            comp = getp(note, ['visual_mesh'])
            if comp:
                comp.set_static_mesh(load_asset(CUBE_MESH))
                comp.set_material(0, mats['cyan'])

    log('  step: triggers')
    for (loc, beat_id) in cfg['triggers']:
        trigger = spawn(unreal.StoryTrigger, loc, label='StoryTrigger')
        if trigger:
            setp(trigger, 'beat_id', beat_id)
            setp(trigger, ['trigger_once', 'b_trigger_once'], True)

    log('  step: spawner')
    spawner = spawn(unreal.ZombieWaveSpawner, cfg['spawner_loc'], label='WaveSpawner')
    if spawner:
        zombie_cls = load_class_any(ZOMBIE_PATHS)
        if zombie_cls:
            setp(spawner, 'zombie_class', zombie_cls)
        else:
            warn('Zombie blueprint not found; wave spawner left unconfigured.')
        for key, value in cfg['spawner'].items():
            setp(spawner, key, value)

    log('  step: director')
    director = spawn(unreal.StoryDirector, (0, 0, 0), label='StoryDirector')
    if director:
        setp(director, 'chapter_id', cfg['chapter'])
        setp(director, 'start_delay', 2.5)
        setp(director, 'travel_delay', cfg.get('travel_delay', 10.0))
        setp(director, 'next_map', cfg.get('next_map', ''))
        setp(director, 'finale_beat_id', cfg['spawner'].get('all_cleared_beat_id', ''))

    log('  step: ambience')
    ambience = spawn(unreal.LevelAmbience, (0, 0, 200), label='Ambience')
    if ambience and cfg.get('ambience'):
        sound_path, volume, pitch = cfg['ambience']
        sound = load_asset(sound_path)
        if sound:
            setp(ambience, 'ambient_sound', sound)
        else:
            warn('Missing ambience sound ' + sound_path)
        setp(ambience, 'volume', volume)
        setp(ambience, 'pitch', pitch)

    log('  step: save')
    return save_current_level()


# --------------------------------------------------------------------------------------
# Campaign database
# --------------------------------------------------------------------------------------

def make_beat(beat_id, kind, speaker, text, auto, voice=None):
    beat = unreal.StoryBeat()
    setp(beat, 'beat_id', beat_id)
    setp(beat, 'kind', beatkind(kind))
    setp(beat, 'speaker', speaker)
    setp(beat, 'text', text)
    setp(beat, ['auto_play', 'b_auto_play'], bool(auto))
    if voice:
        sound = load_asset(voice)
        if sound:
            setp(beat, 'voice', sound)
        else:
            warn('Missing voice asset ' + voice)
    return beat


def build_campaign():
    db = load_asset(DB_PATH)
    if db is None:
        # The asset can be left behind by a crashed run; reclaim the package rather than
        # create_asset (which refuses while the stale package is still registered).
        if unreal.EditorAssetLibrary.does_asset_exist(DB_PATH):
            unreal.EditorAssetLibrary.delete_asset(DB_PATH)
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.StoryDatabase)
        db = tools().create_asset('DA_Campaign', '/Game/Story', unreal.StoryDatabase, factory)
    if db is None:
        warn('Could not create DA_Campaign')
        return False

    chapters = []
    voices = import_voices()
    for cfg in CHAPTERS:
        chapter = unreal.StoryChapter()
        setp(chapter, 'chapter_id', cfg['chapter'])
        setp(chapter, 'title', cfg['title'])
        setp(chapter, 'subtitle', cfg['subtitle'])
        setp(chapter, 'objective', cfg['objective'])

        beats = []
        for (beat_id, kind, speaker, text, auto) in cfg['beats']:
            beats.append(make_beat(beat_id, kind, speaker, text, auto, voices.get(beat_id)))
        setp(chapter, 'beats', beats)
        chapters.append(chapter)

    setp(db, 'chapters', chapters)
    unreal.EditorAssetLibrary.save_asset(DB_PATH)
    log('Campaign database written with %d chapters.' % len(chapters))
    return True


def main():
    mats = build_materials()
    if not mats.get('cyan'):
        warn('Materials missing; aborting.')
        return

    for cfg in CHAPTERS:
        log('Building ' + cfg['map'])
        try:
            ok = build_level(cfg, mats)
            log('  level %s: %s' % (cfg['map'], 'saved' if ok else 'NOT saved'))
        except Exception:
            warn('Level %s failed:\n%s' % (cfg['map'], traceback.format_exc()))

    try:
        build_campaign()
    except Exception:
        warn('Campaign build failed:\n%s' % traceback.format_exc())

    log('Build finished.')


if __name__ == '__main__':
    main()
