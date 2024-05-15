
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.SegmentLevelSets,
    'description',
    """Segment a level set grid in to multiple level set grids.""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to segment.
            """
        ]
    }
)
