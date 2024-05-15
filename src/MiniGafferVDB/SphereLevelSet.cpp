
#include "MiniGafferVDB/SphereLevelSet.h"
#include "GafferVDB/Interrupter.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/MeshPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "openvdb/tools/LevelSetSphere.h"


using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreVDB;

IE_CORE_DEFINERUNTIMETYPED( SphereLevelSet );

size_t SphereLevelSet::g_firstPlugIndex = 0;

SphereLevelSet::SphereLevelSet( const std::string &name )
        :	ObjectSource( name, "vdb" )
{
    storeIndexOfNextChild( g_firstPlugIndex );
    addChild( new StringPlug( "grid", Plug::In, "surface") );
    addChild( new FloatPlug( "radius", Plug::In, 1.0f, 0.0f ) );
    addChild( new V3fPlug( "center", Plug::In, V3f( 0.0f, 0.0f, 0.0f ) ) );
    addChild( new FloatPlug( "voxelSize", Plug::In, 0.1f, 0.0f ) );
    addChild( new FloatPlug( "halfWidth", Plug::In, (float) openvdb::LEVEL_SET_HALF_WIDTH, 0.0f ) );
}

SphereLevelSet::~SphereLevelSet()
{
}

Gaffer::StringPlug *SphereLevelSet::gridPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *SphereLevelSet::gridPlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::FloatPlug *SphereLevelSet::radiusPlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::FloatPlug *SphereLevelSet::radiusPlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

Gaffer::V3fPlug *SphereLevelSet::centerPlug()
{
    return getChild<V3fPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::V3fPlug *SphereLevelSet::centerPlug() const
{
    return getChild<V3fPlug>( g_firstPlugIndex + 2 );
}

Gaffer::FloatPlug *SphereLevelSet::voxelSizePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::FloatPlug *SphereLevelSet::voxelSizePlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

Gaffer::FloatPlug *SphereLevelSet::halfWidthPlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::FloatPlug *SphereLevelSet::halfWidthPlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

void SphereLevelSet::affects( const Plug *input, AffectedPlugsContainer &outputs ) const
{
    ObjectSource::affects( input, outputs );

    if( input == gridPlug() ||
        input == radiusPlug() ||
        input->parent() == centerPlug() ||
        input == voxelSizePlug() ||
        input == halfWidthPlug()
       )
    {
        outputs.push_back( sourcePlug() );
    }
}

void SphereLevelSet::hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    gridPlug()->hash( h );
    radiusPlug()->hash( h );
    centerPlug()->hash( h );
    voxelSizePlug()->hash( h );
    halfWidthPlug()->hash( h );
}

IECore::ConstObjectPtr SphereLevelSet::computeSource( const Context *context ) const
{
    GafferVDB::Interrupter interrupter( context->canceller() );

    const auto center = centerPlug()->getValue();

    openvdb::FloatGrid::Ptr grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid, GafferVDB::Interrupter>(
            radiusPlug()->getValue(),
            openvdb::Vec3f(center.x, center.y, center.z),
            voxelSizePlug()->getValue(),
            halfWidthPlug()->getValue(),
            &interrupter);

    if ( interrupter.wasInterrupted() )
    {
        throw IECore::Cancelled();
    }

    grid->addStatsMetadata();
    grid->setName( gridPlug()->getValue() );

    VDBObjectPtr newVDBObject = new VDBObject();
    newVDBObject->insertGrid( grid );

    return newVDBObject;

}

