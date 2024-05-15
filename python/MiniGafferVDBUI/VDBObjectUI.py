
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.VDBObject,
    'description',
    """Create a new VDB Object""",
    plugs={
        'gridType' : [
            'description',
            """
            Grid type to create (fog or level set)
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:Fog Volume", 0,
            "preset:Level Set", 1,
        ],
        'gridName' : [
            'description',
            """
            name of the grid to create in the VDB Object
            """
        ],
        'voxelSize' : [
            'description',
            """
            Size of the voxels in the empty grid. Reducing the size will increase the resolution and detail which can
            be stored in the grid but increase memory. 
            """
        ]

    }
)

