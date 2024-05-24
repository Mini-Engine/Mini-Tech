import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniGrowCurves,

    "description",
    """
    Grow curves from the positions in another primitive
    """,

    "icon", "mini.png",
    "documentation:url", "",
)