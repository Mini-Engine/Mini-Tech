#include <Mini/MiniResampleCurves.h>

#include <IECore/DataAlgo.h>

#include <IECoreScene/CurvesPrimitiveEvaluator.h>
#include <IECoreScene/CurvesPrimitive.h>

namespace
{
    struct ResamplePrimvars
    {
        ResamplePrimvars(IECoreScene::CurvesPrimitiveEvaluator& evaluator, std::vector<int>& verticesPerCurve, int numVertices)
        : evaluator(evaluator),
          verticesPerCurve(verticesPerCurve),
          numVertices(numVertices)
        {}

        IECoreScene::CurvesPrimitiveEvaluator& evaluator;
        std::vector<int>& verticesPerCurve;
        int numVertices;

        IECore::DataPtr operator()( const IECore::Color3fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto result = evaluator.createResult();
            IECore::Color3fVectorDataPtr newArray{ new IECore::Color3fVectorData };

            newArray->writable().resize(numVertices);
            int index = 0;
            for (size_t c = 0; c < verticesPerCurve.size(); ++c)
            {
                for (size_t v = 0; v < verticesPerCurve[c]; ++v)
                {
                    const float param = v / static_cast<float>(verticesPerCurve[c] - 1);
                    evaluator.pointAtV(c, param, result.get());

                    newArray->writable()[index++] = result->colorPrimVar(primitiveVariable);
                }
            }

            return newArray;
        }

        IECore::DataPtr operator()( const IECore::V2fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto result = evaluator.createResult();
            IECore::V2fVectorDataPtr newArray{ new IECore::V2fVectorData };

            newArray->writable().resize(numVertices);
            int index = 0;
            for (size_t c = 0; c < verticesPerCurve.size(); ++c)
            {
                for (size_t v = 0; v < verticesPerCurve[c]; ++v)
                {
                    const float param = v / static_cast<float>(verticesPerCurve[c] - 1);
                    evaluator.pointAtV(c, param, result.get());

                    newArray->writable()[index++] = result->vec2PrimVar(primitiveVariable);
                }
            }

            return newArray;
        }

        IECore::DataPtr operator()( const IECore::V3fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto result = evaluator.createResult();
            IECore::V3fVectorDataPtr newArray{ new IECore::V3fVectorData };

            newArray->writable().resize(numVertices);
            int index = 0;
            for (size_t c = 0; c < verticesPerCurve.size(); ++c)
            {
                for (size_t v = 0; v < verticesPerCurve[c]; ++v)
                {
                    const float param = v / static_cast<float>(verticesPerCurve[c] - 1);
                    evaluator.pointAtV(c, param, result.get());

                    newArray->writable()[index++] = result->vectorPrimVar(primitiveVariable);
                }
            }

            return newArray;
        }

        IECore::DataPtr operator()( const IECore::FloatVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto result = evaluator.createResult();
            IECore::FloatVectorDataPtr newArray{ new IECore::FloatVectorData };

            newArray->writable().resize(numVertices);
            int index = 0;
            for (size_t c = 0; c < verticesPerCurve.size(); ++c)
            {
                for (size_t v = 0; v < verticesPerCurve[c]; ++v)
                {
                    const float param = v / static_cast<float>(verticesPerCurve[c] - 1);
                    evaluator.pointAtV(c, param, result.get());

                    newArray->writable()[index++] = result->floatPrimVar(primitiveVariable);
                }
            }

            return newArray;
        }

        IECore::DataPtr operator()( const IECore::IntVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto result = evaluator.createResult();
            IECore::IntVectorDataPtr newArray{ new IECore::IntVectorData };

            newArray->writable().resize(numVertices);
            int index = 0;
            for (size_t c = 0; c < verticesPerCurve.size(); ++c)
            {
                for (size_t v = 0; v < verticesPerCurve[c]; ++v)
                {
                    const float param = v / static_cast<float>(verticesPerCurve[c] - 1);
                    evaluator.pointAtV(c, param, result.get());

                    newArray->writable()[index++] = result->intPrimVar(primitiveVariable);
                }
            }

            return newArray;
        }

        IECore::DataPtr operator()( const IECore::Data *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            return nullptr;
            //throw IECore::Exception( fmt::format( "PrimitiveVariable \"{}\" has unsupported type \"{}\"", name, data->typeName() ) );
        }
    };

    int DivRoundUp(int A, int B)
    {
        return (A + (B-1)) / B;
    };
}

namespace MiniGaffer
{

    IE_CORE_DEFINERUNTIMETYPED(MiniGaffer::MiniResampleCurves);

    size_t MiniResampleCurves::g_firstPlugIndex = 0;

    MiniResampleCurves::MiniResampleCurves(const std::string &name) : GafferScene::ObjectProcessor(name)
    {
        storeIndexOfNextChild(g_firstPlugIndex);
        addChild( new Gaffer::IntPlug( "mode", Gaffer::Plug::In, 1 ) );
        addChild( new Gaffer::IntPlug( "numControlPoints", Gaffer::Plug::In, 5, 2) );
        addChild( new Gaffer::FloatPlug( "fraction", Gaffer::Plug::In, 1.0f, 0.0f, 1000.0f ) );
    }

