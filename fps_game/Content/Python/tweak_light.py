import unreal
EAL = unreal.EditorAssetLibrary
EAL.load_asset("/Game/Maps/Ch01")
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    label = a.get_actor_label()
    if label == "Sun":
        a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(8000.0)
        print("[Tweak] sun 8000")
    if label == "SkyLight":
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_intensity(1.0)
        print("[Tweak] sky 1")
    if label == "PostProcess":
        s = a.get_editor_property("settings")
        for prop, val in (("override_auto_exposure_bias", True), ("auto_exposure_bias", -1.0),
                          ("override_auto_exposure_min_brightness", True), ("auto_exposure_min_brightness", 0.03),
                          ("override_auto_exposure_max_brightness", True), ("auto_exposure_max_brightness", 8.0)):
            try:
                s.set_editor_property(prop, val)
            except Exception as e:
                print("[Tweak] pp ERR", prop, e)
        a.set_editor_property("settings", s)
        print("[Tweak] pp adjusted")
print("[Tweak] save", unreal.EditorLevelLibrary.save_current_level())
