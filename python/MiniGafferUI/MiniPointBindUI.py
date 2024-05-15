import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniPointBind,

    "description",
    """
    Binds primitives' points based on a binding to cage geometry
    """,
    "icon", "mini.png",
    plugs = {
        "cageLocation" : [
            "description",
            "Path to location which contains a primitive who's points the filtered geometry is going to bind to"
        ],
        "maxDistance" : [
            "description",
            "Maximum radius around each render geometry point to search for vertices in the cage primitive"
        ],
        "maxWeights" : [
            "description",
            "Maximum number of vertices in the cage mesh which can influence a render mesh vertex"
        ],
        "WeightsPrimVar": [
            "description",
            "Name of the constant float vector primvar (size = num points in render mesh * maxWeights) which contains the weight of the influence of the cage primitive vertex referenced the matching value in IndicesPrimVar "
        ],
        "IndicesPrimVar" : [
            "description",
            "Name of a the constant int vector primvar (size = num points in render mesh * maxWeights) which constains the index to the point in the cage mesh which should contribute to the deformation"
        ]
    },

)
