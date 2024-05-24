#include <Mini/MiniGrowCurves.h>

#include <IECoreScene/CurvesPrimitive.h>
#include <IECoreScene/PointsPrimitive.h>

namespace MiniGaffer
{
    size_t MiniGrowCurves::g_firstPlugIndex = 0;

    IE_CORE_DEFINERUNTIMETYPED( MiniGaffer::MiniGrowCurves );

    MiniGrowCurves::MiniGrowCurves ( const std::string &name ) : GafferScene::ObjectProcessor( name )
    {
        storeIndexOfNextChild(g_firstPlugIndex);
    }

    bool MiniGrowCurves::affectsProcessedObject( const Gaffer::Plug *input ) const
    {
        if (input == inPlug()->objectPlug())
        {
            return true;
        }
        return false;
    }

    void MiniGrowCurves::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
    {
        ObjectProcessor::hashProcessedObject(path, context, h);
    }

    IECore::ConstObjectPtr MiniGrowCurves::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
    {
        auto inputPrimitive = IECore::runTimeCast<const IECoreScene::Primitive>(inputObject);

        if (!inputPrimitive)
        {
            return inputObject;
        }

        IECore::ConstV3fVectorDataPtr pointData = inputPrimitive->expandedVariableData<IECore::V3fVectorData>("P", IECoreScene::PrimitiveVariable::Interpolation::Vertex);
        IECore::ConstV3fVectorDataPtr normalData = inputPrimitive->expandedVariableData<IECore::V3fVectorData>("N", IECoreScene::PrimitiveVariable::Interpolation::Vertex);
        IECore::ConstIntVectorDataPtr numCVsData = inputPrimitive->expandedVariableData<IECore::IntVectorData>("NumCVs", IECoreScene::PrimitiveVariable::Interpolation::Vertex);
        IECore::ConstFloatVectorDataPtr curveLengthData = inputPrimitive->expandedVariableData<IECore::FloatVectorData>("CurveLength", IECoreScene::PrimitiveVariable::Interpolation::Vertex);

        if (!pointData)
        {
            throw IECore::Exception("Missing P V3f vertex interpolated primvar");
        }

        if (!normalData)
        {
            throw IECore::Exception("Missing N V3f vertex interpolated primvar");
        }

        if (!numCVsData)
        {
            throw IECore::Exception("Missing NumCVs int vertex interpolated primvar");
        }

        if (!curveLengthData)
        {
            throw IECore::Exception("Missing CurveLength float vertex interpolated primvar");
        }

        IECoreScene::CurvesPrimitivePtr newCurves { new IECoreScene::CurvesPrimitive() };

        newCurves->setTopology(numCVsData, IECore::CubicBasisf::linear(), false);

        IECore::V3fVectorDataPtr newPositions =  IECore::V3fVectorDataPtr { new IECore::V3fVectorData };
        auto & writablePositions = newPositions->writable();

        for (int c = 0; c < pointData->readable().size(); ++c)
        {
            const float delta = curveLengthData->readable()[c] / (float)(numCVsData->readable()[c] - 1);

            for (int v = 0; v < numCVsData->readable()[c]; ++v)
            {
                writablePositions.push_back(pointData->readable()[c] + normalData->readable()[c] * float(v) * delta);
            }
        }

        newCurves->variables["P"] = IECoreScene::PrimitiveVariable(IECoreScene::PrimitiveVariable::Interpolation::Vertex, newPositions);

        return newCurves;
    }

    Gaffer::ValuePlug::CachePolicy MiniGrowCurves::processedObjectComputeCachePolicy() const
    {
        return Gaffer::ValuePlug::CachePolicy::Standard;
    }
}