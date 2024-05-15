import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniBullet,

    "description",
    """
    Simulate Rigid Bodies using Bullet
    """,

    "icon", "mini.png",
    "documentation:url", "",

    plugs = {
    },
)