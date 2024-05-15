import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniResampleCurves,

    "description",
    """
    Resample Curves
    """,

    "icon", "mini.png",
    "documentation:url", "",

    "layout:activator:isFactionMode", lambda parent : parent["mode"].getValue() == 1,
    "layout:activator:isConstantMode", lambda parent : parent["mode"].getValue() == 0,

    plugs = {
        "mode" : [

            "description",
            """
            Method used to caculate the number of vertices per curve

            - constant number of CVs
            - faction of the original curve's CVs

            """,

            "preset:Constant", 0,
            "preset:Fraction", 1,

            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
        ],

        "numControlPoints" : [
            "description",
            """
            Fixed number of CVs per curve

            """,
            "layout:activator", "isConstantMode",
        ],
        "fraction" : [
            "description",
            """
            Fraction of the number of CVs in the new curves

            """,
            "layout:activator", "isFactionMode",
        ]
    },
)