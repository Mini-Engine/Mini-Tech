
#include "MiniGafferVDB/TransformGrids.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/CompoundNumericPlug.h"
#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/GridTransformer.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED( TransformGrids );

size_t TransformGrids::g_firstPlugIndex = 0;

TransformGrids::TransformGrids( const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new StringPlug( "grids", Plug::In, "*" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
	addChild( new TransformPlug( "transform", Plug::In ) );
}

TransformGrids::~TransformGrids()
{
}

Gaffer::StringPlug *TransformGrids::gridsPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *TransformGrids::gridsPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex  );
}

Gaffer::StringPlug *TransformGrids::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *TransformGrids::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::TransformPlug *TransformGrids::transformPlug()
{
	return  getChild<TransformPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::TransformPlug *TransformGrids::transformPlug() const
{
	return  getChild<TransformPlug>( g_firstPlugIndex + 2 );
}

void TransformGrids::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if( input == gridsPlug() ||  input == transformPlug() || input->parent() == transformPlug() || input->parent()->parent() == transformPlug() || input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool TransformGrids::processesObject() const
{
	return true;
}

void TransformGrids::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	transformPlug()->hash( h );
	gridsPlug()->hash( h );
	outputGridPlug()->hash( h );
}

IECore::ConstObjectPtr TransformGrids::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const VDBObject *vdbObject = runTimeCast<const VDBObject>(inputObject.get());

	if( !vdbObject )
	{
		return inputObject;
	}

	std::string grids = gridsPlug()->getValue();

	VDBObjectPtr newVDBObject = runTimeCast<VDBObject>(vdbObject->copy());
	std::vector<std::string> gridNames = newVDBObject->gridNames();

	auto convert = [](Imath::V3f v) -> openvdb::Vec3R
	{
		return openvdb::Vec3R(v[0], v[1],v[2]);
	};

	openvdb::tools::GridTransformer transfomer (
			convert( transformPlug()->pivotPlug()->getValue() ),
			convert( transformPlug()->scalePlug()->getValue() ),
			convert( transformPlug()->rotatePlug()->getValue() ),
			convert( transformPlug()->translatePlug()->getValue() )
			);

	for ( const auto &gridName : gridNames )
	{
		if ( !IECore::StringAlgo::matchMultiple(gridName, grids) )
		{
		    continue;
        }

        openvdb::GridBase::ConstPtr srcGrid = newVDBObject->findGrid( gridName );

        auto srcFloatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid );

        if ( !srcFloatGrid )
        {
            continue;
        }

        openvdb::FloatGrid::Ptr newGrid ( new openvdb::FloatGrid() );
        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

        newGrid->setName( outGridName );

        auto outFloatGrid = srcFloatGrid->deepCopy();

        transfomer.transformGrid<openvdb::tools::PointSampler>( *srcFloatGrid, *newGrid);

        newVDBObject->insertGrid( newGrid );
	}

	return newVDBObject;

}

bool TransformGrids::processesBound() const
{
	return true;
}

void TransformGrids::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	gridsPlug()->hash( h );
}

Imath::Box3f TransformGrids::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}


