import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniBendDeformer,

    "description",
    """
    Bends the filtered geometry like Maya's Bend nonlinear deformer.

    The geometry is bent along the handle's Y axis, towards its X axis. Move,
    rotate and scale the handle to choose where and in which direction the bend
    happens.
    """,
    "icon", "mini.png",
    plugs = {

        "envelope" : [
            "description",
            """
            Blends between the original shape ( 0 ) and the fully bent shape ( 1 ).
            """,
        ],

        "curvature" : [
            "description",
            """
            How sharply to bend, in radians per unit of length along the handle's Y axis.
            Positive values bend towards the handle's +X, negative values towards -X.
            0 leaves the geometry untouched. A curvature of `1` curls 1 unit of length through
            about 57 degrees.
            """,
        ],

        "lowBound" : [
            "description",
            """
            Where the bend starts, measured along the handle's Y axis. Must be 0 or less.
            Geometry below this is not curved any further and continues straight.
            """,
        ],

        "highBound" : [
            "description",
            """
            Where the bend ends, measured along the handle's Y axis. Must be 0 or more.
            Geometry above this is not curved any further and continues straight.
            """,
        ],

        "handle" : [
            "description",
            """
            The bend handle. The bend is centred on its origin, runs along its Y axis
            and curves towards its X axis. The bounds are measured in the handle's space.
            """,
        ],

    }

)
