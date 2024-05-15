#include "MiniGafferVDB/AdvectPoints.h"

#include "MiniGafferVDB/AdvectGrids.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"
#include "IECore/LRUCache.h"
#include "IECore/LRUCache.inl"
#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/Interpolation.h"
#include "openvdb/tools/PointAdvect.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

namespace
{
    // todo ask if there is a common function or class for this?
    GafferScene::ScenePlug::ScenePath makeScenePath (const std::string& p)
    {
        GafferScene::ScenePlug::ScenePath output;
        IECore::StringAlgo::tokenize<IECore::InternedString>( p, '/', back_inserter( output ) );
        return output;
    };
}

IE_CORE_DEFINERUNTIMETYPED( AdvectPoints );

size_t AdvectPoints::g_firstPlugIndex = 0;

AdvectPoints::AdvectPoints( const std::string &name )
        : SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild ( new StringPlug( "velocityLocation", Plug::In, "" ) );
    addChild ( new StringPlug( "velocityGrid", Plug::In, "vel" ) );

    addChild ( new FloatPlug( "startFrame", Plug::In, 0.0f ) );
    addChild ( new IntPlug( "subSteps", Plug::In, 8 ) );
}

AdvectPoints::~AdvectPoints()
{
}

Gaffer::StringPlug *AdvectPoints::velocityLocationPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 0);
}

const Gaffer::StringPlug *AdvectPoints::velocityLocationPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex + 0);
}

Gaffer::StringPlug *AdvectPoints::velocityGridPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *AdvectPoints::velocityGridPlug() const
{
    return getChild<const StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::FloatPlug *AdvectPoints::startFramePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::FloatPlug *AdvectPoints::startFramePlug() const
{
    return getChild<const FloatPlug>( g_firstPlugIndex + 2 );
}

Gaffer::IntPlug *AdvectPoints::subStepsPlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::IntPlug *AdvectPoints::subStepsPlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

void AdvectPoints::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
    SceneElementProcessor::affects( input, outputs );

    if ( input == velocityLocationPlug()
        || input == velocityGridPlug()
        || input == startFramePlug()
        || input == subStepsPlug()
        )
    {
        outputs.push_back( outPlug()->objectPlug() );
        outputs.push_back( outPlug()->boundPlug() );
    }
}

bool AdvectPoints::processesObject() const
{
    return true;
}

void AdvectPoints::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedObject( path, context, h );

    ScenePlug::ScenePath velocityLocationPath;
    ScenePlug::stringToPath( velocityLocationPlug()->getValue(), velocityLocationPath);

    h.append( subStepsPlug()->hash() );
    h.append ( inPlug()->objectHash( velocityLocationPath ) );
    h.append ( inPlug()->transformHash( velocityLocationPath ) );
    h.append( startFramePlug()->hash() );
    h.append( subStepsPlug()->hash() );
}

struct Simulation;

// get the current time
struct SimulationStep
{
    int subStep;

    SimulationStep( int subStep)
     : subStep(subStep)
    {}

    bool Step(Simulation& simulation);

};

struct Simulation
{
    float startFrame;
    float stepDuration;
    using GetVelocityGridFn = std::function<openvdb::Vec3fGrid::ConstPtr(float)>;
    GetVelocityGridFn getVelocityGridFn;

    using GetPointsFn = std::function<IECoreScene::ConstPointsPrimitivePtr(float)>;
    GetPointsFn getPointsFn;

    explicit Simulation( float startFrame, float stepDuration, GetVelocityGridFn getVelocityGridFn, GetPointsFn getPointsFn)
       : startFrame(startFrame), stepDuration(stepDuration), getVelocityGridFn(getVelocityGridFn), getPointsFn(getPointsFn)
    {
    }


    IECoreScene::ConstPointsPrimitivePtr Execute(int subStep)
    {
        std::map<int, IECoreScene::ConstPointsPrimitivePtr> cache;
        IECoreScene::ConstPointsPrimitivePtr points = getPointsFn(startFrame);

        for (int i = 0; i < subStep; ++i)
        {
            float frame =  startFrame + i * stepDuration;
            points = Step(points, stepDuration, frame);
        }

        return points;
    }

