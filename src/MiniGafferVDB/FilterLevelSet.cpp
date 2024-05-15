#include "MiniGafferVDB/FilterLevelSet.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/LevelSetFilter.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(FilterLevelSet );

size_t FilterLevelSet::g_firstPlugIndex = 0;

// todo rename to FilterLevelSet
FilterLevelSet::FilterLevelSet(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new StringPlug( "grids", Plug::In, "surface" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
	addChild( new IntPlug( "filterType", Plug::In, 0 ) );
}

FilterLevelSet::~FilterLevelSet()
{
}

Gaffer::StringPlug *FilterLevelSet::gridsPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *FilterLevelSet::gridsPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *FilterLevelSet::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *FilterLevelSet::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::IntPlug *FilterLevelSet::filterTypePlug()
{
	return  getChild<IntPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::IntPlug *FilterLevelSet::filterTypePlug() const
{
	return  getChild<IntPlug>( g_firstPlugIndex + 2 );
}

void FilterLevelSet::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if( input == filterTypePlug() || input == gridsPlug() || input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool FilterLevelSet::processesObject() const
{
	return true;
}

void FilterLevelSet::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	filterTypePlug()->hash( h );
	h.append( gridsPlug()->hash() );
	h.append( outputGridPlug()->hash() );
}

IECore::ConstObjectPtr FilterLevelSet::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const IECoreVDB::VDBObject *vdbObject = runTimeCast<const IECoreVDB::VDBObject>(inputObject.get());
	if( !vdbObject )
	{
		return inputObject;
	}

	std::vector<std::string> grids = vdbObject->gridNames();

	std::string gridsToProcess = gridsPlug()->getValue();

	int filterType = filterTypePlug()->getValue();
	int width = 1;

	IECoreVDB::VDBObjectPtr newVDBObject = vdbObject->copy();

	for (const auto &gridName : grids )
	{
		if ( !IECore::StringAlgo::matchMultiple( gridName, gridsToProcess ) )
		{
		    continue;
        }

        openvdb::GridBase::ConstPtr grid = vdbObject->findGrid( gridName );
        openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( grid );

        if ( !floatGrid )
        {
            continue;
        }

        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );


        openvdb::FloatGrid::Ptr filteredGrid = floatGrid->deepCopy();
        filteredGrid->setName( outGridName );
        openvdb::tools::LevelSetFilter<openvdb::FloatGrid> filter ( *filteredGrid );

        switch( filterType )
        {
            case 0:
                // todo width parameter
                // todo mask
                filter.mean( width );
                break;
            case 1:
                // todo width parameter
                // todo mask
                filter.gaussian( width );
                break;
            case 2:
                // todo width parameter
                // todo mask
                filter.median( width );
                break;
            case 3:
                // todo mask
                filter.laplacian();
                break;
            case 4:
                // todo mask
                filter.meanCurvature();
                break;
        }

        newVDBObject->insertGrid( filteredGrid );
	}

	return newVDBObject;
}

bool FilterLevelSet::processesBound() const
{
	return true;
}

void FilterLevelSet::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	hashProcessedObject( path, context, h);
}

Imath::Box3f FilterLevelSet::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}

