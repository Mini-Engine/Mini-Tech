import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniCurvesType,

    "description",
    """
    Set the Basis of the curves
    """,

    "icon", "mini.png",
    "documentation:url", "",


    plugs = {
        "basis" : [

            "description",
            """
            Cubic basis

            """,

            "preset:Linear", 0,
            "preset:Bezier", 1,
            "preset:BSpline", 2,
            "preset:CatmullRom", 3,

            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
        ],

    },
)