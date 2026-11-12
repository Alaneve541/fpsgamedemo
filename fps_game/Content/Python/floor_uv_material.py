import unreal

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

MAT_ROOT = "/Game/Levels/Materials"
TEX_ROOT = "/Game/Levels/Textures"


def log(msg):
    unreal.log("[TiledUV] " + str(msg))


def build_master():
    path = MAT_ROOT + "/M_TiledUV"
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    mat = AT.create_asset("M_TiledUV", MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    if mat is None:
        log("material creation failed")
        return None
    mat.set_editor_property("used_with_nanite", True)

    def sample_param(name, y, sampler):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -900, y)
        n.set_editor_property("parameter_name", name)
        n.set_editor_property("sampler_type", sampler)
        return n

    p_alb = sample_param("Albedo", -400, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    p_arm = sample_param("ARM", -100, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    p_nrm = sample_param("NormalMap", 200, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)

    p_tiles = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1500, 460)
    p_tiles.set_editor_property("parameter_name", "UVTiles")
    p_tiles.set_editor_property("default_value", 24.0)

    tc = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1500, 320)
    mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -1250, 380)
    ok_a = MEL.connect_material_expressions(tc, "", mul, "A")
    ok_b = MEL.connect_material_expressions(p_tiles, "", mul, "B")
    log("uv multiply %s %s" % (ok_a, ok_b))

    for p in (p_alb, p_arm, p_nrm):
        for in_name in ("UVs", "UV"):
            try:
                if MEL.connect_material_expressions(mul, "", p, in_name):
                    log("uv -> %s" % p.get_editor_property("parameter_name"))
                    break
            except Exception:
                pass

    def mask(p, keep, x, y):
        m = MEL.create_material_expression(mat, unreal.MaterialExpressionComponentMask, x, y)
        for c in ("r", "g", "b", "a"):
            m.set_editor_property(c, c in keep)
        MEL.connect_material_expressions(p, "", m, "")
        return m

    ao = mask(p_arm, "r", -550, 300)
    rough = mask(p_arm, "g", -550, 120)
    metal = mask(p_arm, "b", -550, -20)
    MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)

    nrm_rgb = mask(p_nrm, "rgb", -550, 460)
    nrm_ok = MEL.connect_material_property(nrm_rgb, "", unreal.MaterialProperty.MP_NORMAL)
    log("normal connect %s" % nrm_ok)

    mul_col = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -550, -400)
    MEL.connect_material_expressions(p_alb, "", mul_col, "A")
    MEL.connect_material_expressions(ao, "", mul_col, "B")
    MEL.connect_material_property(mul_col, "", unreal.MaterialProperty.MP_BASE_COLOR)

    MEL.recompile_material(mat)
    EAL.save_asset(mat.get_path_name())
    log("M_TiledUV built")
    return mat


def retile(mi_path, albedo_tex, tiles):
    mi = EAL.load_asset(mi_path)
    if mi is None:
        log("missing " + mi_path)
        return
    master = EAL.load_asset(MAT_ROOT + "/M_TiledUV")
    if mi.get_editor_property("parent") != master:
        mi.set_editor_property("parent", master)
    norm = EAL.load_asset(TEX_ROOT + "/T_FlatNormal")
    arm = EAL.load_asset(TEX_ROOT + "/T_FlatARM")
    alb = EAL.load_asset(albedo_tex)
    MEL.set_material_instance_texture_parameter_value(mi, "Albedo", alb)
    MEL.set_material_instance_texture_parameter_value(mi, "NormalMap", norm)
    MEL.set_material_instance_texture_parameter_value(mi, "ARM", arm)
    MEL.set_material_instance_scalar_parameter_value(mi, "UVTiles", float(tiles))
    MEL.update_material_instance(mi)
    EAL.save_asset(mi.get_path_name())
    log("retiled %s tiles=%d" % (mi_path, tiles))


mat = build_master()
if mat:
    retile(MAT_ROOT + "/MI_Asphalt", TEX_ROOT + "/T_asphalt_02_D", 26)
    retile(MAT_ROOT + "/MI_GarageFloor", TEX_ROOT + "/T_garage_floor_D", 26)
    retile(MAT_ROOT + "/MI_DirtyConcrete", TEX_ROOT + "/T_dirty_concrete_D", 20)

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.load_level("/Game/Maps/Ch01")
mi_asphalt = EAL.load_asset(MAT_ROOT + "/MI_Asphalt")
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_actor_label().startswith("Floor"):
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_material(0, mi_asphalt)
            log("restored floor material on " + actor.get_actor_label())
level_editor.save_current_level()
log("done")
