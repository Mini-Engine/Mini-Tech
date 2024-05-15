#include <Mini/MiniMergeGeometry.h>

#include <fmt/format.h>

#include "Gaffer/StringPlug.h"
#include "Gaffer/Path.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/CurvesAlgo.h"
#include "IECoreScene/PointsAlgo.h"

#include "GafferScene/SceneAlgo.h"

#include "IECore/InternedString.h"
#include "IECore/StringAlgo.h"
#include "IECoreScene/CurvesMergeOp.h"


// todo - pedantic mode (checking primitive types of input and primvar types & interpolation),
// todo - what if the output location doesn't exist?

namespace
{
    GafferScene::ScenePlug::ScenePath makeScenePath (const std::string& p)
    {
        GafferScene::ScenePlug::ScenePath output;
        IECore::StringAlgo::tokenize<IECore::InternedString>( p, '/', back_inserter( output ) );
        return output;
    };

    IECoreScene::ConstCurvesPrimitivePtr mergeCurves(const std::vector<const IECoreScene::CurvesPrimitive*> &curvesPrimitives, const IECore::Canceller *canceller  = nullptr  )
    {

        if (curvesPrimitives.empty())
        {
            return IECoreScene::CurvesPrimitivePtr { new IECoreScene::CurvesPrimitive };

        }
        IECoreScene::CurvesPrimitivePtr NewCurves { curvesPrimitives[0]->copy() };
        IECoreScene::CurvesMergeOp CurveMerge;

        IECore::ObjectPtr ptr = NewCurves;

        for( size_t c = 1; c <  curvesPrimitives.size(); ++c)
        {
            const auto curves = curvesPrimitives[c];
            CurveMerge.inputParameter()->setValue( ptr );
            CurveMerge.curvesParameter()->setValue( curves->copy() );
            ptr = CurveMerge.operate();
        }

        return IECore::runTimeCast<const IECoreScene::CurvesPrimitive>(ptr);
    }

    // missing privar
    // primvar with a different type / interpolation

    struct PrimvarMismatch
    {
        std::string path;
        std::string name;

        IECoreScene::PrimitiveVariable::Interpolation interpolation;
        IECore::StringVectorDataPtr Test;
    };

    // todo enable warnings as errors
    template<typename T>
    std::vector<PrimvarMismatch> CheckPrimvars(const std::vector<T>& primitives)
    {
        return std::vector<PrimvarMismatch>();

    }
}

namespace MiniGaffer
{
    GAFFER_NODE_DEFINE_TYPE( MiniMergeGeometry );

    size_t MiniMergeGeometry::g_firstPlugIndex = 0;

    MiniMergeGeometry::MiniMergeGeometry ( const std::string &name )
    : GafferScene::SceneProcessor( name )
    {
        storeIndexOfNextChild( g_firstPlugIndex );

        addChild(new GafferScene::FilterPlug("filter"));
        addChild(new Gaffer::StringPlug ("mergedName", Gaffer::Plug::In, "merged"));
        addChild(new Gaffer::BoolPlug ("pedantic", Gaffer::Plug::In, false));

        outPlug()->globalsPlug()->setInput(inPlug()->globalsPlug());
    }

    GafferScene::FilterPlug *MiniMergeGeometry::filterPlug()
    {
        return getChild<GafferScene::FilterPlug>( g_firstPlugIndex + 0 );
    }
    const GafferScene::FilterPlug *MiniMergeGeometry::filterPlug() const
    {
        return getChild<GafferScene::FilterPlug>( g_firstPlugIndex + 0 );
    }

    Gaffer::StringPlug *MiniMergeGeometry::mergedNamePlug()
    {
        return getChild<Gaffer::StringPlug>( g_firstPlugIndex + 1 );
    }

    const Gaffer::StringPlug *MiniMergeGeometry::mergedNamePlug() const
    {
        return getChild<Gaffer::StringPlug>( g_firstPlugIndex + 1 );
    }

    const Gaffer::BoolPlug *MiniMergeGeometry::pedanticPlug() const
    {
        return getChild<Gaffer::BoolPlug>( g_firstPlugIndex + 2 );
    }

    Gaffer::BoolPlug *MiniMergeGeometry::pedanticPlug()
    {
        return getChild<Gaffer::BoolPlug>( g_firstPlugIndex + 2 );
    }

    void MiniMergeGeometry::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
    {
        SceneProcessor::affects(input, outputs);

        if (input == filterPlug() ||
            input == mergedNamePlug() ||
            input == pedanticPlug() ||
            input == inPlug() ||
            input == inPlug()->objectPlug()
            )
        {
            outputs.push_back(outPlug()->objectPlug());
        }

        if (input == mergedNamePlug())
        {
            outputs.push_back(outPlug()->childNamesPlug());
        }
    }

    void MiniMergeGeometry::hashBound( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashBound(path, context, parent, h);
        if (path.empty())
        {
            return;
        }

        h.append(GafferScene::SceneAlgo::matchingPathsHash(filterPlug(), inPlug()));

        IECore::PathMatcher pathMatcher;
        GafferScene::SceneAlgo::matchingPaths(filterPlug(), inPlug(), pathMatcher);

        std::vector<const IECoreScene::MeshPrimitive*> meshes;

        for(IECore::PathMatcher::Iterator it = pathMatcher.begin(); it != pathMatcher.end(); ++it)
        {
            h.append(inPlug()->objectHash(*it));
        }
    }

