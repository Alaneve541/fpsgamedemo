import unreal
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary

for p in ("/Game/Levels/Materials/MI_Asphalt", "/Game/Levels/Materials/MI_GarageFloor"):
    mi = unreal.load_asset(p)
    print("[MI]", p)
    for name in ("Albedo", "ARM", "NormalMap"):
        print("   ", name, MEL.get_material_instance_texture_parameter_value(mi, name))
    print("    TileSize", MEL.get_material_instance_scalar_parameter_value(mi, "TileSize"))

EAL.load_asset("/Game/Maps/Ch01")
print("[MI] loaded", EAL.load_asset("/Game/Maps/Ch01"))
world = unreal.EditorLevelLibrary.get_editor_world()
print("[MI] world", world.get_name())
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label() == "Floor":
        comp = a.static_mesh_component
        print("[MI] Floor actor scale", a.get_actor_scale3d())
        for s in range(max(1, comp.get_num_materials())):
            m = comp.get_material(s)
            print("     slot", s, m.get_name() if m else None)
        print("     bounds", comp.bounds)
