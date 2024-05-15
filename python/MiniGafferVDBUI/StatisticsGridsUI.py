
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.StatisticsGrids,
    'description',
    """Calculate statistics (min, max, mean and standard devition ) for grids""",

    "layout:activator:histogram", lambda node : node["histogram"].getValue(),

    plugs={
        'grids' : [
            'description',
            """
            Names of grids to generate statistics for.
            """
        ],
        'histogram' : [
            'description',
            """
            Calculate the histogram of the grids 
            """
        ],
        'bins' : [
            'description',
            """
            Number of bins for the histogram calculation
            """,

            "layout:activator", "histogram",
        ]
    }
)
