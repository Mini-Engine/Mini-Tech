import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.IntersectLevelSet,
    'description',
    """Intersect a Level Set grid in a VDB Object.""",
    plugs={
        'grids' : [
            'description',
            """
            Name of level set grid to intersect against
            """
        ]
    }
)
