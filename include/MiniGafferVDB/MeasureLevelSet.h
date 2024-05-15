#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "GafferScene/ScenePlug.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

	class MINIGAFFERVDB_API MeasureLevelSet : public GafferScene::SceneElementProcessor
	{

	public :

		MeasureLevelSet(const std::string &name = defaultName<MeasureLevelSet>() );
		~MeasureLevelSet();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGafferVDB::MeasureLevelSet, TypeId::LevelSetMeasureTypeId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridsPlug();
		const Gaffer::StringPlug *gridsPlug() const;

        Gaffer::BoolPlug *curvaturePlug();
        const Gaffer::BoolPlug *curvaturePlug() const;

        Gaffer::BoolPlug *worldUnitsPlug();
        const Gaffer::BoolPlug *worldUnitsPlug() const;

		virtual void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesAttributes() const override;
		void hashProcessedAttributes( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstCompoundObjectPtr computeProcessedAttributes( const ScenePath &path, const Gaffer::Context *context, IECore::ConstCompoundObjectPtr inputAttributes ) const override;

	private:

		static size_t g_firstPlugIndex;
	};

	IE_CORE_DECLAREPTR(MeasureLevelSet )

}

