import unreal

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_editor.load_level("/Game/Maps/Ch01")

plain = EAL.load_asset("/Engine/EngineMaterials/DefaultMaterial")
count = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_actor_label().startswith("Floor"):
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_material(0, plain)
            count += 1
        unreal.log("[FloorTest] Applied DefaultMaterial to %s" % actor.get_actor_label())

level_editor.save_current_level()
unreal.log("[FloorTest] done, comps=%d" % count)
