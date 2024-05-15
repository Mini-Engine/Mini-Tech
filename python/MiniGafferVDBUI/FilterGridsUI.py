import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.FilterGrids,
    'description',
    """Filter grid values""",
    plugs={
        'grids' : [
            'description',
            """
            Names of grids to filter (info about matching rules? )
            """
        ],
        'outputGrid' : [
            'description',
            """
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
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
            "preset:median", 2
        ],

        'width' : [
            'description',
            """
            Number of voxels to filter over
            """
        ],
        'iterations' : [
            'description',
            """
            Apply the filter this many times
            """
        ]
    }
)
