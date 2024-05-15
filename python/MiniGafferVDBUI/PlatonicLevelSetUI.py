
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.PlatonicLevelSet,
    'description',
    """Create a platonic solid level set""",
    plugs={
        'grid' : [
            'description',
            """
            """
        ],

        "faces" : [
            "description",
            """
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:Tetrahedron", 4,
            "preset:Cube", 6,
            "preset:Octahedron", 8,
            "preset:Dodecahedron", 12,
            "preset:Icosahedron", 20,
        ],

        "scale" : [
            "description",
            """
            """
        ],
        "center" : [
            "description",
            """
            """
        ],
        "voxelSize" : [
            "description",
            """
            """
        ],
        "halfWidth" : [
            "description",
            """
            """
        ],

    }
)
