#include "MiniGafferVDB/AdvectGrids.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/VolumeAdvect.h"
#include "openvdb/tools/LevelSetAdvect.h"
#include "openvdb/tools/Interpolation.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED( AdvectGrids );

size_t AdvectGrids::g_firstPlugIndex = 0;

AdvectGrids::AdvectGrids( const std::string &name )
        : SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild ( new ScenePlug( "velocityScene", Plug::In ) );
    addChild ( new StringPlug( "velocityLocation", Plug::In, "" ) );
    addChild ( new StringPlug( "velocityGrid", Plug::In, "vel" ) );

    addChild ( new StringPlug( "grids", Plug::In, "" ) );
    addChild ( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
    addChild ( new FloatPlug( "time", Plug::In, 0.0f ) );
}

AdvectGrids::~AdvectGrids()
{
}

GafferScene::ScenePlug *AdvectGrids::velocityScenePlug()
{
    return getChild<GafferScene::ScenePlug>( g_firstPlugIndex );
}

const GafferScene::ScenePlug *AdvectGrids::velocityScenePlug() const
{
    return getChild<const GafferScene::ScenePlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *AdvectGrids::velocityLocationPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *AdvectGrids::velocityLocationPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::StringPlug *AdvectGrids::velocityGridPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 2);
}

const Gaffer::StringPlug *AdvectGrids::velocityGridPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex + 2);
}

Gaffer::StringPlug *AdvectGrids::gridsPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 3);
}

const Gaffer::StringPlug *AdvectGrids::gridsPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex + 3);
}

Gaffer::StringPlug *AdvectGrids::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::StringPlug *AdvectGrids::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 4 );
}

Gaffer::FloatPlug *AdvectGrids::timePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 5 );
}

const Gaffer::FloatPlug *AdvectGrids::timePlug() const
{
    return getChild<const FloatPlug>( g_firstPlugIndex + 5 );
}

void AdvectGrids::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
    SceneElementProcessor::affects( input, outputs );

    if ( input->parent() == velocityScenePlug()
        || input == velocityLocationPlug()
        || input == velocityGridPlug()
        || input == gridsPlug()
        || input == outputGridPlug()
        || input == timePlug()
        )
    {
        outputs.push_back( outPlug()->objectPlug() );
        outputs.push_back( outPlug()->boundPlug() );
    }
}

bool AdvectGrids::processesObject() const
{
    return true;
}

void AdvectGrids::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    ScenePlug::ScenePath velocityLocationPath;
    ScenePlug::stringToPath( velocityLocationPlug()->getValue(), velocityLocationPath);

    h.append ( velocityScenePlug()->objectHash( velocityLocationPath ) );
    h.append ( velocityScenePlug()->transformHash( velocityLocationPath ) );

    h.append( gridsPlug()->hash() );
    h.append( outputGridPlug()->hash() );
    h.append( timePlug()->hash() );

}

IECore::ConstObjectPtr AdvectGrids::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    const VDBObject *vdbObject = runTimeCast<const VDBObject>(inputObject.get());
    if( !vdbObject )
    {
        return inputObject;
    }

    ScenePlug::ScenePath velocityLocationPath;
    ScenePlug::stringToPath( velocityLocationPlug()->getValue(), velocityLocationPath);

    IECore::ConstObjectPtr obj = velocityScenePlug()->object( velocityLocationPath );

    if ( !obj )
    {
        return inputObject;
    }

    IECoreVDB::ConstVDBObjectPtr velocityVDBObject = runTimeCast<const VDBObject>( obj );

    openvdb::GridBase::ConstPtr velocityGrid = velocityVDBObject->findGrid( velocityGridPlug()->getValue() );
    if ( !velocityGrid )
    {
        return inputObject;
    }

    openvdb::Vec3fGrid::ConstPtr v3fGrid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( velocityGrid );

    if ( !v3fGrid )
    {
        return inputObject;
    }

    GafferVDB::Interrupter interrupter( context->canceller() );
    openvdb::tools::VolumeAdvection<openvdb::Vec3fGrid, false, GafferVDB::Interrupter> volumeAdvection( *v3fGrid );

    std::vector<std::string> gridNames = vdbObject->gridNames();

    std::string gridsToAdvect = gridsPlug()->getValue();

    VDBObjectPtr newVDBObject = runTimeCast<VDBObject>(vdbObject->copy());



    for ( const auto &gridName : gridNames )
    {
        if ( !IECore::StringAlgo::matchMultiple( gridName, gridsToAdvect ) )
        {
            continue;
        }

        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

        openvdb::GridBase::ConstPtr srcGrid = newVDBObject->findGrid( gridName );

        // todo support multiple grid types
        openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid );

        if ( !floatGrid )
        {
            continue;
        }

        // todo there are other choices for the sampler type other than point sampler
        openvdb::FloatGrid::Ptr dstGrid = volumeAdvection.advect<openvdb::FloatGrid, openvdb::tools::PointSampler >( *floatGrid, (double) timePlug()->getValue() );

        if ( interrupter.wasInterrupted() )
        {
            throw IECore::Cancelled();
        }

        dstGrid->setName( outGridName );
        newVDBObject->insertGrid( dstGrid );
    }

    return newVDBObject;
}

bool AdvectGrids::processesBound() const
{
    return true;
}

void AdvectGrids::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedBound( path, context, h );

    //gridsPlug()->hash( h );
}

Imath::Box3f AdvectGrids::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
    // todo calculate bounds from vdb grids
    return inputBound;
}

