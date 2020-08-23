// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <nanoflann.hpp>
#include <vector>

#include "open3d/core/Tensor.h"

namespace open3d {
namespace core {
namespace nn {

enum Metric { L1, L2, Linf };

/// This class is the Adaptor for connecting Open3D Tensor and NanoFlann.
template <class T>
class Adaptor {
public:
    Adaptor(size_t num_points, int dimension, const T *const data)
        : num_points(num_points), dimension(dimension), data(data) {}

    inline size_t kdtree_get_point_count() const { return num_points; }

    inline T kdtree_get_pt(const size_t idx, const size_t dim) const {
        return data[idx * dimension + dim];
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX &) const {
        return false;
    }

private:
    size_t num_points = 0;
    int dimension = 0;
    const T *const data;
};

template <int METRIC, class T>
struct SelectNanoflannAdaptor {};

template <class T>
struct SelectNanoflannAdaptor<L2, T> {
    typedef nanoflann::L2_Adaptor<T, Adaptor<T>> Adaptor_t;
};

template <class T>
struct SelectNanoflannAdaptor<L1, T> {
    typedef nanoflann::L1_Adaptor<T, Adaptor<T>> Adaptor_t;
};

/// \class NanoFlann
///
/// \brief KDTree with NanoFlann for nearest neighbor search.
class NanoFlann {
    typedef nanoflann::KDTreeSingleIndexAdaptor<
            typename SelectNanoflannAdaptor<L2, double>::Adaptor_t,
            Adaptor<double>>
            KDTree_t;

public:
    /// \brief Default Constructor.
    NanoFlann();
    /// \brief Parameterized Constructor
    ///
    /// \param tensor Provides a set of data points as Tensor for KDTree
    /// construction.
    NanoFlann(const core::Tensor &tensor);
    ~NanoFlann();
    NanoFlann(const NanoFlann &) = delete;
    NanoFlann &operator=(const NanoFlann &) = delete;

public:
    /// Set the data for the KDTree from a Tensor.
    ///
    /// \param data Data points for KDTree Construction.
    bool SetTensorData(const core::Tensor &data);
    /// Perform K nearest neighbor search.
    ///
    /// \param query Data points for querying.
    /// \param knn Number of nearest neighbor to search.
    std::pair<core::Tensor, core::Tensor> SearchKnn(const core::Tensor &query,
                                                    int knn);
    /// Perform radius search.
    ///
    /// \param query Data points for querying.
    /// \param radii A list of radius. Same size with query.
    std::tuple<core::Tensor, core::Tensor, core::Tensor> SearchRadius(
            const core::Tensor &query, double *radii);
    /// Perform radius search.
    ///
    /// \param query Data points for querying.
    /// \param radius Radius.
    std::tuple<core::Tensor, core::Tensor, core::Tensor> SearchRadius(
            const core::Tensor &query, double radius);

protected:
    core::Tensor data_;
    std::unique_ptr<KDTree_t> index_;
    std::unique_ptr<Adaptor<double>> adaptor_;
    int dimension_ = 0;
    size_t dataset_size_ = 0;
};
}  // namespace nn
}  // namespace core
}  // namespace open3d
