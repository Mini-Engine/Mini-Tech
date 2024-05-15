import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.CompositeGrids,
    'description',
    """Perform Composite operation on arbitrary grids""",
    plugs={
        'vdbLocation' : [
            'description',
            """
            path of vdb object B
            """
        ],
        'grids' : [
            'description',
            """
            name of grids
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
        ],
        'operation' : [
            'operation',
            """
            CSG operation
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:min", 0,
            "preset:max", 1,
            "preset:sum", 2,
            "preset:mul", 3,
            "preset:div", 4
        ]
    }
)
