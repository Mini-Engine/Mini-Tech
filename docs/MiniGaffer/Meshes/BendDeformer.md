---
layout: default
title: Bend Deformer
parent: Meshes
grand_parent: Mini Tech
nav_order: 65
---

# Mini Bend Deformer

{: .note-title }
> Example Scene
>
> Help > Examples > MiniGaffer > Bend Deformer Example
>

Bends the filtered geometry like Maya's Bend nonlinear deformer.

The geometry is bent along the **handle's Y axis**, curving towards the handle's **X axis**. Only the part of the geometry between the low and high bound is curved. Anything beyond a bound carries on in a straight line from the end of the curve, so a long object bends in the middle while its ends stay rigid.

| Plug | Description |
| --- | --- |
| Envelope | Blends between the original shape (`0`) and the fully bent shape (`1`). |
| Curvature | How sharply to bend, in radians per unit of length. Positive bends towards the handle's +X, negative towards -X. `0` does nothing. |
| Low Bound | Where the bend starts along the handle's Y axis. `0` or less. |
| High Bound | Where the bend ends along the handle's Y axis. `0` or more. |
| Handle | Translate, rotate and scale to choose where the bend is centred and which way it bends. |

* Normals, if present, are rotated with the bend.
* The handle is in the local space of each filtered location.
* To bend a different direction, rotate the handle. For example, rotate Y by 90 to bend towards Z.
