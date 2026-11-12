"""Import Poly Haven PBR textures (2k JPG) and build a world-aligned master material + MIs.

Run headless:
  UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript=<this> -RenderOffscreen -windowed ...
"""
import os
import unreal

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

TEX_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'Textures')
DEST = '/Game/Levels/Textures'
MAT_ROOT = '/Game/Levels/Materials'
WA_FUNC = '/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture'
WA_NORMAL_FUNC = '/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedNormal'

# texture id -> (material instance name, world tile size cm, image basename)
TEX_SETS = [
    ('concrete_wall_003', 'MI_Concrete', 300.0),
    ('cracked_concrete_wall', 'MI_CrackedConcrete', 350.0),
    ('garage_floor', 'MI_GarageFloor', 400.0),
    ('dirty_concrete', 'MI_DirtyConcrete', 500.0),
    ('metal_plate', 'MI_MetalPlate', 220.0),
    ('rusty_metal_02', 'MI_RustyMetal', 300.0),
    ('wood_planks_dirt', 'MI_WoodPlanks', 260.0),
    ('asphalt_02', 'MI_Asphalt', 600.0),
    ('forest_ground_04', 'MI_ForestGround', 900.0),
]

MAP_SUFFIX = {'diff': 'D', 'nor_gl': 'N', 'arm': 'ARM'}


def log(msg):
    print('[Tex] ' + msg)


def import_texture(tex_id, suffix):
    base = '%s_%s_2k.jpg' % (tex_id, suffix)
    src = os.path.join(TEX_DIR, tex_id, base)
    if not os.path.exists(src):
        log('missing source ' + src)
        return None
    name = 'T_%s_%s' % (tex_id, MAP_SUFFIX[suffix])
    path = '%s/%s' % (DEST, name)
    if not EAL.does_asset_exist(path):
        task = unreal.AssetImportTask()
        task.filename = src
        task.destination_path = DEST
        task.destination_name = name
        task.automated = True
        task.replace_existing = True
        task.save = True
        factory = unreal.TextureFactory()
        try:
            factory.set_editor_property('srgb', suffix == 'diff')
        except Exception:
            pass
        if suffix == 'nor_gl':
            try:
                factory.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_NORMALMAP)
            except Exception:
                pass
        elif suffix == 'arm':
            try:
                factory.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_MASKS)
            except Exception:
                pass
        task.factory = factory
        AT.import_asset_tasks([task])
    tex = unreal.load_asset(path)
    if tex is None:
        log('import failed ' + path)
        return None
    # enforce settings on the asset as well (factory may ignore them)
    try:
        tex.set_editor_property('srgb', suffix == 'diff')
        if suffix == 'nor_gl':
            tex.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_NORMALMAP)
        elif suffix == 'arm':
            tex.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_MASKS)
        tex.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_WORLD)
        tex.update_resource()
        EAL.save_asset(path)
    except Exception as exc:
        log('setting tweak failed %s: %s' % (name, exc))
    return tex


