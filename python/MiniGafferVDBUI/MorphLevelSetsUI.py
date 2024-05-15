
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.MorphLevelSets,
    'description',
    """Morph from one level set to another""",
    plugs={
        'grid' : [
            'description',
            """
            Name of the level set grids to morph.
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
