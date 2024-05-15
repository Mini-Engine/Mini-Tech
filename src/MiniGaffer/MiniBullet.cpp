#include <Mini/MiniBullet.h>

#include <GafferScene/ScenePlug.h>
#include <GafferScene/SceneAlgo.h>

#include <string>

namespace MiniGaffer
{
    GAFFER_NODE_DEFINE_TYPE( MiniBullet );

    size_t MiniBullet::g_firstPlugIndex = 0;

    MiniBullet::MiniBullet ( const std::string &name )
            : GafferScene::SceneProcessor( name )
    {
        storeIndexOfNextChild( g_firstPlugIndex );

        addChild(new Gaffer::FloatPlug ("frame", Gaffer::Plug::In, 50.0f));

        outPlug()->boundPlug()->setInput(inPlug()->boundPlug());
        outPlug()->attributesPlug()->setInput(inPlug()->attributesPlug());
        outPlug()->objectPlug()->setInput(inPlug()->objectPlug());
        outPlug()->childNamesPlug()->setInput(inPlug()->childNamesPlug());
        outPlug()->existsPlug()->setInput(inPlug()->existsPlug());
        outPlug()->childBoundsPlug()->setInput(inPlug()->childBoundsPlug());
        outPlug()->globalsPlug()->setInput(inPlug()->globalsPlug());
        outPlug()->setNamesPlug()->setInput(inPlug()->setNamesPlug());
        outPlug()->setPlug()->setInput(inPlug()->setPlug());
    }

    void MiniBullet::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
    {
        SceneProcessor::affects(input, outputs);

        if (input == framePlug())
        {
            outputs.push_back(outPlug()->transformPlug());
        }
    }

    Gaffer::FloatPlug *MiniBullet::framePlug()
    {
        return getChild<Gaffer::FloatPlug>( g_firstPlugIndex + 0 );
    }

    const Gaffer::FloatPlug *MiniBullet::framePlug() const
    {
        return getChild<Gaffer::FloatPlug>( g_firstPlugIndex + 0 );
    }

    void MiniBullet::hashTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const
    {
        auto physicsSceneHash = Solver::HashPhysicsInputScene( inPlug() );

        SceneProcessor::hashTransform(path, context, parent, h);
        h.append(physicsSceneHash);
        h.append(context->getFrame());
        inPlug()->transformPlug()->hash(h);
        framePlug()->hash(h);
    }

    Imath::M44f MiniBullet::computeTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const
    {
        const float frame = context->get<float>("frame");
        {
            auto solver { Solver::FindOrCreateSolver(inPlug(), frame) };

            MiniGaffer::Solver::MeshHandle handle = solver->FindHandle(path);

            if (handle != -1)
            {
                Imath::M44f transform = solver->GetTransform(handle);
                return transform;
            }
        }

        return inPlug()->transform(path);

    }
}