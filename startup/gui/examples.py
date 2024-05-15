import GafferUI

import MiniGaffer

import pathlib

MTECH_ROOT = pathlib.Path( __file__ ).resolve().parents[2]

GafferUI.Examples.registerExample(
    "MiniGaffer/Mesh Curvature",
    MTECH_ROOT / "examples/meshCurvatureExample.gfr",
    description = "Demonstrates how to compute curvature from a mesh.",
    notableNodes = [
        MiniGaffer.MiniMeshCurvature,
        MiniGaffer.MiniBlurPrimitiveVariables,
    ]
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Blur Primitive Variables",
    MTECH_ROOT / "examples/blurPrimitiveVariablesExample.gfr",
    description = "Demonstrates how blur primitive variables across connected mesh vertices.",
    notableNodes = [
        MiniGaffer.MiniMeshCurvature,
        MiniGaffer.MiniBlurPrimitiveVariables,
    ]
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Noise Examples",
    MTECH_ROOT / "examples/noise.gfr",
    description = "Demonstrates noise OSL shaders."
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Merge Examples",
    MTECH_ROOT / "examples/GeometryMerge.gfr",
    description = "Merge Primitives"
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Curve Measure Example",
    MTECH_ROOT / "examples/measureCurves.gfr",
    description = "Measure Curves"
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Curve Resample Example",
    MTECH_ROOT / "examples/resample.gfr",
    description = "Resample Curves"
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Curves Type Example",
    MTECH_ROOT / "examples/curvesType.gfr",
    description = "Curves Type"
)

GafferUI.Examples.registerExample(
    "MiniGaffer/Subdivide Mesh Example",
    MTECH_ROOT / "examples/subdivide.gfr",
    description = "Subdivide Mesh"
)

