#include "MiniGafferVDB/DeleteGrids.h"

#include "IECore/StringAlgo.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED( DeleteGrids );

size_t DeleteGrids::g_firstPlugIndex = 0;

DeleteGrids::DeleteGrids( const std::string &name )
        :	SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild( new StringPlug( "grids", Plug::In, "") );
}

DeleteGrids::~DeleteGrids()
{
}

Gaffer::StringPlug *DeleteGrids::gridsPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *DeleteGrids::gridsPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex );
}


void DeleteGrids::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
    SceneElementProcessor::affects( input, outputs );

    if( input == gridsPlug() )
    {
        outputs.push_back( outPlug()->objectPlug() );
        outputs.push_back( outPlug()->boundPlug() );
    }
}

bool DeleteGrids::processesObject() const
{
    return true;
}

void DeleteGrids::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    gridsPlug()->hash( h );
}

IECore::ConstObjectPtr DeleteGrids::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    const VDBObject *vdbObject = runTimeCast<const VDBObject>(inputObject.get());
    if( !vdbObject )
    {
        return inputObject;
    }

    std::string gridsToDelete = gridsPlug()->getValue();

    VDBObjectPtr newVDBObject = runTimeCast<VDBObject>(vdbObject->copy());
    std::vector<std::string> gridNames = newVDBObject->gridNames();


    for (const auto &gridName : gridNames )
    {
        if ( IECore::StringAlgo::matchMultiple( gridName, gridsToDelete ) )
        {
            newVDBObject->removeGrid( gridName );
        }
    }

    return newVDBObject;
}


bool DeleteGrids::processesBound() const
{
    return true;
}

void DeleteGrids::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedBound( path, context, h );

    gridsPlug()->hash( h );
}

Imath::Box3f DeleteGrids::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
    // todo calculate bounds from vdb grids
    return inputBound;
}
