import bpy
from mathutils import Vector

bpy.ops.object.mode_set(mode='POSE')

animations = {
  "test_animation1": {
    0: {
      'Bone':  [0, 0, 0],
      'Bone.006': [0, 0, 0],
      'Bone.007': [0, 0, 0],
    },
    3: {
      'Bone': [0, 0, 0],
      'Bone.006': [0.2, 0, 0],
      'Bone.007': [0.2, 0, 0],
    },
    5: {
      'Bone': [0, 0, 0],
      'Bone.006': [-0.2, 0, 0],
      'Bone.007': [-0.2, 0, 0],
    }
  },
}

def get_position(bonename, keyframe_index, animation_name):
  current_frame = animations[animation_name][keyframe_index]
  if not bonename in current_frame:
      return None
  return current_frame[bonename]

def set_keyframe(bone, position, keyframe_index):
  print ('setting -> ' + bone.name + ' to ' + str(position))
  bone.location = Vector(position)
  bone.keyframe_insert(data_path='location', frame=keyframe_index)

def add_mocap_frames(animation_name):
  bones = bpy.context.scene.objects['Armature'].pose.bones
  for keyframe_index in animations[animation_name]:
    for bone in bones:
      position = get_position(bone.name, keyframe_index, animation_name)
      if position != None:
        set_keyframe(bone, position, keyframe_index * 10)  

def create_animation(animation_name):
  bpy.data.actions.new(animation_name)

def animation_exists(animation_name):
  return animation_name in bpy.data.actions.keys()

def delete_animation(animation_name):
  bpy.data.actions.remove(bpy.data.actions[animation_name])

def delete_all_animations():
  for animation in bpy.data.actions:
    delete_animation(animation.name)

def set_animation(animation_name):
  bpy.context.scene.objects['Armature'].animation_data.action = bpy.data.actions[animation_name]

delete_all_animations()
for animation_name in animations:
  if animation_exists(animation_name):     # TODO -> add option here to 
    delete_animation(animation_name)
  create_animation(animation_name)
  set_animation(animation_name)
  add_mocap_frames(animation_name)