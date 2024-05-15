import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.MathOpGrids,
    'description',
    """Perform Mathematical operation on Grids""",
    plugs={
        'grids' : [
            'description',
            """
            Names of the grids to be operated on
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
        ],
        'type' : [
            'description',
            """
            Type of operation
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:gadient", 0,
            "preset:lapacian", 1,
            "preset:divergence", 2,
            "preset:curl", 3,
            "preset:magnitude", 4,
            "preset:normalize", 5,
            "preset:meanCurvature", 6,
        ],

    }
)
