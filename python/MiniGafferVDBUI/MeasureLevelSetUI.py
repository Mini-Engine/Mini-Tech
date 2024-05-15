import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.MeasureLevelSet,
    'description',
    """Calculate LevelSet area, volume & curvature for grids""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to generate measures for.
            """
        ],
        'curvature' : [
            'description',
            """
            Calculate the mean curvature of the level set. Note this is much slower than calculating area & volume
            """
        ],
        'worldUnits' : [
            'description',
            """
            Calculate the metrics using world space units instead of index space.
            """
        ]
    }
)