    IECoreScene::ConstPointsPrimitivePtr  Step(IECoreScene::ConstPointsPrimitivePtr points, float deltaT, float frame);

};

IECoreScene::ConstPointsPrimitivePtr  Simulation::Step(IECoreScene::ConstPointsPrimitivePtr points, float deltaT, float frame)
{
    // advect form previous positions
    openvdb::tools::PointAdvect<> pointAdvector(*getVelocityGridFn(frame)); // todo add interuptor

    const auto it = points->variables.find("P");
    const auto PositionData = runTimeCast<IECore::V3fVectorData>(it->second.expandedData());
    const auto& readablePoints = PositionData->readable();

    std::vector<openvdb::math::Vec3<float>> vdbPoints;

    // todo remove this copy
    for (const auto p : readablePoints)
    {
        vdbPoints.emplace_back(p.x, p.y, p.z);
    }

    pointAdvector.setIntegrationOrder(4);
    pointAdvector.advect(vdbPoints, deltaT, 4);

    auto newPoints = points->copy();
    auto& writable = runTimeCast<IECore::V3fVectorData>(newPoints->variables["P"].data)->writable();

    // todo this copy is also crappy
    for (int i = 0; i < vdbPoints.size(); ++i)
    {
        writable[i] = Imath::V3f(vdbPoints[i].x(), vdbPoints[i].y(), vdbPoints[i].z());
    }

    return newPoints;

}

IECore::ConstObjectPtr AdvectPoints::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    const float frame = context->get<float>("frame");
    const int subSteps = subStepsPlug()->getValue();
    const float startFrame = startFramePlug()->getValue();

    const auto velocityVDBPath = makeScenePath(velocityLocationPlug()->getValue());

    IECoreScene::ConstPointsPrimitivePtr inputPointsStartFrame = nullptr;
    {
        Gaffer::Context::EditableScope StartFrameScope(context);
        StartFrameScope.set("frame", &startFrame);
        inputPointsStartFrame = runTimeCast<const IECoreScene::PointsPrimitive>(inPlug()->object(path));
    }

    if (!inputPointsStartFrame)
    {
        return inputObject;
    }

    const float framesPerSecond = context->getFramesPerSecond();

    const GafferScene::ScenePlug* scenePlug = inPlug();
    const auto gridName = velocityGridPlug()->getValue();
    auto velocityGridFn = [scenePlug, &velocityVDBPath, &gridName, &context](float frame) -> openvdb::Vec3fGrid::ConstPtr
    {
        Gaffer::Context::EditableScope StartFrameScope(context);
        frame = floor(frame);
        StartFrameScope.set("frame", &frame);

        const auto object = scenePlug->object(velocityVDBPath);
        const auto vdb =  runTimeCast<const VDBObject>(object);
        if (!vdb)
        {
            return nullptr;
        }

        const auto velocityGrid = vdb->findGrid(gridName);

        if (!velocityGrid)
        {
            return nullptr;
        }

        openvdb::Vec3fGrid::ConstPtr v3fVelocityGrid = openvdb::GridBase::constGrid<openvdb::Vec3fGrid>( velocityGrid );

        return v3fVelocityGrid;
    };

    const GafferScene::ScenePlug* pointsScenePlug =  inPlug();

    auto getPointsFn = [context, pointsScenePlug, & path](const float frame) -> IECoreScene::ConstPointsPrimitivePtr
    {
        Gaffer::Context::EditableScope StartFrameScope(context);
        StartFrameScope.set("frame", &frame);

        const auto inputPointsStartFrame = runTimeCast<const IECoreScene::PointsPrimitive>(pointsScenePlug->object(path));

        return inputPointsStartFrame;
    };

    int startSubStep = (frame - startFrame) * subSteps;

    Simulation sim(startFrame, 1.0f / (subSteps * framesPerSecond), velocityGridFn, getPointsFn);
    auto result = sim.Execute(startSubStep);

    if (!result)
    {
        return inputObject;
    }

    return result;
}

bool AdvectPoints::processesBound() const
{
    return true;
}

void AdvectPoints::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    SceneElementProcessor::hashProcessedBound( path, context, h );
}

Imath::Box3f AdvectPoints::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
    return inputBound;
}