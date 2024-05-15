#pragma once

#include "MiniGafferVDB/Export.h"
#include "GafferScene/ObjectSource.h"

#include "Gaffer/CompoundNumericPlug.h"

#include "MiniGafferVDB/TypeIds.h"

namespace MiniGafferVDB
{

    class MINIGAFFERVDB_API SphereLevelSet : public GafferScene::ObjectSource
    {

    public :

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::SphereLevelSet, TypeId::SphereLevelSetTypeId, ObjectSource );

        SphereLevelSet( const std::string &name=defaultName<SphereLevelSet>() );
        ~SphereLevelSet() override;

        Gaffer::StringPlug *gridPlug();
        const Gaffer::StringPlug *gridPlug() const;

        Gaffer::FloatPlug *radiusPlug();
        const Gaffer::FloatPlug *radiusPlug() const;

        Gaffer::V3fPlug *centerPlug();
        const Gaffer::V3fPlug *centerPlug() const;

        Gaffer::FloatPlug *voxelSizePlug();
        const Gaffer::FloatPlug *voxelSizePlug() const;

        Gaffer::FloatPlug *halfWidthPlug();
        const Gaffer::FloatPlug *halfWidthPlug() const;

        void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

    protected :

        void hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeSource( const Gaffer::Context *context ) const override;

    private :

        static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( SphereLevelSet )

}
