---
title: Point Bind
parent: Curves
grand_parent: Mini Tech
nav_order: 50
---

# Mini Point Bind

{: .note-title }
> Example Scene
>
> Help > Examples > MiniGaffer > Point Deformer Example
>



This node finds for each vertex position in the filtered geometry (render high resolution geometry) the N closest points in the cage geometry within radius R. Using the distances (D) to these closest points calculate a weight value (1/D). Store the weight and index to the position in the cage geometry in two vector constant primvars respectively.

This allows you to deform the bound high resolution render geometry using an animated low resolution cage. 

{: .highlight }
> Note any points based primitive type and be bound to another type as long as the cage geometry can define an animated basis

Use cases would be:
* Binding and deforming curves to skin meshes
* Binding and deforming animated characters' skin meshes to bones

An example can be found in:

Help > Examples > MiniGaffer > Point Deformer Example

{: .note }
> Typically used in conjunction with [Deform Transform](DeformTransform.md) & [Mini Point Deform](MiniPointDeformer.md)


