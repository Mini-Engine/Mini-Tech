#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

	class MINIGAFFERVDB_API PointsToLevelSet : public GafferScene::SceneElementProcessor
	{

	public :

		PointsToLevelSet(const std::string &name = defaultName<PointsToLevelSet>() );
		~PointsToLevelSet();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::PointsToLevelSet, TypeId::PointsToLevelSetTypeId, GafferScene::SceneElementProcessor );

		GafferScene::ScenePlug *otherPlug();
		const GafferScene::ScenePlug *otherPlug() const;

		Gaffer::StringPlug *pointsLocationPlug();
		const Gaffer::StringPlug *pointsLocationPlug() const;

        Gaffer::FloatPlug *radiusScalePlug();
        const Gaffer::FloatPlug *radiusScalePlug() const;

        Gaffer::FloatPlug *velocityScalePlug();
        const Gaffer::FloatPlug *velocityScalePlug() const;

        Gaffer::BoolPlug *trailsPlug();
        const Gaffer::BoolPlug *trailsPlug() const;

        Gaffer::FloatPlug *trailDeltaPlug();
        const Gaffer::FloatPlug *trailDeltaPlug() const;

		Gaffer::StringPlug *gridPlug();
		const Gaffer::StringPlug *gridPlug() const;

        Gaffer::StringPlug *velocityAttributePlug();
        const Gaffer::StringPlug *velocityAttributePlug() const;

        Gaffer::StringPlug *widthAttributePlug();
        const Gaffer::StringPlug *widthAttributePlug() const;

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

	IE_CORE_DECLAREPTR( PointsToLevelSet )

}
