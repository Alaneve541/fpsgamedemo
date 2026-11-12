import os
import unreal

AT = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

SRC = "C:/Users/alane/Desktop/111/fps_game 5.7/Content/Textures/flat"
DST = "/Game/Levels/Textures"


def import_flat(filename, dest_name, compression):
    factory = unreal.TextureFactory()
    factory.set_editor_property("compression_settings", compression)
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = DST
    task.destination_name = dest_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.factory = factory
    AT.import_asset_tasks([task])
    tex = EAL.load_asset(DST + "/" + dest_name)
    if tex:
        EAL.save_loaded_asset(tex, False)
        print("[Flat] imported", dest_name,
              "comp=", tex.get_editor_property("compression_settings"))
    else:
        print("[Flat] FAILED", dest_name)
    return tex


normal = import_flat(os.path.join(SRC, "flat_normal.png"), "T_FlatNormal",
                     unreal.TextureCompressionSettings.TC_NORMALMAP)
arm = import_flat(os.path.join(SRC, "flat_arm.png"), "T_FlatARM",
                  unreal.TextureCompressionSettings.TC_MASKS)

for mi_name in ("MI_Asphalt", "MI_GarageFloor", "MI_DirtyConcrete", "MI_ForestGround"):
    mi = EAL.load_asset("/Game/Levels/Materials/" + mi_name)
    if not mi:
        print("[Flat] missing MI", mi_name)
        continue
    if normal:
        MEL.set_material_instance_texture_parameter_value(mi, "NormalMap", normal)
    if arm:
        MEL.set_material_instance_texture_parameter_value(mi, "ARM", arm)
    MEL.update_material_instance(mi)
    EAL.save_loaded_asset(mi, False)
    print("[Flat] patched", mi_name)

print("[Flat] exists normal:", EAL.does_asset_exist(DST + "/T_FlatNormal"),
      "arm:", EAL.does_asset_exist(DST + "/T_FlatARM"))