    void MiniMergeGeometry::hashTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashTransform(path, context, parent, h);
    }

    void MiniMergeGeometry::hashAttributes( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashAttributes(path, context, parent, h);
    }

    void MiniMergeGeometry::hashObject( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashObject(path, context, parent, h);
        if (path.empty())
        {
            return;
        }

        pedanticPlug()->hash(h);
        h.append(GafferScene::SceneAlgo::matchingPathsHash(filterPlug(), inPlug()));

        IECore::PathMatcher pathMatcher;
        GafferScene::SceneAlgo::matchingPaths(filterPlug(), inPlug(), pathMatcher);

        std::vector<const IECoreScene::MeshPrimitive*> meshes;

        for(IECore::PathMatcher::Iterator it = pathMatcher.begin(); it != pathMatcher.end(); ++it)
        {
            h.append(inPlug()->objectHash(*it));
        }

    }

    void MiniMergeGeometry::hashChildNames( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashChildNames(path, context, parent, h);
        if (path.empty())
        {
            mergedNamePlug()->hash(h);
        }
    }

    void MiniMergeGeometry::hashSetNames( const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashSetNames(context, parent, h);
    }

    void MiniMergeGeometry::hashSet( const IECore::InternedString &setName, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        SceneProcessor::hashSet(setName, context, parent, h);
    }

    Imath::Box3f MiniMergeGeometry::computeBound( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        Imath::Box3f bounds;

        IECore::PathMatcher pathMatcher;
        GafferScene::SceneAlgo::matchingPaths(filterPlug(), inPlug(), pathMatcher);

        std::vector<const IECoreScene::MeshPrimitive*> meshes;

        for(IECore::PathMatcher::Iterator it = pathMatcher.begin(); it != pathMatcher.end(); ++it)
        {
            bounds.extendBy(inPlug()->bound(*it));
        }

        return bounds;
    }

    Imath::M44f MiniMergeGeometry::computeTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        return Imath::M44f ();
    }

    IECore::ConstCompoundObjectPtr MiniMergeGeometry::computeAttributes( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        return IECore::ConstCompoundObjectPtr{ new IECore::CompoundObject{} } ;
    }

    IECore::ConstObjectPtr MiniMergeGeometry::computeObject( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        if (path.empty())
        {
            return IECore::ConstObjectPtr{};
        }

        IECore::PathMatcher pathMatcher;
        GafferScene::SceneAlgo::matchingPaths(filterPlug(), inPlug(), pathMatcher);

        std::vector<const IECoreScene::MeshPrimitive*> meshes;
        std::vector<const IECoreScene::CurvesPrimitive*> curves;
        std::vector<const IECoreScene::PointsPrimitive*> points;

        unsigned int numPrimitives = 0;
        for(IECore::PathMatcher::Iterator it = pathMatcher.begin(); it != pathMatcher.end(); ++it)
        {
            if ( IECoreScene::ConstMeshPrimitivePtr meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>(inPlug()->object(*it)); meshPrimitive )
            {
                meshes.push_back(meshPrimitive.get());
                numPrimitives++;
            }
            else if ( IECoreScene::ConstPointsPrimitivePtr pointsPrimitive = IECore::runTimeCast<const IECoreScene::PointsPrimitive>(inPlug()->object(*it ) ); pointsPrimitive )
            {
                points.push_back(pointsPrimitive.get());
                numPrimitives++;
            }
            else if ( IECoreScene::ConstCurvesPrimitivePtr curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>(inPlug()->object(*it ) ); curvesPrimitive )
            {
                curves.push_back(curvesPrimitive.get());
                numPrimitives++;
            }
        }

        if (numPrimitives == 0)
        {
            return  new IECoreScene::MeshPrimitive;
        }
        // todo check primvars of all primitives
        // todo check all object types match
        if (numPrimitives == meshes.size())
        {
            const auto PrimvarWarnings = CheckPrimvars(meshes);

            return IECoreScene::MeshAlgo::merge(meshes);
        }

        if (numPrimitives == points.size())
        {
            const auto PrimvarWarnings = CheckPrimvars(points);
            return IECoreScene::PointsAlgo::mergePoints(points);
        }

        if (numPrimitives == curves.size())
        {
            const auto PrimvarWarnings = CheckPrimvars(curves);
            return mergeCurves(curves);
        }

        throw IECore::Exception( fmt::format( "Filtered locations don't contain the same primitive type" ) );

        return nullptr;
    }

    IECore::ConstInternedStringVectorDataPtr MiniMergeGeometry::computeChildNames( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        auto data = new IECore::InternedStringVectorData;
        if (path.empty())
        {
            data->writable().emplace_back(mergedNamePlug()->getValue());
        }
        return  IECore::ConstInternedStringVectorDataPtr { data };
    }

    IECore::ConstInternedStringVectorDataPtr MiniMergeGeometry::computeSetNames( const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        auto data = new IECore::InternedStringVectorData;
        return IECore::ConstInternedStringVectorDataPtr { data };
    }

    IECore::ConstPathMatcherDataPtr MiniMergeGeometry::computeSet( const IECore::InternedString &setName, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        return IECore::ConstPathMatcherDataPtr { new IECore::PathMatcherData {} } ;
    }


}