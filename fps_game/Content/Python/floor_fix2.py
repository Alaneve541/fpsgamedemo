import unreal

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
MAT_ROOT = "/Game/Levels/Materials"
TEX_ROOT = "/Game/Levels/Textures"


def log(msg):
    unreal.log("[FloorFix2] " + str(msg))


master = EAL.load_asset(MAT_ROOT + "/M_WorldAligned")
norm = EAL.load_asset(TEX_ROOT + "/T_FlatNormal")
arm = EAL.load_asset(TEX_ROOT + "/T_FlatARM")

KEEP = [("MI_Asphalt", "/T_asphalt_02_D", 600.0),
        ("MI_GarageFloor", "/T_garage_floor_D", 400.0),
        ("MI_DirtyConcrete", "/T_dirty_concrete_D", 500.0)]
for name, alb, tile in KEEP:
    mi = EAL.load_asset(MAT_ROOT + "/" + name)
    if mi is None:
        log("missing " + name)
        continue
    mi.set_editor_property("parent", master)
    MEL.set_material_instance_texture_parameter_value(mi, "Albedo", EAL.load_asset(TEX_ROOT + alb))
    MEL.set_material_instance_texture_parameter_value(mi, "NormalMap", norm)
    MEL.set_material_instance_texture_parameter_value(mi, "ARM", arm)
    MEL.set_material_instance_scalar_parameter_value(mi, "TileSize", tile)
    MEL.update_material_instance(mi)
    EAL.save_asset(mi.get_path_name())
    log("restored parent of " + name)

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.load_level("/Game/Maps/Ch01")
concrete = EAL.load_asset(MAT_ROOT + "/MI_Concrete")
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_actor_label().startswith("Floor"):
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_material(0, concrete)
            log("floor -> MI_Concrete")
level_editor.save_current_level()
log("done")
