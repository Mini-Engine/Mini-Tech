#include "MiniGafferVDB/ClipGrids.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/Clip.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED(ClipGrids );

size_t ClipGrids::g_firstPlugIndex = 0;

ClipGrids::ClipGrids(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new ScenePlug( "in", Gaffer::Plug::In ) );

	addChild( new StringPlug( "clipLocation", Plug::In, "/vdb" ) );
	addChild( new StringPlug( "grids", Plug::In, "density" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
	addChild( new BoolPlug( "invert", Plug::In, false ) );

}

ClipGrids::~ClipGrids()
{
}

GafferScene::ScenePlug *ClipGrids::otherPlug()
{
	return  getChild<ScenePlug>( g_firstPlugIndex );
}

const GafferScene::ScenePlug *ClipGrids::otherPlug() const
{
	return  getChild<ScenePlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *ClipGrids::clipLocationPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *ClipGrids::clipLocationPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::StringPlug *ClipGrids::gridsPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *ClipGrids::gridsPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *ClipGrids::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *ClipGrids::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

Gaffer::BoolPlug *ClipGrids::invertPlug()
{
    return  getChild<BoolPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::BoolPlug *ClipGrids::invertPlug() const
{
    return  getChild<BoolPlug>( g_firstPlugIndex + 4 );
}

void ClipGrids::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if( input->parent() == otherPlug() ||
	    input == gridsPlug() ||
	    input == clipLocationPlug() ||
	    input == invertPlug() ||
	    input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool ClipGrids::processesObject() const
{
	return true;
}

void ClipGrids::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	ScenePlug::ScenePath p ;
	ScenePlug::stringToPath( clipLocationPlug()->getValue(), p );
	h.append( otherPlug()->objectHash( p ) );
	h.append( otherPlug()->fullTransformHash( p ) );
	h.append( gridsPlug()->hash() );
	h.append( clipLocationPlug()->hash() );
	h.append( invertPlug()->hash() );
	h.append( outputGridPlug()->hash() );

}

IECore::ConstObjectPtr ClipGrids::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const VDBObject *vdbObject = runTimeCast<const VDBObject>(inputObject.get());
	if( !vdbObject )
	{
		return inputObject;
	}

	std::vector<std::string> grids = vdbObject->gridNames();
	std::string gridsToProcess = gridsPlug()->getValue();

	ScenePlug::ScenePath p ;
	ScenePlug::stringToPath( clipLocationPlug()->getValue(), p );

	Imath::Box3f bound = otherPlug()->bound( p );
	Imath::M44f transform = otherPlug()->fullTransform( p );

	Imath::M44f inputTransform = inPlug()->fullTransform( path );
	inputTransform.inverse();
	bound = Imath::transform( bound, transform * inputTransform);

	IECoreVDB::VDBObjectPtr newVDBObject = vdbObject->copy();

	for ( const auto &gridName : grids )
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

        openvdb::BBoxd bbox ( openvdb::Vec3d(bound.min[0],bound.min[1],bound.min[2] ), openvdb::Vec3d(bound.max[0], bound.max[1], bound.max[2] ) );

        openvdb::FloatGrid::Ptr clippedGrid = openvdb::tools::clip( *floatGrid, bbox, !invertPlug()->getValue() );
        clippedGrid->setName( outGridName );
        newVDBObject->insertGrid( clippedGrid );
	}

	return newVDBObject;
}

bool ClipGrids::processesBound() const
{
	return true;
}

void ClipGrids::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	gridsPlug()->hash( h );
}

Imath::Box3f ClipGrids::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}


