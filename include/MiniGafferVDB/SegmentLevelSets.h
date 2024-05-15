#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "GafferScene/ScenePlug.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

	class MINIGAFFERVDB_API SegmentLevelSets : public GafferScene::SceneElementProcessor
	{

	public :

		SegmentLevelSets(const std::string &name = defaultName<SegmentLevelSets>() );
		~SegmentLevelSets();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGafferVDB::SegmentLevelSets, TypeId::SegmentGridsTypeId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridsPlug();
		const Gaffer::StringPlug *gridsPlug() const;

        Gaffer::StringPlug *outputGridPlug();
        const Gaffer::StringPlug *outputGridPlug() const;

		virtual void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

        bool processesObject() const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

	private:

		static size_t g_firstPlugIndex;
	};

	IE_CORE_DECLAREPTR(SegmentLevelSets )

}

