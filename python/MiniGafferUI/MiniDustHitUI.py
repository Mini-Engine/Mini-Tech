import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniDustHit,

    "description",
    """
    Creates a puff of dust, as a points location, whenever two of the collider
    locations start touching.

    Collisions are found by testing the vertices of each collider against the
    surface of the others, so no simulation is needed and the colliders can be
    animated or come from a simulation such as MiniBullet. The dust is a pure
    function of the hits in the last `lifetime` frames, so any frame can be
    evaluated directly.

    The points have `v`, `width`, `opacity`, `age`, `Cd` and `id` primitive
    variables. Use a DustHitVolume node to turn them into a VDB volume.

    > Note : Contacts are found against the surfaces of meshes, but only against
    > the vertices of other kinds of primitive.
    """,
    "icon", "mini.png",
    plugs = {

        "parent" : [
            "description",
            """
            The location the dust is created below, used when no filter is connected.
            """,
        ],

        "destination" : [
            "description",
            """
            Where the dust is created in the output scene. The default, `${scene:path}`, creates it
            below the filtered location.
            """,
        ],

        "name" : [
            "description",
            """
            The name of the location the dust is created at, below the filtered location.
            """,
        ],

        "colliders" : [
            "description",
            """
            The locations that can collide with each other, as a space separated list
            of paths. Wildcards are supported, for instance `/cubes/*`. Every pair of
            colliders is tested.
            """,
        ],

        "contactDistance" : [
            "description",
            """
            Two colliders are touching when part of one is closer than this to the
            surface of the other. Increase it for fast moving objects or sparse meshes.
            """,
        ],

        "minSpeed" : [
            "description",
            """
            The minimum relative speed, in units per second, for a contact to
            make dust. Use this to ignore objects that are only resting together.
            """,
        ],

        "cooldown" : [
            "description",
            """
            The minimum number of frames between two dust hits from the same pair
            of colliders.
            """,
        ],

        "countPerHit" : [
            "description",
            """
            The number of dust particles made by each hit.
            """,
        ],

        "countBySpeed" : [
            "description",
            """
            Extra particles made for each unit per second of impact speed.
            """,
        ],

        "seed" : [
            "description",
            """
            Changes the random variation of the dust.
            """,
        ],

        "lifetime" : [
            "description",
            """
            How many frames each particle lives for.
            """,
        ],

        "speed" : [
            "description",
            """
            The average speed particles are emitted at, in units per second.
            """,
        ],

        "spread" : [
            "description",
            """
            How much the emission direction is randomised. 0 sends every particle
            along the contact normal.
            """,
        ],

        "normalBias" : [
            "description",
            """
            How strongly the emission direction follows the contact normal rather than
            being random. Raise it for a jet, lower it for a puff that spreads in all
            directions.
            """,
        ],

        "drag" : [
            "description",
            """
            How quickly the particles slow down. Higher values make the dust stop
            sooner and hang in the air.
            """,
        ],

        "gravity" : [
            "description",
            """
            Acceleration applied to the particles, in units per second squared. Dust is
            light, so this is usually small. Use a positive Y for rising smoke.
            """,
        ],

        "startWidth" : [
            "description",
            """
            The width of the particles when they are born.
            """,
        ],

        "endWidth" : [
            "description",
            """
            The width of the particles at the end of their life. Dust expands as it ages.
            """,
        ],

        "color" : [
            "description",
            """
            The colour of the particles, stored in the `Cd` primitive variable.
            """,
        ],

        "fadeOut" : [
            "description",
            """
            Controls how the `opacity` primitive variable falls from 1 to 0 over a particle's
            life. 1 is linear, higher values fade sooner, lower values stay opaque for longer.
            """,
        ],

    }

)
