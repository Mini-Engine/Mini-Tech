
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.ScatterPoints,
    'description',
    """Scatter points into active voxels of VDB grid""",

    "layout:activator:nonuniform", lambda node : node["nonuniform"].getValue(),
    "layout:activator:uniform", lambda node : not node["nonuniform"].getValue(),
    plugs={

        'outputType' : [
            'description',
            """
            Type of primitive to generate 
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:Points Primitive", 0,
            "preset:VDB Points", 1
        ],
        'grid' : [
            'description',
            """
            Name of grid in VDBObject in which points will be scattered
            """,
        ],

        'nonuniform' : [
            'description',
            """
            If nonuniform the grid value is used to weight the number of points
            """,
        ],
        'pointCount' : [
            'description',
            """
            If 'uniform' the total number of points to generate.
            """,
            "layout:activator", "uniform",
        ],
        'probability' : [
            'description',
            """
            If 'nonuniform' the global probability which is used with the voxel value to weight the number of points.
            """,
            "layout:activator", "nonuniform",
        ],

    }
)
