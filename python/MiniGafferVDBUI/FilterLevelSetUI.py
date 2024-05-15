import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.FilterLevelSet,
    'description',
    """Filter VDB LevelSets""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to filter (info about matching rules? )
            """
        ],
        'filterType' : [
            'description',
            """
            Type of filter
            """,
            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
            "preset:mean", 0,
            "preset:gaussian", 1,
            "preset:median", 2,
            "preset:laplacian", 3,
            "preset:meanCurvature", 4
        ],
    }
)
