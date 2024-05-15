
__import__( "Gaffer" )
__import__( "GafferScene" )

from ._MiniGafferVDB import *

__import__( "IECore" ).loadConfig( "GAFFER_STARTUP_PATHS", subdirectory = "MiniGafferVDB" )