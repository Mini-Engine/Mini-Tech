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
            "description",
            "Name of the constant float vector primvar (size = num points in render mesh * maxWeights) which contains the weight of the influence of the cage primitive vertex referenced the matching value in IndicesPrimVar "
        ],
        "IndicesPrimVar": [
            "description",
            "Name of a the constant int vector primvar (size = num points in render mesh * maxWeights) which contains the index to the point in the cage mesh which should contribute to the deformation"
        ],
        "BindTransform": [
            "description",
            "Name of the Vertex Matrix44 attribute which transforms from the reference space to animated space."
        ]
    }

)

