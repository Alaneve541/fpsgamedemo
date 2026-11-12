import unreal
unreal.EditorAssetLibrary.load_asset("/Game/Maps/Ch01")
world = unreal.EditorLevelLibrary.get_editor_world()
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    label = a.get_actor_label()
    if label == "Sun":
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        for p in ("atmosphere_sun_light", "intensity", "mobility", "light_color", "dynamic_shadow_distance_movable_light"):
            try:
                print("[Sun]", p, c.get_editor_property(p))
            except Exception as e:
                print("[Sun]", p, "ERR", e)
        print("[Sun] comp class", c.get_class().get_name())
    if label == "SkyAtmosphere":
        for p in ("transform_mode", "atmosphere_height", "rayleigh_scattering_scale", "mie_scattering_scale"):
            try:
                print("[Atmo]", p, a.get_editor_property(p))
            except Exception as e:
                print("[Atmo]", p, "ERR", e)
