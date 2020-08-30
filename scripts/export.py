import bpy
import sys

argv=sys.argv
argv=argv[argv.index("--") + 1:]

print("Processing " + bpy.context.blend_data.filepath)
filename=argv[0]

bpy.ops.export_scene.gltf(export_apply=True, export_cameras=True, export_lights=True, filepath=filename)
