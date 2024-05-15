
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
    MiniGafferVDB.PointsToLevelSet,
    'description',
    """Rasterise points into a level set grid""",

    "layout:activator:isTrails", lambda node : node["trails"].getValue(),
    plugs={
        'pointsLocation' : [
            'description',
            """
            Scene graph path to points primitive in 2nd scene input
            """
        ],
        'grid' : [
            'description',
            """
            name of the grid in the 1st input's VDB object 
            """
        ],
        'radiusScale' : [
            'description',
            """
            Multiplier for the **width** attribute in the points primitive. If the points primitive doesn't have a **width**
            attribute then this value is used as the radius of the points.
            """
        ],
        'trails' : [
            'description',
            """
            The velocity is used to create a trail using multiple spheres unioned along the velocity vector, scaled by trailDelta
            """
        ],
        'trailDelta' : [
            'description',
            """
            Scale for each sphere to stamp out along the velocity vector.
            """,
            "layout:activator", "isTrails",
        ],
        'velocityScale' : [
            'description',
            """
            Multiplier for the **velocity** attribute in the points primitive.
            """,
            "layout:activator", "isTrails",
        ],

    }
)
