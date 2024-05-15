#include "MiniGafferVDB/FilterGrids.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/Filter.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED( FilterGrids );

size_t FilterGrids::g_firstPlugIndex = 0;

FilterGrids::FilterGrids( const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new StringPlug( "grids", Plug::In, "density" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
	addChild( new IntPlug( "filterType", Plug::In, 0 ) );
	addChild( new IntPlug( "width", Plug::In, 1 ) );
	addChild( new IntPlug( "iterations", Plug::In, 1 ) );
}

FilterGrids::~FilterGrids()
{
}

Gaffer::StringPlug *FilterGrids::gridsPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *FilterGrids::gridsPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *FilterGrids::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *FilterGrids::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::IntPlug *FilterGrids::filterTypePlug()
{
	return  getChild<IntPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::IntPlug *FilterGrids::filterTypePlug() const
{
	return  getChild<IntPlug>( g_firstPlugIndex + 2 );
}

Gaffer::IntPlug *FilterGrids::widthPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::IntPlug *FilterGrids::widthPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

Gaffer::IntPlug *FilterGrids::iterationsPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::IntPlug *FilterGrids::iterationsPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 4 );
}

void FilterGrids::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if( input == filterTypePlug() ||
	input == gridsPlug() ||
	input == iterationsPlug() ||
	input == widthPlug() ||
	input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool FilterGrids::processesObject() const
{
	return true;
}

void FilterGrids::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	filterTypePlug()->hash( h );
	h.append( gridsPlug()->hash() );
	h.append( outputGridPlug()->hash() );
    h.append( iterationsPlug()->hash() );
    h.append( widthPlug()->hash() );
}

IECore::ConstObjectPtr FilterGrids::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const IECoreVDB::VDBObject *vdbObject = runTimeCast<const IECoreVDB::VDBObject>(inputObject.get());
	if( !vdbObject )
	{
		return inputObject;
	}

	std::vector<std::string> grids = vdbObject->gridNames();

	std::string gridsToProcess = gridsPlug()->getValue();

	int filterType = filterTypePlug()->getValue();
	int width = widthPlug()->getValue();
	int iterations = iterationsPlug()->getValue();

	IECoreVDB::VDBObjectPtr newVDBObject = vdbObject->copy();

    GafferVDB::Interrupter interrupter( context->canceller() );

	for (const auto &gridName : grids )
	{
        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

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

        openvdb::FloatGrid::Ptr filteredGrid = floatGrid->deepCopy();
        filteredGrid->setName( outGridName );
        openvdb::tools::Filter<openvdb::FloatGrid, openvdb::FloatGrid::ValueConverter<float>::Type, GafferVDB::Interrupter > filter ( *filteredGrid, &interrupter );

        switch( filterType )
        {
            case 0:
                filter.mean( width, iterations );
                break;
            case 1:
                filter.gaussian( width, iterations );
                break;
            case 2:
                filter.median( width, iterations );
                break;
        }

        if ( interrupter.wasInterrupted() )
        {
            throw IECore::Cancelled();
        }

        newVDBObject->insertGrid( filteredGrid );

	}

	return newVDBObject;
}

bool FilterGrids::processesBound() const
{
	return true;
}

void FilterGrids::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	hashProcessedObject( path, context, h);
}

Imath::Box3f FilterGrids::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}
