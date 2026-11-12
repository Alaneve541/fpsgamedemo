import unreal

SRC = "C:/Users/alane/Desktop/111/fps_game 5.7/Content/Models/Guns/baked/"
DEST = "/Game/Models/Guns"

for name in ["gun_virtus", "gun_k9", "gun_m90", "gun_dmr"]:
    task = unreal.AssetImportTask()
    task.filename = SRC + name + ".obj"
    task.destination_path = DEST
    task.destination_name = "SM_" + name
    task.automated = True
    task.replace_existing = True
    task.save = True
    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        print("[GunImport] imported", name, "->", task.imported_object_paths)
    except Exception as e:
        print("[GunImport] FAILED", name, e)

for name in ["gun_virtus", "gun_k9", "gun_m90", "gun_dmr"]:
    m = unreal.load_asset(DEST + "/SM_" + name)
    if m:
        b = m.get_bounds().box_extent
        print("[GunImport]", name, "extent_cm", b, "mats", len(m.get_editor_property("static_materials")))
    else:
        print("[GunImport] missing", name)
