#include "MiniGafferVDB/MathOpGrids.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/GridOperators.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(MathOpGrids );

size_t MathOpGrids::g_firstPlugIndex = 0;

MathOpGrids::MathOpGrids(const std::string &name )
        : SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild ( new StringPlug( "grids", Plug::In, "*" ) );
    addChild ( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
    addChild ( new IntPlug( "type", Plug::In, 0 ) );
}

MathOpGrids::~MathOpGrids()
{
}

Gaffer::StringPlug *MathOpGrids::gridsPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *MathOpGrids::gridsPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *MathOpGrids::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *MathOpGrids::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::IntPlug *MathOpGrids::typePlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 2);
}

const Gaffer::IntPlug *MathOpGrids::typePlug() const
{
    return getChild<const IntPlug>( g_firstPlugIndex + 2);
}

void MathOpGrids::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
    SceneElementProcessor::affects( input, outputs );

    if ( input == gridsPlug() || input == typePlug() || input == outputGridPlug() )
    {
        outputs.push_back( outPlug()->objectPlug() );
        outputs.push_back( outPlug()->boundPlug() );
    }
}

bool MathOpGrids::processesObject() const
{
    return true;
}

void MathOpGrids::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    h.append( gridsPlug()->hash() );
    h.append( outputGridPlug()->hash() );
    h.append( typePlug()->hash() );

}

IECore::ConstObjectPtr MathOpGrids::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    auto vdbObject = runTimeCast<const VDBObject>(inputObject.get());
    if( !vdbObject )
    {
        return inputObject;
    }

    std::string grids = gridsPlug()->getValue();

    VDBObjectPtr newVDBObject = runTimeCast<VDBObject>(vdbObject->copy());
    std::vector<std::string> gridNames = newVDBObject->gridNames();

    for ( const auto &gridName : gridNames )
    {
        if ( !IECore::StringAlgo::matchMultiple( gridName, grids ) )
        {
            continue;
        }

        openvdb::GridBase::ConstPtr srcGrid = newVDBObject->findGrid( gridName );

        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"),  &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

        // todo support multiple grid types
        const int type = typePlug()->getValue();
        if ( type == 0 )
        {
            if ( openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid ) )
            {
                typename openvdb::tools::ScalarToVectorConverter<openvdb::FloatGrid>::Type::Ptr gradientGrid = openvdb::tools::gradient( *floatGrid );
                gradientGrid->setName( outGridName );
                newVDBObject->insertGrid( gradientGrid );
            }
        }
        else if ( type == 1 )
        {
            if ( openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid ) )
            {
                openvdb::FloatGrid::Ptr laplacianGrid = openvdb::tools::laplacian( *floatGrid );
                laplacianGrid->setName( outGridName );
                newVDBObject->insertGrid( laplacianGrid );
            }
        }
        else if ( type == 2 )
        {
            if ( openvdb::Vec3fGrid::ConstPtr v3grid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( srcGrid ) )
            {
                openvdb::FloatGrid::Ptr divGrid = openvdb::tools::divergence( *v3grid );
                divGrid->setName( outGridName );
                newVDBObject->insertGrid( divGrid );
            }
        }
        else if ( type == 3 )
        {
            if ( openvdb::Vec3fGrid::ConstPtr v3grid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( srcGrid ) )
            {
                openvdb::Vec3fGrid::Ptr curlGrid = openvdb::tools::curl( *v3grid );
                curlGrid->setName( outGridName );
                newVDBObject->insertGrid( curlGrid );
            }

        }
        else if ( type == 4 )
        {
            if ( openvdb::Vec3fGrid::ConstPtr v3grid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( srcGrid ) )
            {
                openvdb::FloatGrid::Ptr magnitudeGrid = openvdb::tools::magnitude( *v3grid );
                magnitudeGrid->setName( outGridName );
                newVDBObject->insertGrid( magnitudeGrid );
            }
        }
        else if ( type == 5 )
        {
            if ( openvdb::Vec3fGrid::ConstPtr v3grid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( srcGrid ) )
            {
                openvdb::Vec3fGrid::Ptr normalizedGrid = openvdb::tools::normalize( *v3grid );
                normalizedGrid->setName( outGridName );
                newVDBObject->insertGrid( normalizedGrid );
            }
        }
        else if ( type == 6 )
        {
            if ( openvdb::FloatGrid::ConstPtr srcGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid ) )
            {
                openvdb::FloatGrid::Ptr meanCurvature = openvdb::tools::meanCurvature( *srcGrid );
                meanCurvature->setName( outGridName );
                newVDBObject->insertGrid( meanCurvature );
            }
        }
    }

    return newVDBObject;

}

bool MathOpGrids::processesBound() const
{
    return true;
}

void MathOpGrids::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedBound( path, context, h );

    gridsPlug()->hash( h );
}

Imath::Box3f MathOpGrids::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
    // todo calculate bounds from vdb grids
    return inputBound;
}

