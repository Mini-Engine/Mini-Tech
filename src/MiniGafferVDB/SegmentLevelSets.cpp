
#include "MiniGafferVDB/SegmentLevelSets.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/LevelSetUtil.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(SegmentLevelSets );

size_t SegmentLevelSets::g_firstPlugIndex = 0;

SegmentLevelSets::SegmentLevelSets(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild ( new StringPlug( "grids", Plug::In, "*" ) );
    addChild ( new StringPlug( "outputGrid", Plug::In, "${grid}_${segment}" ) );
}

SegmentLevelSets::~SegmentLevelSets()
{
}

Gaffer::StringPlug *SegmentLevelSets::gridsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *SegmentLevelSets::gridsPlug() const
{
	return getChild<const StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *SegmentLevelSets::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *SegmentLevelSets::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

void SegmentLevelSets::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if ( input == gridsPlug() || input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
	}
}

bool SegmentLevelSets::processesObject() const
{
    return true;
}

void SegmentLevelSets::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    h.append( inPlug()->objectHash( path ) );
    h.append( gridsPlug()->getValue() );
    h.append( outputGridPlug()->getValue() );
}

IECore::ConstObjectPtr SegmentLevelSets::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	auto vdbObject = IECore::runTimeCast<const IECoreVDB::VDBObject> ( inPlug()->object( path ) );

	if ( !vdbObject )
	{
		return inputObject;
	}

	std::vector<std::string> gridNames = vdbObject->gridNames();
	std::string grids = gridsPlug()->getValue();

    VDBObjectPtr newVDBObject = vdbObject->copy();

	for (const auto &gridName : gridNames )
	{
		if (!IECore::StringAlgo::matchMultiple(gridName, grids))
		{
            continue;
        }

        openvdb::GridBase::ConstPtr srcGrid = vdbObject->findGrid( gridName );

        openvdb::FloatGrid::ConstPtr grid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid );

        if ( !grid )
        {
            continue;
        }

        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );

        std::vector<openvdb::FloatGrid::Ptr> segments;
        openvdb::tools::segmentSDF( *grid, segments );

        int i = 0;
        for ( auto segment : segments )
        {
            scope.set( IECore::InternedString("segment"), &i );
            const std::string outGridName = context->substitute( outputGridPlug()->getValue() );
            segment->setName( outGridName );
            newVDBObject->insertGrid( segment );
            i++;
        }
	}

	return newVDBObject;
}


