import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.LevelSetToFog,
    'description',
    """Converts a level set VDB grids into fog volumes""",

    plugs={
        'grids' : [
            'description',
            """
            Name of the level set grids to create fog grids from.
            """
        ]
    }
)
