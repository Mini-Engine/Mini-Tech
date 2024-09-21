#include <Mini/MiniMeasureCurves.h>

#include <IECoreScene/CurvesPrimitiveEvaluator.h>
#include <IECoreScene/CurvesPrimitive.h>

namespace MiniGaffer
{

    size_t MiniMeasureCurves::g_firstPlugIndex = 0;

    IE_CORE_DEFINERUNTIMETYPED( MiniGaffer::MiniMeasureCurves );

    MiniMeasureCurves::MiniMeasureCurves ( const std::string &name ) : GafferScene::ObjectProcessor( name )
    {
        storeIndexOfNextChild(g_firstPlugIndex);
        addChild(new Gaffer::StringPlug ("lengthPrimvar", Gaffer::Plug::In, "length"));
        addChild(new Gaffer::BoolPlug ("normalize", Gaffer::Plug::In, false) );
    }

    Gaffer::StringPlug *MiniMeasureCurves::lengthNamePlug()
    {
        return getChild<Gaffer::StringPlug>( g_firstPlugIndex + 0 );
    }

    const Gaffer::StringPlug *MiniMeasureCurves::lengthNamePlug() const
    {
        return getChild<Gaffer::StringPlug>( g_firstPlugIndex + 0 );
    }

    Gaffer::BoolPlug *MiniMeasureCurves::normalizePlug()
    {
        return getChild<Gaffer::BoolPlug>( g_firstPlugIndex + 1 );
    }

    const Gaffer::BoolPlug *MiniMeasureCurves::normalizePlug() const
    {

        return getChild<Gaffer::BoolPlug>( g_firstPlugIndex + 1 );
    }

    bool MiniMeasureCurves::affectsProcessedObject( const Gaffer::Plug *input ) const
    {
        if (input == inPlug()->objectPlug() || input == lengthNamePlug() || input == normalizePlug())
        {
            return true;
        }
        return false;
    }

    void MiniMeasureCurves::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
    {
        ObjectProcessor::hashProcessedObject(path, context, h);
        lengthNamePlug()->hash(h);
        normalizePlug()->hash(h);
    }

    IECore::ConstObjectPtr MiniMeasureCurves::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
    {
        auto curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>(inputObject);

        if (!curvesPrimitive)
        {
            return inputObject;
        }

        IECoreScene::CurvesPrimitiveEvaluator evaluator(curvesPrimitive);

        auto newCurves = curvesPrimitive->copy();
        auto LengthData = IECore::FloatVectorDataPtr { new IECore::FloatVectorData () };

        auto positionData = IECore::runTimeCast<const IECore::V3fVectorData>(newCurves->variables["P"].data);

        auto& lengths = LengthData->writable();
        lengths.resize(positionData->readable().size());

        const auto& vertsPerCurve = newCurves->verticesPerCurve()->readable();

        size_t i = 0;
        const bool normalize = normalizePlug()->getValue();
        for (size_t c = 0; c < newCurves->numCurves(); ++c)
        {
            float previous = 0.0f;
            float lengthAcc = 0.0f;
            const float totalLength = evaluator.curveLength(c);
            for (size_t v = 0; v < vertsPerCurve[c]; ++v)
            {
                const float param = v / static_cast<float>(vertsPerCurve[c] - 1);
                lengthAcc += evaluator.curveLength(c, previous, param);

                lengths[i++] = lengthAcc / (normalize ? totalLength : 1.0f);
                previous = param;
            }

        }

        newCurves->variables[lengthNamePlug()->getValue()] = IECoreScene::PrimitiveVariable(IECoreScene::PrimitiveVariable::Interpolation::Vertex, LengthData);
        return newCurves;
    }

    Gaffer::ValuePlug::CachePolicy MiniMeasureCurves::processedObjectComputeCachePolicy() const
    {
        return Gaffer::ValuePlug::CachePolicy::Standard;
    }
}