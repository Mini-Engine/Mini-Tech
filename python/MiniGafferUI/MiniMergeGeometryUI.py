import IECore

import Gaffer
import GafferUI
import GafferSceneUI
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniMergeGeometry,

    "description",
    """
    Merge Geometry (Meshes, Curves or Points)
    """,

    "icon", "mini.png",
    "documentation:url", "",

    plugs = {
        "filter" : [

            "description",
            """
            The filter used to control which parts of the scene are
            processed. A Filter node should be connected here.
            """,

            "layout:section", "Filter",
            "noduleLayout:section", "right",
            "layout:index", -3, # Just before the enabled plug,
            "nodule:type", "GafferUI::StandardNodule",
            "plugValueWidget:type", "GafferSceneUI.FilterPlugValueWidget",
        ],
        "pedantic" : [
            "description",
            """
            Check everyting, primvars, object types, etc
            """,
        ],
        "mergedName" : [
            "description",
            """
            name of the root merged object created
            """,
        ]
    },
)

def __nodeGadget( node ) :
    nodeGadget = GafferUI.StandardNodeGadget( node )
    GafferSceneUI.PathFilterUI.addObjectDropTarget( nodeGadget )
    return nodeGadget

GafferUI.NodeGadget.registerNodeGadget( MiniGaffer.MiniMergeGeometry, __nodeGadget )