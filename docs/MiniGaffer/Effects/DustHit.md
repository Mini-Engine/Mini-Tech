---
layout: default
title: Dust Hit
parent: Effects
grand_parent: Mini Tech
nav_order: 10
---

# Mini Dust Hit

{: .note-title }
> Example Scene
>
> Help > Examples > MiniGaffer > Dust Hit Example
>

Creates a puff of dust, as a points location, whenever two objects start touching. Use it for a ball landing on the ground, a rock falling onto a cliff, or anything else that should kick up dust when it hits.

Connect a filter to choose where the dust location is created (for example `/`) and list the `colliders`. Every pair of colliders is tested. No simulation is required, so the colliders can be keyframed, driven by expressions, or come from a simulation such as MiniBullet.

| Plug | Description |
| --- | --- |
| Name | The name of the dust location, created below the filtered location. |
| Colliders | Space separated list of locations that can collide, wildcards allowed, for example `/world/rocks/*`. |
| Contact Distance | Two colliders touch when part of one is closer than this to the surface of the other. |
| Min Speed | Minimum relative speed, in units per second, for a hit to make dust. |
| Cooldown | Minimum number of frames between hits from the same pair. |
| Count Per Hit | Number of particles made by each hit. |
| Count By Speed | Extra particles for each unit per second of impact speed. |
| Seed | Changes the random variation. |
| Lifetime | Number of frames each particle lives for. |
| Speed | Average emission speed. |
| Spread | How random the emission direction is. |
| Normal Bias | How strongly particles follow the contact normal. |
| Drag | How quickly particles slow down. |
| Gravity | Acceleration applied to particles. Small for dust, positive Y for rising smoke. |
| Start Width, End Width | Particle width at birth and at the end of life. |
| Color | Stored in the `Cd` primitive variable. |
| Fade Out | How `opacity` falls from 1 to 0 over a particle's life. |

The points have `v`, `width`, `opacity`, `age`, `Cd` and `id` primitive variables.

* Hits are found by testing the vertices of each collider against the surface of the others, so meshes with large faces are handled correctly. Other primitives are tested by their vertices only.
* The dust is a pure function of the hits in the last `lifetime` frames, so it can be evaluated at any frame, in any order, and is the same every time. Evaluating a frame looks at the colliders over that range of frames, so very long lifetimes with heavy colliders are slower.
* A hit is made when contact begins. Objects resting together do not keep making dust.

## Dust Hit Volume

`DustHitVolume`, in the **Mini > VDB** menu, turns the dust into a VDB fog volume. Connect the output of Dust Hit to its **points** input and set **Points Location** to the dust location (default `/dust`). It creates a `/dustVolume` location with a `density` grid. **Voxel Size** and **Radius Scale** control the resolution and softness of the puffs.
