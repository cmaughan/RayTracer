import numpy as np
import matplotlib.pyplot as plt
import camera
from raymath import *

w = 256
h = 256

def intersect(O, D, obj):
    if obj['type'] == 'plane':
        return intersect_plane(O, D, obj['position'], obj['normal'])
    elif obj['type'] == 'sphere':
        return intersect_sphere(O, D, obj['position'], obj['radius'])

def get_normal(obj, M):
    # Find normal.
    if obj['type'] == 'sphere':
        N = normalize(M - obj['position'])
    elif obj['type'] == 'plane':
        N = obj['normal']
    return N

def get_color(obj, M):
    color = obj['color']
    if not hasattr(color, '__len__'):
        color = color(M)
    return color

def get_specular(obj, M):
    color = obj['specular']
    if not hasattr(specular, '__len__'):
        specular = specular(M)
    return specular

def add_sphere(position, radius, color, specular, emissive, reflection):
    return dict(type='sphere', position=np.array(position), 
        radius=np.array(radius), color=np.array(color), specular=np.array(specular), emissive=np.array(emissive), reflection=reflection)

def add_plane(position, normal):
    return dict(type='plane', position=np.array(position), 
        normal=np.array(normal),
        color=lambda M: (color_plane0 
            if (int(M[0] * 2) % 2) == (int(M[2] * 2) % 2) else color_plane1),
        diffuse_c=.75, specular_c=.5, reflection=.25)

def trace_ray(rayO, rayD):
    # Find first point of intersection with the scene.
    t = np.inf
    for i, obj in enumerate(scene):
        t_obj = intersect(rayO, rayD, obj)
        if t_obj < t:
            t, obj_idx = t_obj, i

    # Return None if the ray does not intersect any object.
    if t == np.inf:
        return

    # Find the object.
    obj = scene[obj_idx]

    # Find the point of intersection on the object.
    M = rayO + rayD * t

    # Find properties of the object.
    N = get_normal(obj, M)
    color = get_color(obj, M)
    toL = normalize(L - M)
    toO = normalize(O - M)

    # Shadow: find if the point is shadowed or not.
    l = [intersect(M + N * .0001, toL, obj_sh) 
            for k, obj_sh in enumerate(scene) if k != obj_idx]

    if l and min(l) < np.inf:
        return

    # Start computing the color.
    col_ray = ambient

    # Lambert shading (diffuse).
    col_ray += obj.get('diffuse_c', diffuse_c) * max(np.dot(N, toL), 0) * color

    # Blinn-Phong shading (specular).
    col_ray += obj.get('specular_c', specular_c) * max(np.dot(N, normalize(toL + toO)), 0) ** specular_k * color_light

    return obj, M, N, col_ray

# List of objects.
color_plane0 = 1. * np.ones(3)
color_plane1 = 0. * np.ones(3)

scene = [add_sphere([0.0, 2.0, 0.0], 2.0, [0.7, 0.1, .1], [0.9, 0.1, 0.1], [0.0, 0.0, 0.0], .5),   # Red
         add_sphere([-2.5, 1.0, 2.0], 1.0, [0.7, 0.0, 0.7], [0.9, 0.9, 0.8], [0.0, 0.0, 0.0], .5), # Purple
         add_sphere([-0.0, 0.5, 3.0], .5, [0.0, 0.3, 1.0], [0.0, 0.0, 1.0], [0.0, 0.0, 0.0], 0.0), # Blue
         #add_sphere([2.8, 0.8, 2.0], .8, [1.0, 1.0, 1.0], [0.0, 0.0, 0.0], [1.0, 1.0, 0.2], 0.0), # Light ball
         #add_sphere([-10.8, 6.4, 10.0], .4, [0.0, 0.8, 0.0], [0.0, 0.0, 0.0], [1.0, 1.0, 1.0], 0.0), # Light ball
         add_plane([0., 0.0, 0.], [0., 1., 0.]),
    ]

# Light position and color, if you just want the global light and not a sphere.
# Note - you can use the 'emissive' property of the spheres to find the lights...
L = np.array([-10.8, 6.4, 10.])
color_light = np.ones(3)

# Default light and material parameters.
ambient = .05
diffuse_c = 1.
specular_c = 1.
specular_k = 50
depth_max = 3  # Maximum number of light reflections.

col = np.zeros(3)  # Current color.
O = np.array([0., 6., 8.0])  # Camera.
Q = np.array([0., -.8, -1.0])  # Camera pointing to.
img = np.zeros((h, w, 3))

# Screen coordinates: x0, y0, x1, y1.
S = (-8.0, -8.0, 8.0, 8.0)

cam = camera.Camera(O, Q, 60, w, h)

# Loop through all pixels.
for i, x in enumerate(np.linspace(S[0], S[2], w)):
    if i % 10 == 0:
        print(i / float(w) * 100, "%")

    for j, y in enumerate(np.linspace(S[1], S[3], h)):
        col[:] = 0
        depth = 0
        rayO = O
        rayD = cam.GetWorldRay(i, h - j -1) 
        reflection = 1.

        # Loop through initial and secondary rays.
        while depth < depth_max:
            traced = trace_ray(rayO, rayD)
            if not traced:
                break

            obj, M, N, col_ray = traced

            # Reflection: create a new ray.
            rayO, rayD = M + N * .0001, normalize(rayD - 2 * np.dot(rayD, N) * N)
            depth += 1
            col += reflection * col_ray
            reflection *= obj.get('reflection', 1.)

        img[h - j - 1, i, :] = np.clip(col, 0, 1)

plt.imsave('out.png', img)
plt.imshow(img)
plt.show()