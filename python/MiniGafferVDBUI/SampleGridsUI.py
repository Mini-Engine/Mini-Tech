
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.SampleGrids,
    'description',
    """Sample VDB Grid at points defined by a Points Primitive""",

    plugs={
        'vdbLocation' : [
            'description',
            """
            Scene graph location of the VDB object to sample
            """
        ],
        'grids' : [
            'description',
            """
            Names of grid to sample.
            """
        ],
        'position' : [
            'description',
            """
            name of position attribute on points objects to use for vdb sampling
            """
        ],
        'interpolation' : [
            'description',
            """
           
            """
        ],
    }
)
