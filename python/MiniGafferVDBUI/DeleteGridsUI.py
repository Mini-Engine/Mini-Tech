import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.DeleteGrids,
    'description',
    """Deletes grids from a VDB Object.""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to delete (info about matching rules? )
            """
        ]
    }
)
