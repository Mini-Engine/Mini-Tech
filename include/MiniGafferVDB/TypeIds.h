#pragma once

namespace MiniGafferVDB {

    enum class TypeId {

        FirstTypeId = 115050,
        SampleGridsTypeId = 115051,

        ClipGridsTypeId = 115052,
        VDBObjectTypeId = 115053,
        MorphLevelSetsTypeId = 115054,
        AdvectPointsTypeId = 115055,
        //Unused = 115056,
        LevelSetOffsetTypeId = 115057,
        PointsGridToPointsId = 115058,
        DeleteGridsTypeId = 115059,
        //Unused = 115060,
        AdvectGridsTypeId = 115061,
        MathOpTypeId = 115062,
        StatisticsTypeId = 115063,
        CSGGridsTypeId = 115064,
        TransformGridsTypeId = 115065,
        LevelSetFractureTypeId = 115066,
        PointsToLevelSetTypeId = 115067,
        //Unused = 115068,
        FilterGridsTypeId = 115069,
        LevelSetMeasureTypeId = 115070,
        LevelSetFilterTypeId = 115071,
        VolumeToSpheresTypeId = 115072,
        LevelSetToFogTypeId = 115073,
        SegmentGridsTypeId = 115074,
        IntersectGridsTypeId = 115075,
        CompositeGridsTypeId = 115076,
        PlatonicLevelSetTypeId = 115077,
        SphereLevelSetTypeId = 115078,
        MaskGridsTypeId = 115079,

        LastTypeId = 115099,
    };

} // Mini