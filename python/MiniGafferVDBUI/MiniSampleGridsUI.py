import IECore

import Gaffer
import MiniGaffer
import MiniGafferVDB

Gaffer.Metadata.registerNode(

    MiniGafferVDB.SampleGrids,

    "description",
    """
    Sample VDB Grids
    """,

    "icon", "mini.png",
    "documentation:url", "",

    plugs = {},
)