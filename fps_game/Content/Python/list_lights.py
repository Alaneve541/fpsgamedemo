import unreal
EAL = unreal.EditorAssetLibrary
EAL.load_asset("/Game/Maps/Ch01")
world = unreal.EditorLevelLibrary.get_editor_world()
print("[Lvl]", world.get_name())
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    n = a.get_actor_label()
    cls = a.get_class().get_name()
    if cls in ("DirectionalLight", "SkyLight", "SkyAtmosphere", "ExponentialHeightFog", "PostProcessVolume", "PointLight", "SpotLight"):
        extra = ""
        for attr in ("intensity", "atmosphere_sun_light", "rotation"):
            try:
                v = a.get_editor_property(attr) if attr != "rotation" else a.get_actor_rotation()
                extra += " %s=%s" % (attr, v)
            except Exception:
                pass
        try:
            comp = a.get_component_by_class(unreal.LightComponent)
            if comp:
                extra += " intensity=%.1f" % comp.get_intensity()
        except Exception:
            pass
        print("[Lvl] %-22s %-28s %s" % (n, cls, extra))
