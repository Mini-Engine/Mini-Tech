
#include "MiniGafferVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/MeshPrimitive.h"

#include "IECoreVDB/VDBObject.h"


using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( VDBObject );

size_t VDBObject::g_firstPlugIndex = 0;

VDBObject::VDBObject( const std::string &name )
		:	ObjectSource( name, "vdb" )
{
	storeIndexOfNextChild( g_firstPlugIndex );
	addChild( new V3iPlug( "dimensions", Plug::In, V3f( 1.0f ), V3f( 0.0f ) ) );
	addChild( new IntPlug( "gridType", Plug::In, 0 ) );
    addChild( new StringPlug( "gridName", Plug::In, "" ) );
    addChild( new FloatPlug( "voxelSize", Plug::In, 0.5f, 0.0f ) );
}

VDBObject::~VDBObject()
{
}

Gaffer::V3iPlug *VDBObject::dimensionsPlug()
{
	return getChild<V3iPlug>( g_firstPlugIndex );
}

const Gaffer::V3iPlug *VDBObject::dimensionsPlug() const
{
	return getChild<V3iPlug>( g_firstPlugIndex );
}

Gaffer::IntPlug *VDBObject::gridTypePlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::IntPlug *VDBObject::gridTypePlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *VDBObject::gridNamePlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *VDBObject::gridNamePlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::FloatPlug *VDBObject::voxelSizePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::FloatPlug *VDBObject::voxelSizePlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

void VDBObject::affects( const Plug *input, AffectedPlugsContainer &outputs ) const
{
	ObjectSource::affects( input, outputs );

	if( input->parent<V3iPlug>() == dimensionsPlug() || input == gridTypePlug() || input == gridNamePlug() || input == voxelSizePlug())
	{
		outputs.push_back( sourcePlug() );
	}
}

void VDBObject::hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	dimensionsPlug()->hash( h );
    gridTypePlug()->hash( h );
    gridNamePlug()->hash( h );
    voxelSizePlug()->hash( h );
}

IECore::ConstObjectPtr VDBObject::computeSource( const Context *context ) const
{
	auto vdbObject = new IECoreVDB::VDBObject();

	//V3i dimensions = dimensionsPlug()->getValue();

	const bool isFog = gridTypePlug()->getValue() == 0;
	const float background = isFog ? 0.0f : 1.0f;

	openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create( background );

	grid->setTransform( openvdb::math::Transform::createLinearTransform(/*voxel size=*/voxelSizePlug()->getValue() ) );
	grid->setName( gridNamePlug()->getValue() );

	if ( isFog )
    {
        grid->setGridClass( openvdb::GRID_FOG_VOLUME );
    }
	else
    {
        grid->setGridClass( openvdb::GRID_LEVEL_SET );
    }

	grid->addStatsMetadata();

	vdbObject->insertGrid( grid );

	return vdbObject;
}

