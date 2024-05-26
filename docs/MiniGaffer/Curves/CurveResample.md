---
title: Resample Curves
parent: Curves
grand_parent: Mini Tech
nav_order: 20
---

# Mini Resample Curves

Change the number of control points in a curve, either by setting an absolute target value or as a fraction of the input control point count


### Plugs

| Name               | Type     | default value | Notes                                                   |
|--------------------|----------|---------------|---------------------------------------------------------|
| Mode               | int enum | faction       | Faction / Constant                                      |
| Num Control Points | int      | 5             | Number of control points in constant mode               |
| Faction            | float    | 1.0           | Multiplier for number of control points in Faction Mode |

{: .note-title }
> Example Scene
>
> Help > Examples > MiniGaffer > Curve Resample Example
> 
