
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.SphereLevelSet,
    'description',
    """Create a sphere level set""",
    plugs={
        'grid' : [
            'description',
            """
            """
        ],
        "radius" : [
            "description",
            """
            """
        ],
        "center" : [
            "description",
            """
            """
        ],
        "voxelSize" : [
            "description",
            """
            """
        ],
        "halfWidth" : [
            "description",
            """
            """
        ],

    }
)