    Gaffer::IntPlug *MiniResampleCurves::modePlug()
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }

    const Gaffer::IntPlug *MiniResampleCurves::modePlug() const
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }

    Gaffer::IntPlug *MiniResampleCurves::numControlPointsPlug()
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 1 );
    }

    const Gaffer::IntPlug *MiniResampleCurves::numControlPointsPlug() const
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 1 );
    }

    Gaffer::FloatPlug *MiniResampleCurves::factionPlug()
    {
        return getChild<Gaffer::FloatPlug>( g_firstPlugIndex + 2 );
    }

    const Gaffer::FloatPlug *MiniResampleCurves::factionPlug() const
    {
        return getChild<Gaffer::FloatPlug>( g_firstPlugIndex + 2 );
    }

    bool MiniResampleCurves::affectsProcessedObject( const Gaffer::Plug *input ) const
    {
        if (GafferScene::ObjectProcessor::affectsProcessedObject(input))
        {
            return true;
        }

        if (input == inPlug()->objectPlug() ||
            input == numControlPointsPlug() ||
            input == factionPlug() ||
            input == modePlug())
        {
            return true;
        }
        return false;
    }

    void MiniResampleCurves::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
    {
        GafferScene::ObjectProcessor::hashProcessedObject(path, context, h);
        numControlPointsPlug()->hash(h);
        factionPlug()->hash(h);
        modePlug()->hash(h);

        // todo we get all matching filters here I assume - we should just hash assuming it's something we can use in the compute?
    }

    IECore::ConstObjectPtr MiniResampleCurves::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
    {
        auto srcCurvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>(inputObject);
        if (!srcCurvesPrimitive)
        {
            return inputObject;
        }

        const auto& verticesPerCurve = srcCurvesPrimitive->verticesPerCurve()->readable();
        const float fraction = factionPlug()->getValue();

        IECoreScene::CurvesPrimitivePtr resampledCurves { new IECoreScene::CurvesPrimitive };

        IECore::IntVectorDataPtr newVertsPerCurve { new IECore::IntVectorData };
        auto& writableNewVertsPerCurve = newVertsPerCurve->writable();
        writableNewVertsPerCurve.resize(srcCurvesPrimitive->numCurves());

        IECore::V3fVectorDataPtr newPositions { new IECore::V3fVectorData  };

        int numVertices = 0;
        int mode = modePlug()->getValue();
        int fixedNumberOfControlPoints = numControlPointsPlug()->getValue();


        auto fixCVCount = [basis = srcCurvesPrimitive->basis().standardBasis()](int count)
        {
            if (basis == IECore::StandardCubicBasis::Linear)
            {
                if (count < 2)
                    return 2;

                return count;
            }

            if (count < 4)
            {
                return 4;
            }

            if (basis == IECore::StandardCubicBasis::Bezier)
            {
                // minimum of 4 CVs and then we add segments with 3 additional CVs
                return DivRoundUp(count - 4, 3) * 3 + 4;
            }

            return count;
        };

        // todo linear / bezier -> bspline / catmull rom (create phantom points at the begining and end)
        // todo bspline -> catmull rom -> linear / bezier (check for and remove phantom points)
        // todo math for where these points should be are in the USD Curves docs

        // todo this has to be number of segments or make sure the number of vertices is valid
        for (size_t c = 0; c < verticesPerCurve.size(); ++c)
        {
            writableNewVertsPerCurve[c] = fixCVCount(mode ? static_cast<int>(verticesPerCurve[c] * fraction) : fixedNumberOfControlPoints);
            numVertices += writableNewVertsPerCurve[c];
        }

        IECoreScene::CurvesPrimitiveEvaluator evaluator (srcCurvesPrimitive);
        auto result = evaluator.createResult();

        resampledCurves->setTopology(newVertsPerCurve, srcCurvesPrimitive->basis(), srcCurvesPrimitive->periodic());

        ResamplePrimvars resampler(evaluator, writableNewVertsPerCurve, numVertices);
        for (const auto primvar : srcCurvesPrimitive->variables)
        {
            if (
                    primvar.second.interpolation == IECoreScene::PrimitiveVariable::Uniform ||
                    primvar.second.interpolation == IECoreScene::PrimitiveVariable::Constant)
            {
                resampledCurves->variables[primvar.first] = IECoreScene::PrimitiveVariable(primvar.second.interpolation,
                                                                                           primvar.second.data->copy());
                continue;
            }
            const IECore::DataPtr resampledData = IECore::dispatch( primvar.second.data.get(), resampler, primvar.first, primvar.second );
            if (resampledData)
            {
                resampledCurves->variables[primvar.first] = IECoreScene::PrimitiveVariable(primvar.second.interpolation,
                                                                                           resampledData);
            }
        }
        return resampledCurves;
    }

    Gaffer::ValuePlug::CachePolicy MiniResampleCurves::processedObjectComputeCachePolicy() const
    {
        return Gaffer::ValuePlug::CachePolicy::Standard;
    }


}