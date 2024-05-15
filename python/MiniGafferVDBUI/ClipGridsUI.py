import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.ClipGrids,
    'description',
    """Clip grids against bounding box, frustrum or active tiles""",
    plugs={
        'clipLocation' : [
            'description',
            """
            **in1** clip geometry scene graph location 
            """
        ],
        'grids' : [
            'description',
            """
            Names of grids to clip.
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
        ],
        'invert' : [
            'description',
            """
            If enabled the the voxels inside the clip geometry is discarded and the geometry outside kept.
            """
        ],

    }
)
