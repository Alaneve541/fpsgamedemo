import unreal

mi = unreal.load_asset("/Game/Levels/Materials/MI_Asphalt")
print("[MI] class", mi.get_class().get_name())
props = [p for p in dir(mi) if ('param' in p.lower() or 'texture' in p.lower() or 'scalar' in p.lower())]
print("[MI] props", props)
for p in ("texture_parameter_values", "scalar_parameter_values", "vector_parameter_values"):
    try:
        v = mi.get_editor_property(p)
        print("[MI]", p, "=", v)
    except Exception as e:
        print("[MI]", p, "ERR", e)

MEL = unreal.MaterialEditingLibrary
for name in ("get_material_instance_texture_parameter_value", "get_material_instance_scalar_parameter_value"):
    fn = getattr(MEL, name, None)
    print("[MI] fn", name, fn)
    if fn:
        try:
            print("   ->", fn(mi, "Albedo") if "texture" in name else fn(mi, "TileSize"))
        except Exception as e:
            print("   ERR", e)
