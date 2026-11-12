import unreal

TEX_ROOT = "/Game/Levels/Textures"
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary

def import_tex(src, name, compressed):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src)
    task.set_editor_property("destination_path", TEX_ROOT)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    factory = unreal.TextureFactory()
    factory.set_editor_property("compression_settings", compressed)
    task.set_editor_property("factory", factory)
    AT.import_asset_tasks([task])
    tex = unreal.load_asset(TEX_ROOT + "/" + name)
    print("[Flat] imported", name, tex)
    return tex

flat_n = import_tex("C:/Users/alane/Desktop/111/fps_game 5.7/Content/Textures/flat/flat_normal.png",
                    "T_FlatNormal", unreal.TextureCompressionSettings.TC_NORMALMAP)
flat_a = import_tex("C:/Users/alane/Desktop/111/fps_game 5.7/Content/Textures/flat/flat_arm.png",
                    "T_FlatARM", unreal.TextureCompressionSettings.TC_MASKS)

for mi_path in ("/Game/Levels/Materials/MI_Asphalt", "/Game/Levels/Materials/MI_GarageFloor",
                "/Game/Levels/Materials/MI_DirtyConcrete"):
    mi = unreal.load_asset(mi_path)
    if not mi:
        print("[Flat] missing", mi_path); continue
    MEL.set_material_instance_texture_parameter_value(mi, "NormalMap", flat_n)
    MEL.set_material_instance_texture_parameter_value(mi, "ARM", flat_a)
    MEL.update_material_instance(mi)
    EAL.save_asset(mi_path)
    print("[Flat] updated", mi_path)

# verify readback
mi = unreal.load_asset("/Game/Levels/Materials/MI_Asphalt")
print("[Flat] readback", MEL.get_material_instance_texture_parameter_value(mi, "NormalMap"))
