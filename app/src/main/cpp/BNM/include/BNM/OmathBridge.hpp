#pragma once

#include <omath/omath.hpp>
#include "UnityStructures/Vector2.hpp"
#include "UnityStructures/Vector3.hpp"
#include "UnityStructures/Vector4.hpp"
#include "UnityStructures/Matrix4x4.hpp"
#include "UnityStructures/Quaternion.hpp"

namespace BNM::OmathBridge {

    // Vector2
    inline omath::Vector2<float> ToOmath(const Structures::Unity::Vector2& v) { return {v.x, v.y}; }
    inline Structures::Unity::Vector2 FromOmath(const omath::Vector2<float>& v) { return {v.x, v.y}; }

    // Vector3
    inline omath::Vector3<float> ToOmath(const Structures::Unity::Vector3& v) { return {v.x, v.y, v.z}; }
    inline Structures::Unity::Vector3 FromOmath(const omath::Vector3<float>& v) { return {v.x, v.y, v.z}; }

    // Vector4
    inline omath::Vector4<float> ToOmath(const Structures::Unity::Vector4& v) { return {v.x, v.y, v.z, v.w}; }
    inline Structures::Unity::Vector4 FromOmath(const omath::Vector4<float>& v) { return {v.x, v.y, v.z, v.w}; }

    // Quaternion
    inline omath::Quaternion<float> ToOmath(const Structures::Unity::Quaternion& q) { return {q.x, q.y, q.z, q.w}; }
    inline Structures::Unity::Quaternion FromOmath(const omath::Quaternion<float>& q) { return {q.x, q.y, q.z, q.w}; }

    // Matrix4x4
    // Unity matrices are Column-Major.
    // omath::Mat defaults to ROW_MAJOR, so we specify COLUMN_MAJOR for direct data copying if needed,
    // or just use at(row, col) for safety.
    inline omath::Mat<4, 4, float, omath::MatStoreType::COLUMN_MAJOR> ToOmath(const Structures::Unity::Matrix4x4& m) {
        omath::Mat<4, 4, float, omath::MatStoreType::COLUMN_MAJOR> res;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                res.at(row, col) = m.Get(row, col);
        return res;
    }
    inline Structures::Unity::Matrix4x4 FromOmath(const omath::Mat<4, 4, float, omath::MatStoreType::COLUMN_MAJOR>& m) {
        Structures::Unity::Matrix4x4 res;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                res.Get(row, col) = m.at(row, col);
        return res;
    }
}
