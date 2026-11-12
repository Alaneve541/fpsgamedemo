import unreal

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MAT_ROOT = "/Game/Levels/Materials"


def log(msg):
    unreal.log("[FlatFloor] " + str(msg))


path = MAT_ROOT + "/M_FloorSimple"
if EAL.does_asset_exist(path):
    EAL.delete_asset(path)
mat = AT.create_asset("M_FloorSimple", MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("used_with_nanite", True)

col = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -600, -200)
col.set_editor_property("constant", unreal.LinearColor(0.022, 0.021, 0.020, 1.0))
MEL.connect_material_property(col, "", unreal.MaterialProperty.MP_BASE_COLOR)

rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -600, 0)
rough.set_editor_property("r", 0.88)
MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

spec = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -600, 120)
spec.set_editor_property("r", 0.25)
try:
    MEL.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)
except Exception as exc:
    log("specular skipped: %s" % exc)

MEL.recompile_material(mat)
EAL.save_asset(mat.get_path_name())
log("M_FloorSimple built")

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.load_level("/Game/Maps/Ch01")
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_actor_label().startswith("Floor"):
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_material(0, mat)
            log("floor -> M_FloorSimple")
level_editor.save_current_level()
log("done")
