import unreal

MEL = unreal.MaterialEditingLibrary

def dump_mi(path):
    mi = unreal.load_asset(path)
    if not mi:
        print("[Dump] missing", path); return
    print(f"[Dump] {path} parent={mi.parent.get_name() if mi.parent else None}")
    for pname in ["TileSize", "Albedo", "ARM", "NormalMap"]:
        try:
            v = MEL.get_material_instance_scalar_parameter_value(mi, pname)
            print(f"    scalar {pname} = {v}")
        except Exception:
            pass
        try:
            t = MEL.get_material_instance_texture_parameter_value(mi, pname)
            print(f"    tex {pname} = {t.get_name() if t else None}")
        except Exception:
            pass

for p in ["/Game/Levels/Materials/MI_Asphalt", "/Game/Levels/Materials/MI_Concrete",
          "/Game/Levels/Materials/MI_GarageFloor", "/Game/Levels/Materials/MI_ForestGround"]:
    dump_mi(p)

print("[Dump] --- Ch01 static meshes ---")
world = unreal.EditorLevelLibrary.get_editor_world()
for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StaticMeshActor):
    label = a.get_actor_label()
    comp = a.static_mesh_component
    mesh = comp.static_mesh.get_name() if comp.static_mesh else None
    mats = []
    for s in range(max(1, comp.get_num_materials())):
        mm = comp.get_material(s)
        mats.append(mm.get_name() if mm else "None")
    print(f"  {label} mesh={mesh} mats={mats} scale={comp.get_world_scale()}")
print("[Dump] done")
