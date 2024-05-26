---
title: Deform Transform
parent: Curves
grand_parent: Mini Tech
nav_order: 40
---

# Deform Transform

Create the required PrimVars to bind Curves Primitive to a Mesh Primitive.

### Required Input PrimVars

| Name | Type | Interpolation | Notes               |
|------|------|---------------|---------------------|
| P    | V3f  | Vertex        | *Animated* Position |
| Pref | V3f  | Vertex        | *Rest* Position     |
| uv   | V2d  | Face Varying  | texture coordinates |




{: .note }
> Typically used in conjunction with [Rigid Bind](MiniCurvesRigidBind.md) & [Mini Point Deform](MiniPointDeformer.md)


{: .note-title }
> Point Deformer Example Scene
>
> Help > Examples > MiniGaffer > Point Deformer Example
> 
