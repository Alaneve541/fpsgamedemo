import unreal
MEL = unreal.MaterialEditingLibrary
print("[MEL]", [m for m in dir(MEL) if 'instance' in m.lower()])
tex = unreal.load_asset("/Game/Levels/Textures/T_FlatNormal") if unreal.EditorAssetLibrary.does_asset_exist("/Game/Levels/Textures/T_FlatNormal") else None
print("[TEX]", tex)
if tex:
    for p in ("srgb", "compression_settings", "lod_group"):
        try:
            print("   ", p, tex.get_editor_property(p))
        except Exception as e:
            print("   ", p, "ERR", e)
mi = unreal.load_asset("/Game/Levels/Materials/MI_Asphalt")
print("[MIparam]", [m for m in dir(mi) if 'set' in m.lower() and 'param' in m.lower()])
