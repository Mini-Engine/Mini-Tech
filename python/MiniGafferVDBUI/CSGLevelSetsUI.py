import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.CSGLevelSets,
    'description',
    """Perform CSG on levelset grids""",
    plugs={
        'vdbLocation' : [
            'description',
            """
            path of vdb object B
            """
        ],
        'grid' : [
            'description',
            """
            name of levelset grid
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
            "preset:A union B", 0,
            "preset:A intersection B", 1,
            "preset:A difference B", 2,
            "preset:B difference A", 3
        ]
    }
)
