---
layout: default
title: Blend Deformer
parent: Meshes
grand_parent: Mini Tech
nav_order: 60
---

# Mini Blend Deformer

Blends the points of the filtered geometry towards one or more target shapes, like Maya's blendShape.

For each target, the deformer adds `weight * ( target - base )` to the points of the filtered geometry. The sum is multiplied by the envelope and the optional per point mask.

| Plug | Description |
| --- | --- |
| Envelope | Overall strength. `0` leaves the geometry untouched. |
| Base Location | Location of the geometry the targets were modelled from. Leave empty to measure target offsets from the points being deformed. |
| Mask PrimVar | Optional float vertex primitive variable which scales the deformation per point, like painted blendShape weights. |
| Targets | Add targets with the **+** button, remove them from the right click menu. Each has an **Enabled** toggle, a **Location** and a **Weight**. |

* The geometry and every target must have the same number of points in the same order, otherwise the node errors.
* Weights are not clamped, so values below 0 or above 1 extrapolate, as in Maya.
* Multiple targets are summed, so blending two targets at 0.5 each is the average of their offsets, not their mix.
* Animate the weights to drive facial shapes or corrective shapes.
