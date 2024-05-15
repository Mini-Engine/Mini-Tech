
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.TransformGrids,
    'description',
    """Transform grids""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to transform
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
        ],
    }
)
