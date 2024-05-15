import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.AdvectGrids,
    'description',
    """Advect grids in one VDB using a velocity field in another""",
    plugs={
        'grids' : [
            'description',
            """
            Names of the grids to be advected
            """
        ]
    }
)
