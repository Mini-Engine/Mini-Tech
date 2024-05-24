#pragma once

namespace MiniGaffer {

    enum class TypeId {
        
        FirstTypeId = 115000,

        MiniResampleCurvesTypeId = 115001,
        MiniMeasureCurvesTypeId = 115002,
        MiniMergeGeometryTypeId = 115003,
        MiniSubdivideMeshTypeId = 115004,
        MiniBlurMeshAttributesTypeId = 115005,
        MeshCurvatureTypeId = 115006,
        BlurPrimitiveVariablesTypeId = 115007,
        MiniCurvesTypeId = 115008,
        MiniBulletTypeId = 115009,
        MiniPointDeformTypeId = 115010,
        MiniPointBindTypeId = 115011,
        MiniGrowCurvesTypeId = 115012,
        LastTypeId = 115099,
    };

} // Mini