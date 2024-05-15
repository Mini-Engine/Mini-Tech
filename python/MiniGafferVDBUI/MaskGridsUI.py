import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.MaskGrids,
    'description',
    """Generate boolean mask grids""",
    plugs={

        'grids' : [
            'description',
            """
            Names of grids to filter (info about matching rules? )
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
        ]

    }
)
