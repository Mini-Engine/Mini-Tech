import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniPointDeformer,

    "description",
    """
    Deforms a primitives points based on a binding to cage geometry
    """,
    "icon", "mini.png",
    plugs = {
        "cageLocation" : [
            "description",
            "Path to location which contains a primitive who's points the filtered geometry will be deformed by"
        ],
        "WeightsPrimVar": [
            "description", ""
        ],
        "IndicesPrimVar": [
            "description", ""
        ],
        "BindTransform": [
            "description", ""
        ]
    },

)

#
# addChild( new StringPlug( "cageLocation", Plug::In, "/cage" ) );
# addChild( new StringPlug( "WeightsPrimVar", Plug::In, "PointBindWeights" ) );
# addChild( new StringPlug( "IndicesPrimVar", Plug::In, "PointBindIndices" ) );
# addChild( new StringPlug( "BindTransform", Plug::In, "BindTransform" ) );