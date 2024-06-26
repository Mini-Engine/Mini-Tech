---
layout: default
title: Rigid Bind Curves
parent: Curves
grand_parent: Mini Tech
nav_order: 70
---

# Mini Rigid Bind Curves

{: .note-title }
> Rigid Bind and Deform Example Scene
>
> Help > Examples > MiniGaffer > Rigid Bind Example
>


Bind Curves in the T pose / pref pose to an animated character

* Skin mesh in bind pose
* curves in bind pose
* curve is treated as non deforming
* root point of the curve is welded / constrained to a point on the skin
* primvars are created on the curves so that if an animated mesh is supplied, the rigid curves can be transformed

* Two parts
  * bind
  * deform

* Issues
  * long curves don't deform naturally
  * no motion in the curves 
