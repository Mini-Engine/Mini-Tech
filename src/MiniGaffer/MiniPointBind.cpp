#define NOMINMAX

#include "Mini/MiniPointBind.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/Primitive.h"
#include "IECoreScene/PrimitiveEvaluator.h"
#include "IECoreScene/MeshAlgo.h"

#include <embree4/rtcore.h>

#include <tbb/tbb.h>

#include "fmt/format.h"

#include "picoflann.h"
#include "nanoflann.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{
    GafferScene::ScenePlug::ScenePath makeScenePath (const std::string& p)
    {
        GafferScene::ScenePlug::ScenePath output;
        IECore::StringAlgo::tokenize<IECore::InternedString>( p, '/', back_inserter( output ) );
        return output;
    };
}

IE_CORE_DEFINERUNTIMETYPED( MiniPointBind );

size_t MiniPointBind::g_firstPlugIndex = 0;

MiniPointBind::MiniPointBind(const std::string &name )
    : ObjectProcessor( name )
{
    storeIndexOfNextChild(g_firstPlugIndex);

    addChild( new StringPlug( "cageLocation", Plug::In, "/cage" ) );
    addChild( new FloatPlug( "maxDistance", Plug::In, 10.0f ) );
    addChild( new IntPlug( "maxWeights", Plug::In, 4 ) );
    addChild( new StringPlug( "WeightsPrimVar", Plug::In, "PointBindWeights" ) );
    addChild( new StringPlug( "IndicesPrimVar", Plug::In, "PointBindIndices" ) );
}

Gaffer::StringPlug *MiniPointBind::CageLocationPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *MiniPointBind::CageLocationPlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::FloatPlug *MiniPointBind::MaxDistancePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::FloatPlug *MiniPointBind::MaxDistancePlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

Gaffer::IntPlug *MiniPointBind::MaxWeightsPlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::IntPlug *MiniPointBind::MaxWeightsPlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *MiniPointBind::WeightsPrimVarPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *MiniPointBind::WeightsPrimVarPlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

Gaffer::StringPlug *MiniPointBind::IndicesPrimVarPlug()
{
    return getChild<StringPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::StringPlug *MiniPointBind::IndicesPrimVarPlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex + 4 );
}


bool MiniPointBind::affectsProcessedObject(const Gaffer::Plug *input ) const
{
    if (
        input == MaxDistancePlug() ||
        input == MaxWeightsPlug() ||
        input == CageLocationPlug() ||
        input == IndicesPrimVarPlug() ||
        input == WeightsPrimVarPlug() )
    {
        return true;
    }
    return ObjectProcessor::affectsProcessedObject( input );
}

void MiniPointBind::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    MaxDistancePlug()->hash(h);
    MaxWeightsPlug()->hash(h);
    IndicesPrimVarPlug()->hash(h);
    WeightsPrimVarPlug()->hash(h);

    const ScenePath cageScenePath = makeScenePath(CageLocationPlug()->getValue());
    h.append(inPlug()->objectHash(cageScenePath));

    ObjectProcessor::hashProcessedObject( path, context, h );

}

IECore::ConstObjectPtr MiniPointBind::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
    const ScenePath cageScenePath = makeScenePath(CageLocationPlug()->getValue());

    IECore::ConstObjectPtr cageObject = inPlug()->object(cageScenePath);
    const auto cagePrimitive = IECore::runTimeCast<const IECoreScene::Primitive>(cageObject);

    const auto pPrimVarIt = cagePrimitive->variables.find("P");

    if (pPrimVarIt == cagePrimitive->variables.end())
    {
        return inputObject;
    }

    auto positionData = IECore::runTimeCast<IECore::V3fVectorData>(pPrimVarIt->second.expandedData());
    const auto& positions = positionData->readable();

    // Adapter.
    // Given an Point2f element, it returns the element of the dimension specified such that dim=0 is x and dim=1 is y
    struct PicoFlann_V3fAdaptor{
        inline  float operator( )(const Imath::V3f &elem, int dim) const { return elem[dim]; }
    };

    picoflann::KdTreeIndex<3,PicoFlann_V3fAdaptor>  kdtree;
    kdtree.build(positions);

    const auto inputPrimitive = IECore::runTimeCast<const IECoreScene::Primitive>(inputObject);
    const auto pPrimVarItInput = inputPrimitive->variables.find("P");

    auto inputPositionData = IECore::runTimeCast<IECore::V3fVectorData>(pPrimVarItInput->second.expandedData());
    const auto& inputPositions = inputPositionData->readable();

    float maxDistance = MaxDistancePlug()->getValue();
    int maxWeights = MaxWeightsPlug()->getValue();

    std::vector<std::pair<uint32_t,double>> res;

    auto outputPrimitive = inputPrimitive->copy();

    const std::string weightIndicesPrimvarName = IndicesPrimVarPlug()->getValue();
    const std::string weightValuesPrimvarName = WeightsPrimVarPlug()->getValue();

    IECore::IntVectorDataPtr outIndexData = new IECore::IntVectorData();
    auto& writableIndices = outIndexData->writable();

    IECore::FloatVectorDataPtr outWeightData = new IECore::FloatVectorData();
    auto& writableWeights = outWeightData->writable();

    outputPrimitive->variables[weightIndicesPrimvarName] = IECoreScene::PrimitiveVariable(IECoreScene::PrimitiveVariable::Constant, outIndexData);
    outputPrimitive->variables[weightValuesPrimvarName] = IECoreScene::PrimitiveVariable(IECoreScene::PrimitiveVariable::Constant, outWeightData);

    for (size_t i = 0; i < inputPositions.size(); ++i)
    {
        kdtree.radiusSearch(res, positions, inputPositions[i],maxDistance, true, maxWeights);
        std::vector<float> weights;
        float totalWeight = 0.0f;
        for (size_t k = 0; k  < maxWeights; ++k)
        {
            if (k < res.size())
            {
                writableIndices.push_back(res[k].first);
                const float weight = 1.0f / res[k].second;
                totalWeight += weight;
                weights.push_back(weight);
            }
            else
            {
                writableIndices.push_back(0);
                weights.push_back(0.0);
            }
        }
        for (size_t k = 0; k < weights.size(); ++k) {
            writableWeights.push_back(weights[k] / totalWeight);
        }
    }

    return outputPrimitive;
}

Gaffer::ValuePlug::CachePolicy MiniPointBind::MiniPointBind::processedObjectComputeCachePolicy() const
{
    return ValuePlug::CachePolicy::TaskCollaboration;
}
