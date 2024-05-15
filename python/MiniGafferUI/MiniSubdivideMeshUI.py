import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniSubdivideMesh,

    "description",
    """
    Subdivide Mesh
    """,

    "icon", "mini.png",
    "documentation:url", "",

    plugs = {
        "level" : [

            "description",
            """
            Catmull-Clark subdivision Level
            """,

        ],
    },
)