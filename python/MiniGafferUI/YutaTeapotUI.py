import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.YutaTeapot,

    "description",
    """
    Creates a Utah teapot mesh, tessellated from Martin Newell's original 32
    bicubic Bezier patches.

    The teapot sits on the ground with Y up and its spout pointing down +X.
    It has smooth normals and facevarying UVs taken from the patches.
    """,
    "icon", "mini.png",
    plugs = {

        "size" : [
            "description",
            """
            Uniform scale of the teapot. At 1 the teapot is 3.15 units tall.
            """,
        ],

        "divisions" : [
            "description",
            """
            How many quads to split each Bezier patch into, along each direction.
            The teapot has 32 patches, so it has about 32 x divisions x divisions faces.
            """,
        ],

        "lid" : [
            "description",
            """
            Includes the lid. Turn off to see inside the pot.
            """,
        ],

        "bottom" : [
            "description",
            """
            Includes the bottom. The original 1975 teapot had no bottom.
            """,
        ],

    }

)
