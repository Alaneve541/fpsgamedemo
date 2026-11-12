import struct, zlib, os
import unreal

def write_png(path, rgb, size=8):
    raw = b''.join(b'\x00' + bytes(rgb) * size for _ in range(size))
    def chunk(tag, data):
        c = struct.pack('>I', len(data)) + tag + data
        return c + struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff)
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', size, size, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(raw, 9))
    png += chunk(b'IEND', b'')
    with open(path, 'wb') as f:
        f.write(png)

SRC = "C:/Users/alane/Desktop/111/fps_game 5.7/Content/Textures/flat"
os.makedirs(SRC, exist_ok=True)
arm_path = os.path.join(SRC, "flat_arm_rough.png")
write_png(arm_path, (255, 255, 0))
print("[Flat] wrote", arm_path)

AT = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

factory = unreal.TextureFactory()
factory.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
task = unreal.AssetImportTask()
task.filename = arm_path
task.destination_path = "/Game/Levels/Textures"
task.destination_name = "T_FlatARM"
task.replace_existing = True
task.automated = True
task.save = True
task.factory = factory
AT.import_asset_tasks([task])
arm = EAL.load_asset("/Game/Levels/Textures/T_FlatARM")
EAL.save_loaded_asset(arm, False)

for mi_name in ("MI_Asphalt", "MI_GarageFloor", "MI_DirtyConcrete", "MI_ForestGround"):
    mi = EAL.load_asset("/Game/Levels/Materials/" + mi_name)
    if mi:
        MEL.set_material_instance_texture_parameter_value(mi, "ARM", arm)
        MEL.update_material_instance(mi)
        EAL.save_loaded_asset(mi, False)
        print("[Flat] roughened", mi_name)
