import bpy
import sys

argv=sys.argv
argv=argv[argv.index("--") + 1:]

print("Processing " + bpy.context.blend_data.filepath)
filename=bpy.context.blend_data.filepath.split("/")[-1].split(".")[0]
filename=argv[0] + "/" + filename

for i in bpy.data.objects:
    bpy.context.scene.objects.active = i
    for mod in i.modifiers:
        if isinstance(mod, bpy.types.SubsurfModifier) or isinstance(mod, bpy.types.MirrorModifier):
            bpy.ops.object.modifier_apply(modifier=mod.name)

win=bpy.context.window_manager.windows[0]
area=win.screen.areas[0]
area.type='NLA_EDITOR'

C_dict=bpy.context.copy()
C_dict.update(area=area)

anim=False
for obj in bpy.data.objects:
    if obj.animation_data:
        tracks=obj.animation_data.nla_tracks
        for nla in tracks:
            anim=True
            print("Found nla track: " + nla.name)
            tracks.active=nla
            start_frames = []
            end_frames = []
            for strip in nla.strips:
                strip.select=True
                start_frames.append(strip.frame_start)
                end_frames.append(strip.frame_end)
            bpy.context.scene.frame_start = min(start_frames)
            bpy.context.scene.frame_end = max(end_frames) - 1
            bpy.ops.nla.tweakmode_enter(C_dict)
            out=filename + "-" + nla.name + ".ogex"
            print("Exporting to: " + out)
            bpy.ops.export_scene.ogex(C_dict, filepath=out)
            bpy.ops.nla.tweakmode_exit(C_dict)

if not anim:
    out=filename + ".ogex"
    print("Exporting to: " + out)
    bpy.ops.export_scene.ogex(C_dict, filepath=out)
