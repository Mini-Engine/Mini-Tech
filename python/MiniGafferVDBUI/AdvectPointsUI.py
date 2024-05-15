import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.AdvectPoints,
    'description',
    """Advect Points """,
    "icon", "mini.png",
    "documentation:url", "",

    plugs={
        "velocityLocation" : [
            "description", "location in the scene of the vdb which contains the velocity grid to advect the points by"],
        "velocityGrid" : [
            "description", "Name of the velocity grid in VDB at `velocityLocation`"
        ],
        "startFrame": [
            "description", "Initial frame number to read the points from to advect in the vdb velocity field."
        ],
        "subSteps" : [
            "description", "Number of substeps per frame to perform the advection integration"
        ],
    }
)

