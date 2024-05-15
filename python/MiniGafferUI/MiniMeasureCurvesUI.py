import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniMeasureCurves,

    "description",
    """
    Measure Curve Lengths and write into a new primvar
    """,

    "icon", "mini.png",
    "documentation:url", "",

    plugs = {
        "lengthPrimvar": [
            "description",
            "Name of the uniform float primvar which contains the length of each curve"
        ]
    },
)