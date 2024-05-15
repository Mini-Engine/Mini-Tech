#include "MiniGafferVDB/IntersectLevelSet.h"

#include "IECore/StringAlgo.h"

#include "IECoreVDB/VDBObject.h"
#include "IECoreScene/PointsPrimitive.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/tools/RayIntersector.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED(IntersectLevelSet );

size_t IntersectLevelSet::g_firstPlugIndex = 0;

IntersectLevelSet::IntersectLevelSet(const std::string &name )
        :	SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild( new ScenePlug( "in", Gaffer::Plug::In ) );
    addChild( new StringPlug( "vdbLocation", Plug::In, "/vdb" ) );
    addChild( new StringPlug( "grids", Plug::In, "") );
}

IntersectLevelSet::~IntersectLevelSet()
{
}

GafferScene::ScenePlug *IntersectLevelSet::otherPlug()
{
    return  getChild<ScenePlug>( g_firstPlugIndex );
}

const GafferScene::ScenePlug *IntersectLevelSet::otherPlug() const
{
    return  getChild<ScenePlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *IntersectLevelSet::vdbLocationPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *IntersectLevelSet::vdbLocationPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::StringPlug *IntersectLevelSet::gridsPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *IntersectLevelSet::gridsPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

void IntersectLevelSet::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
    SceneElementProcessor::affects( input, outputs );

    if( input == gridsPlug() || input == vdbLocationPlug() || input->parent() == otherPlug() )
    {
        outputs.push_back( outPlug()->objectPlug() );
        outputs.push_back( outPlug()->boundPlug() );
    }
}

bool IntersectLevelSet::processesObject() const
{
    return true;
}

void IntersectLevelSet::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    ScenePlug::ScenePath otherPath;
    ScenePlug::stringToPath( vdbLocationPlug()->getValue(), otherPath );

    gridsPlug()->hash( h );
    h.append( otherPlug()->objectHash( otherPath ) );
    h.append( otherPlug()->fullTransformHash( otherPath ) );
}

IECore::ConstObjectPtr IntersectLevelSet::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    IECoreScene::ConstPrimitivePtr primitive = runTimeCast<const IECoreScene::Primitive>( inputObject ) ;

    if ( !primitive )
    {
        return inputObject;
    }

    ScenePlug::ScenePath p ;
    ScenePlug::stringToPath( vdbLocationPlug()->getValue(), p );

    IECoreVDB::ConstVDBObjectPtr vdbObject = runTimeCast<const IECoreVDB::VDBObject>( otherPlug()->object( p ) );
    if( !vdbObject )
    {
        return inputObject;
    }

    const auto positionPrimVar = primitive->variables.find( "P" )->second;

    const auto positionData = positionPrimVar.data;
    const auto positionV3fData = IECore::runTimeCast<IECore::V3fVectorData>( positionData );
    const auto& positions = positionV3fData->readable();

    IECore::V3fVectorDataPtr intersectedPositions = new IECore::V3fVectorData();
    auto& writablePositions = intersectedPositions->writable();
    writablePositions.resize(positions.size());

    const auto normalData = primitive->variables.find( "N" )->second.data;
    const auto normalV3fData = IECore::runTimeCast<IECore::V3fVectorData>( positionV3fData );
    const auto& normals = normalV3fData->readable();

    std::string gridsToIntersect = gridsPlug()->getValue();

    IECoreScene::PrimitivePtr newPrimitive = runTimeCast<IECoreScene::Primitive>( primitive->copy() );
    std::vector<std::string> gridNames = vdbObject->gridNames();

    for (const auto &gridName : gridNames )
    {
        if ( !IECore::StringAlgo::matchMultiple( gridName, gridsToIntersect ) )
        {
            continue;
        }

        openvdb::GridBase::ConstPtr grid = vdbObject->findGrid( gridName );
        openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( grid );

        openvdb::tools::LevelSetRayIntersector<openvdb::FloatGrid> levelSetIntersector( *floatGrid );

        using RayT = openvdb::math::Ray<openvdb::Real>;

        for( size_t i = 0; i < positions.size(); ++i )
        {
            RayT ray;
            RayT::RealType t;
            ray.setEye( RayT::Vec3T( positions[i].x,  positions[i].y,  positions[i].z ) );
            ray.setDir( RayT::Vec3T( normals[i].x,  normals[i].y,  normals[i].z ) );
            RayT::Vec3Type worldIntersectionPos;
            if ( levelSetIntersector.intersectsWS( ray, worldIntersectionPos, t ) )
            {
                writablePositions[i] = V3f( worldIntersectionPos.x(), worldIntersectionPos.y(), worldIntersectionPos.z() );
            }
            else
            {
                writablePositions[i] = positions[i];
            }
        }

        newPrimitive->variables["P"] = IECoreScene::PrimitiveVariable( positionPrimVar.interpolation, intersectedPositions);

//        openvdb::FloatGrid volumeGrid;
//        openvdb::tools::VolumeRayIntersector<openvdb::FloatGrid > volumeIntersctor( volumeGrid );

    }

    return newPrimitive;
}


bool IntersectLevelSet::processesBound() const
{
    return true;
}

void IntersectLevelSet::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedBound( path, context, h );

    gridsPlug()->hash( h );
}

Imath::Box3f IntersectLevelSet::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
    // todo calculate bounds from vdb grids
    return inputBound;
}