def build_master():
    path = MAT_ROOT + '/M_WorldAligned'
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    mat = AT.create_asset('M_WorldAligned', MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    if mat is None:
        log('master material creation failed')
        return None
    fn = unreal.load_asset(WA_FUNC)
    fn_nrm = unreal.load_asset(WA_NORMAL_FUNC)

    def param(cls, name, x, y, default=None):
        node = MEL.create_material_expression(mat, cls, x, y)
        node.set_editor_property('parameter_name', name)
        if default is not None:
            node.set_editor_property('default_value', default)
        return node

    p_alb = param(unreal.MaterialExpressionTextureObjectParameter, 'Albedo', -1500, -350)
    p_arm = param(unreal.MaterialExpressionTextureObjectParameter, 'ARM', -1500, -80)
    p_nrm = param(unreal.MaterialExpressionTextureObjectParameter, 'NormalMap', -1500, 200)
    p_tile = param(unreal.MaterialExpressionScalarParameter, 'TileSize', -1500, 420, 400.0)

    def wa_call(func, y):
        c = MEL.create_material_expression(mat, unreal.MaterialExpressionMaterialFunctionCall, -1150, y)
        c.set_editor_property('material_function', func)
        return c

    call_alb = wa_call(fn, -350)
    call_arm = wa_call(fn, -80)
    for call, p in ((call_alb, p_alb), (call_arm, p_arm)):
        ok = MEL.connect_material_expressions(p, '', call, 'TextureObject')
        log('connect %s texture: %s' % (p.get_editor_property('parameter_name'), ok))
        for in_name in ('TextureSize', 'Size', 'TextureTiling'):
            try:
                if MEL.connect_material_expressions(p_tile, '', call, in_name):
                    log('tile connected as ' + in_name)
                    break
            except Exception:
                pass

    # albedo -> base color
    ok = MEL.connect_material_property(call_alb, '', unreal.MaterialProperty.MP_BASE_COLOR)
    if not ok:
        for out in ('RGB', 'RGBA', 'Color'):
            try:
                if MEL.connect_material_property(call_alb, out, unreal.MaterialProperty.MP_BASE_COLOR):
                    log('albedo out ' + out)
                    break
            except Exception:
                pass

    def channel_mask(call, keep, x, y):
        m = MEL.create_material_expression(mat, unreal.MaterialExpressionComponentMask, x, y)
        for c in ('r', 'g', 'b', 'a'):
            m.set_editor_property(c, c in keep)
        for out in ('', 'RGB', 'RGBA'):
            try:
                if MEL.connect_material_expressions(call, out, m, ''):
                    break
            except Exception:
                pass
        return m

    rough = channel_mask(call_arm, 'g', -800, 20)
    metal = channel_mask(call_arm, 'b', -800, 150)
    ao = channel_mask(call_arm, 'r', -800, 280)
    MEL.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(metal, '', unreal.MaterialProperty.MP_METALLIC)

    # albedo * ao -> base color (override albedo connect)
    mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -700, -300)
    MEL.connect_material_expressions(call_alb, '', mul, 'A')
    MEL.connect_material_expressions(ao, '', mul, 'B')
    if MEL.connect_material_property(mul, '', unreal.MaterialProperty.MP_BASE_COLOR):
        log('base color = albedo * ao')

    # normal map (best effort)
    if fn_nrm:
        call_nrm = wa_call(fn_nrm, 200)
        ok = MEL.connect_material_expressions(p_nrm, '', call_nrm, 'TextureObject')
        log('normal texture: %s' % ok)
        if ok:
            for out in ('', 'RGB', 'Normal'):
                try:
                    if MEL.connect_material_property(call_nrm, out, unreal.MaterialProperty.MP_NORMAL):
                        log('normal connected')
                        break
                except Exception:
                    pass

    MEL.recompile_material(mat)
    try:
        stats = MEL.get_statistics(mat)
        log('master stats: %s' % stats)
    except Exception:
        pass
    EAL.save_asset(MAT_ROOT + '/M_WorldAligned')
    return mat


def build_mis(master):
    created = {}
    for tex_id, mi_name, tile in TEX_SETS:
        d = import_texture(tex_id, 'diff')
        n = import_texture(tex_id, 'nor_gl')
        a = import_texture(tex_id, 'arm')
        if not (d and a):
            log('skip MI %s (missing textures)' % mi_name)
            continue
        path = MAT_ROOT + '/' + mi_name
        if EAL.does_asset_exist(path):
            EAL.delete_asset(path)
        mi = AT.create_asset(mi_name, MAT_ROOT, unreal.MaterialInstanceConstant,
                             unreal.MaterialInstanceConstantFactoryNew())
        if mi is None:
            log('MI creation failed ' + mi_name)
            continue
        MEL.set_material_instance_parent(mi, master)
        MEL.set_material_instance_texture_parameter_value(mi, 'Albedo', d)
        MEL.set_material_instance_texture_parameter_value(mi, 'ARM', a)
        if n:
            MEL.set_material_instance_texture_parameter_value(mi, 'NormalMap', n)
        MEL.set_material_instance_scalar_parameter_value(mi, 'TileSize', tile)
        EAL.save_asset(path)
        created[mi_name] = mi
        log('MI built ' + path)
    return created


def main():
    EAL.make_directory(DEST)
    master = build_master()
    if master is None:
        return
    mis = build_mis(master)
    log('MIs built: %d' % len(mis))
    log('DONE')


main()
