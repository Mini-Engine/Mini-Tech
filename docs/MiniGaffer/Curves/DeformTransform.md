---
title: Deform Transform
parent: Curves
grand_parent: Mini Tech
nav_order: 4
---

# Deform Transform

Create the required PrimVars to bind Curves Primitive to a Mesh Primitive.

### Required Intput PrimVars

| Name | Type | Interpolation | Notes               |
|------|------|---------------|---------------------|
| P    | V3f  | Vertex        | *Animated* Position |
| Pref | V3f  | Vertex        | *Rest* Position     |
| uv   | V2d  | Face Varying  | texture coordinates |


## Output PrimVars

| Name          | Type | Interpolation | Notes                                           |
|---------------|------|---------------|-------------------------------------------------|
| BindTransform | M44f | Vertex        | Affine transform from *Rest* to *Animated* pose |


{: .note }
> Typically used in conjunction with [Rigid Bind](MiniCurvesRigidBind.md) & [Mini Point Deform](MiniPointDeformer.md)


{: .note-title }
> Point Deformer Example Scene
>
> Help > Examples > MiniGaffer > Point Deformer Example
> 
