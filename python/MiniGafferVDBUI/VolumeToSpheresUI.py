
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.VolumeToSpheres,
    'description',
    """Generate a point cloud of sphere centres which fit inside the volume""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to generate points inside.
            """
        ],
        "miinSpheres" : [
            'description',
            """
            Minimum number of spheres to create
            """
        ],
        "maxSpheres" : [
            'description',
            """
            Maximum number of spheres to create
            """
        ],
        "overlapping" : [
            'description',
            """
            Spheres are prevented from overlapping unless this is selected
            """
        ],
        "minRadius" : [
            'description',
            """
            Minimum radius possible for generated spheres.
            """
        ],
        "maxRadius" : [
            'description',
            """
            Maximum radius possible for generated spheres.
            """
        ],
        "isoValue" : [
            'description',
            """
            Level Set iso value which defines the surface to fill with Spheres.
            """
        ],
    }
)
