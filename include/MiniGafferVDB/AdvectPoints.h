#pragma once

#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "GafferScene/ScenePlug.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{
    class MINIGAFFERVDB_API AdvectPoints : public GafferScene::SceneElementProcessor
    {

    public :

        AdvectPoints(const std::string &name = defaultName<AdvectPoints>() );
        ~AdvectPoints();

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::AdvectPoints, TypeId::AdvectPointsTypeId, GafferScene::SceneElementProcessor );

        Gaffer::StringPlug *velocityLocationPlug();
        const Gaffer::StringPlug *velocityLocationPlug() const;

        Gaffer::StringPlug *velocityGridPlug();
        const Gaffer::StringPlug *velocityGridPlug() const;

        Gaffer::FloatPlug *startFramePlug();
        const Gaffer::FloatPlug *startFramePlug() const;

        Gaffer::IntPlug *subStepsPlug();
        const Gaffer::IntPlug *subStepsPlug() const;

        virtual void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

    protected :

        bool processesObject() const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

        bool processesBound() const override;
        void hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        Imath::Box3f computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const override;

    private:

        static size_t g_firstPlugIndex;
    };

    IE_CORE_DECLAREPTR( AdvectPoints )

}


