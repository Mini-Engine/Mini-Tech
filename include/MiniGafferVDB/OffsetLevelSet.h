#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

class MINIGAFFERVDB_API OffsetLevelSet : public GafferScene::SceneElementProcessor
{

	public :

		OffsetLevelSet(const std::string &name = defaultName<OffsetLevelSet>() );
		~OffsetLevelSet() override;

		GAFFER_GRAPHCOMPONENT_DECLARE_TYPE(MiniGafferVDB::OffsetLevelSet, TypeId::LevelSetOffsetTypeId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridsPlug();
		const Gaffer::StringPlug *gridsPlug() const;

        Gaffer::StringPlug *outputGridPlug();
        const Gaffer::StringPlug *outputGridPlug() const;

		Gaffer::FloatPlug *offsetPlug();
		const Gaffer::FloatPlug *offsetPlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

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

IE_CORE_DECLAREPTR(OffsetLevelSet )

}
