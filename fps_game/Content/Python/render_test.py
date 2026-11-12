import unreal
import math

CUBE = "/Engine/BasicShapes/Cube.Cube"

def box(label, loc, size, mat):
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*loc), unreal.Rotator(0, 0, 0))
    a.set_actor_label(label)
    c = a.static_mesh_component
    c.set_static_mesh(unreal.load_asset(CUBE))
    a.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    c.set_material(0, unreal.load_asset(mat))
    return a

def light(cls, loc, rot, intensity, extra=None, comp_cls=unreal.LightComponent):
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(*rot))
    comp = a.get_component_by_class(comp_cls)
    comp.set_intensity(intensity)
    if extra:
        extra(comp)
    return a

def shoot(eye, target, out_name):
    world = unreal.EditorLevelLibrary.get_editor_world()
    rt = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    ca = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(*eye), unreal.Rotator(0, 0, 0))
    cap = ca.capture_component2d
    cap.set_editor_property("texture_target", rt)
    cap.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    cap.set_editor_property("capture_every_frame", False)
    cap.set_editor_property("fov_angle", 90.0)
    dx, dy, dz = target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]
    yaw = math.degrees(math.atan2(dy, dx))
    pitch = math.degrees(math.atan2(dz, math.hypot(dx, dy)))
    ca.set_actor_rotation(unreal.Rotator(pitch, yaw, 0.0), False)
    cap.capture_scene()
    unreal.RenderingLibrary.export_render_target(world, rt, "C:/Users/alane/Desktop/111/fps_game 5.7/Saved", out_name)
    print("[Render]", out_name)

box("Floor", (0, 0, 0), (8000, 8000, 20), "/Game/Levels/Materials/MI_Asphalt")
box("Wall", (0, 1500, 400), (4000, 40, 800), "/Game/Levels/Materials/MI_Concrete")
box("Crate", (-500, 300, 60), (100, 100, 120), "/Game/Levels/Materials/MI_WoodPlanks")

light(unreal.DirectionalLight, (0, 0, 900), (-50, -30, 0), 5.0)
light(unreal.SkyLight, (0, 0, 600), (0, 0, 0), 1.0,
      lambda c: c.set_editor_property("lower_hemisphere_is_black", False),
      unreal.SkyLightComponent)

shoot((0, -1200, 300), (0, 600, 60), "PreviewA.png")
shoot((0, -400, 900), (0, 200, 0), "PreviewB.png")
