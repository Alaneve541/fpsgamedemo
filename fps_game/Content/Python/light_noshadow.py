import unreal

EAL = unreal.EditorAssetLibrary
level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.load_level("/Game/Maps/Ch01")

changed = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    for light in actor.get_components_by_class(unreal.PointLightComponent):
        light.set_editor_property("cast_shadows", False)
        changed += 1
    for light in actor.get_components_by_class(unreal.SpotLightComponent):
        light.set_editor_property("cast_shadows", False)
        changed += 1
unreal.log("[NoShadow] changed %d lights" % changed)
level_editor.save_current_level()
