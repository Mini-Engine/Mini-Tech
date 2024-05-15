#include <Mini/MiniCurvesType.h>

#include <IECoreScene/CurvesPrimitiveEvaluator.h>
#include <IECoreScene/CurvesPrimitive.h>

namespace MiniGaffer
{

    size_t MiniCurvesType::g_firstPlugIndex = 0;

    IE_CORE_DEFINERUNTIMETYPED( MiniGaffer::MiniCurvesType );

    MiniCurvesType::MiniCurvesType ( const std::string &name ) : GafferScene::ObjectProcessor( name )
    {
        storeIndexOfNextChild(g_firstPlugIndex);
        addChild(new Gaffer::IntPlug ("basis", Gaffer::Plug::In, 0));
    }

    Gaffer::IntPlug *MiniCurvesType::basisPlug()
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }

    const Gaffer::IntPlug *MiniCurvesType::basisPlug() const
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }

    bool MiniCurvesType::affectsProcessedObject( const Gaffer::Plug *input ) const
    {
        if (input == inPlug()->objectPlug() || input == basisPlug())
        {
            return true;
        }
        return false;
    }

    void MiniCurvesType::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
    {
        ObjectProcessor::hashProcessedObject(path, context, h);
        basisPlug()->hash(h);
    }

    IECore::ConstObjectPtr MiniCurvesType::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
    {
        auto curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>(inputObject);

        if (!curvesPrimitive)
        {
            return inputObject;
        }

        auto newCurves = curvesPrimitive->copy();

        int basis = basisPlug()->getValue();
        switch(basis)
        {
            case 0:
                newCurves->setTopology(curvesPrimitive->verticesPerCurve(), IECore::CubicBasisf::linear(), curvesPrimitive->periodic());
                break;

            case 1:
                newCurves->setTopology(curvesPrimitive->verticesPerCurve(), IECore::CubicBasisf::bezier(), curvesPrimitive->periodic());
                break;
            case 2:
                newCurves->setTopology(curvesPrimitive->verticesPerCurve(), IECore::CubicBasisf::bSpline(), curvesPrimitive->periodic());
                break;
            case 3:
                newCurves->setTopology(curvesPrimitive->verticesPerCurve(), IECore::CubicBasisf::catmullRom(), curvesPrimitive->periodic());
                break;
            default:
                throw IECore::Exception( fmt::format( "Unknown curve type: {}", basis));
        }
        return newCurves;
    }

    Gaffer::ValuePlug::CachePolicy MiniCurvesType::processedObjectComputeCachePolicy() const
    {
        return Gaffer::ValuePlug::CachePolicy::Standard;
    }
}