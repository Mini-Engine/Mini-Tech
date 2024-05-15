#pragma once

#include "IECore/Export.h"

#ifdef MiniGafferVDB_EXPORTS
	#define MINIGAFFERVDB_API IECORE_EXPORT
#else
	#define MINIGAFFERVDB_API IECORE_IMPORT
#endif
