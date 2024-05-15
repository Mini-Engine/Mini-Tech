#define NOMINMAX

#include "Mini/MiniPointDeformer.h"

#include "IECoreScene/Primitive.h"
#include "IECoreScene/PrimitiveEvaluator.h"
#include "IECoreScene/MeshAlgo.h"

#include <tbb/tbb.h>

#include "fmt/format.h"

#include "Gaffer/StringPlug.h"

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

IE_CORE_DEFINERUNTIMETYPED( MiniPointDeformer );

size_t MiniPointDeformer::g_firstPlugIndex = 0;

MiniPointDeformer::MiniPointDeformer(const std::string &name )
	: ObjectProcessor( name )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new StringPlug( "cageLocation", Plug::In, "/cage" ) );
	addChild( new StringPlug( "WeightsPrimVar", Plug::In, "PointBindWeights" ) );
	addChild( new StringPlug( "IndicesPrimVar", Plug::In, "PointBindIndices" ) );
	addChild( new StringPlug( "BindTransform", Plug::In, "BindTransform" ) );
}

Gaffer::StringPlug *MiniPointDeformer::CageLocationPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *MiniPointDeformer::CageLocationPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *MiniPointDeformer::WeightsPrimVarPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *MiniPointDeformer::WeightsPrimVarPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *MiniPointDeformer::IndicesPrimVarPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *MiniPointDeformer::IndicesPrimVarPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *MiniPointDeformer::BindTransformPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *MiniPointDeformer::BindTransformPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

bool MiniPointDeformer::affectsProcessedObject(const Gaffer::Plug *input ) const
{
	if (input == CageLocationPlug() ||
        input == IndicesPrimVarPlug() ||
        input == WeightsPrimVarPlug() ||
		input == BindTransformPlug() )
	{
		return true;
	}

	return ObjectProcessor::affectsProcessedObject( input );
}

void MiniPointDeformer::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    IndicesPrimVarPlug()->hash(h);
    WeightsPrimVarPlug()->hash(h);
	BindTransformPlug()->hash(h);
	const ScenePath cageScenePath = makeScenePath(CageLocationPlug()->getValue());

	h.append(inPlug()->objectHash(cageScenePath));

	ObjectProcessor::hashProcessedObject( path, context, h );
}

IECore::ConstObjectPtr MiniPointDeformer::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
	const ScenePath cageScenePath = makeScenePath(CageLocationPlug()->getValue());

	IECore::ConstObjectPtr cageObject = inPlug()->object(cageScenePath);
	const auto cagePrimitive = IECore::runTimeCast<const IECoreScene::Primitive>(cageObject);

	const auto pPrimVarIt = cagePrimitive->variables.find("P");

	if (pPrimVarIt == cagePrimitive->variables.end())
	{
		return inputObject;
	}

	const auto bindTransformIt = cagePrimitive->variables.find(BindTransformPlug()->getValue());
	if (bindTransformIt == cagePrimitive->variables.end())
	{
		return inputObject;
	}

	auto bindTransformData = IECore::runTimeCast<IECore::M44fVectorData>(bindTransformIt->second.expandedData());
	const auto& readableBindTransforms = bindTransformData->readable();

	const auto inputPrimitive = IECore::runTimeCast<const IECoreScene::Primitive>(inputObject);
	auto outputPrimitive = inputPrimitive->copy();

	const std::string weightIndicesPrimvarName = IndicesPrimVarPlug()->getValue();
	const std::string weightValuesPrimvarName = WeightsPrimVarPlug()->getValue();

	const auto weightPrimVarIt = inputPrimitive->variables.find(weightValuesPrimvarName);
	const auto indicesPrimVarIt = inputPrimitive->variables.find(weightIndicesPrimvarName);

	if (weightPrimVarIt == inputPrimitive->variables.end() || indicesPrimVarIt == inputPrimitive->variables.end())
	{
		return inputPrimitive;
	}

	auto inputWeightIndicesData = IECore::runTimeCast<const IECore::IntVectorData>(indicesPrimVarIt->second.expandedData());
	const auto& readableInputIndices = inputWeightIndicesData->readable();

	auto inputWeightData = IECore::runTimeCast<const IECore::FloatVectorData>(weightPrimVarIt->second.expandedData());
	const auto& readableInputWeights = inputWeightData->readable();


	const auto pPrimVarItOutput = outputPrimitive->variables.find("P");

	auto outputPositionData = IECore::runTimeCast<IECore::V3fVectorData>(pPrimVarItOutput->second.expandedData());
	auto& outputPositions = outputPositionData->writable();

	const size_t maxWeights = readableInputIndices.size() / outputPositions.size();
	for (size_t i = 0; i < outputPositions.size(); ++i)
	{
		Imath::V3f originalP = outputPositions[i];
		Imath::V3f deformedP(0.0f, 0.0f, 0.0f);
		for (size_t w = 0; w < maxWeights; ++w)
		{
			const int index = readableInputIndices[i * maxWeights + w];
			const float weight = readableInputWeights[i * maxWeights + w];

			auto pt = originalP * readableBindTransforms[index] ;

			deformedP += weight * pt;
		}
		outputPositions[i] = deformedP;
	}

	outputPrimitive->variables["P"] = IECoreScene::PrimitiveVariable(pPrimVarItOutput->second.interpolation, outputPositionData);
	return outputPrimitive;
}

Gaffer::ValuePlug::CachePolicy MiniGaffer::MiniPointDeformer::processedObjectComputeCachePolicy() const
{
	return ValuePlug::CachePolicy::TaskCollaboration;
}
