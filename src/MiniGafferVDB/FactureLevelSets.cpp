#include "MiniGafferVDB/FractureLevelSets.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/GridOperators.h"
#include "openvdb/tools/LevelSetFracture.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(FractureLevelSets );

size_t FractureLevelSets::g_firstPlugIndex = 0;

FractureLevelSets::FractureLevelSets(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild ( new StringPlug( "grids", Plug::In, "*" ) );

	outPlug()->boundPlug()->setInput( inPlug()->boundPlug() );
	outPlug()->objectPlug()->setInput( inPlug()->objectPlug() );
}

FractureLevelSets::~FractureLevelSets()
{
}

Gaffer::StringPlug *FractureLevelSets::gridsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *FractureLevelSets::gridsPlug() const
{
	return getChild<const StringPlug>( g_firstPlugIndex );
}

void FractureLevelSets::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if ( input == gridsPlug()  )
	{
		outputs.push_back( outPlug()->attributesPlug() );
	}
}

bool FractureLevelSets::processesObject() const
{
	return true;
}

void FractureLevelSets::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedAttributes( path, context, h );

	h.append( inPlug()->objectHash( path ) );
	h.append( gridsPlug()->getValue() );
}

IECore::ConstObjectPtr FractureLevelSets::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	return inputObject;
}


