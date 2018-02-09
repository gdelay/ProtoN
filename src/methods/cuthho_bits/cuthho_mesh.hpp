/*
 *       /\        Matteo Cicuttin (C) 2017,2018
 *      /__\       matteo.cicuttin@enpc.fr
 *     /_\/_\      École Nationale des Ponts et Chaussées - CERMICS
 *    /\    /\
 *   /__\  /__\    This is ProtoN, a library for fast Prototyping of
 *  /_\/_\/_\/_\   Numerical methods.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * If you use this code or parts of it for scientific publications, you
 * are required to cite it as following:
 *
 * Implementation of Discontinuous Skeletal methods on arbitrary-dimensional,
 * polytopal meshes using generic programming.
 * M. Cicuttin, D. A. Di Pietro, A. Ern.
 * Journal of Computational and Applied Mathematics.
 * DOI: 10.1016/j.cam.2017.09.017
 */

#pragma once

#include <array>
#include <cassert>

#include "core/core"

enum class element_location {
    IN_NEGATIVE_SIDE,
    IN_POSITIVE_SIDE,
    ON_INTERFACE,
    UNDEF
};

template<typename T>
struct cell_cuthho_info
{
    element_location            location;
    point<T,2>                  p0, p1;
    std::vector<point<T,2>>     interface;
    bool                        distorted;

    cell_cuthho_info() :
        location(element_location::UNDEF), distorted(false)
    {}
};



template<typename T>
struct face_cuthho_info
{
    element_location    location;
    size_t              node_inside; /* tells which is the node inside the area
                                        delimited by interface. Can be 0 or 1,
                                        nothing else. */
    point<T, 2>         intersection_point;

    face_cuthho_info() :
        location(element_location::UNDEF),
        node_inside(0)
    {}
};

template<typename T>
struct node_cuthho_info
{
    element_location    location;
    //bool                too_close;
    //T                   displacement;
    bool                displaced;
    point<T,2>          displacement;

    node_cuthho_info() :
        location(element_location::UNDEF), displaced(false)
    {}
};


template<typename T, typename ET>
using cuthho_mesh = mesh<T, ET, cell_cuthho_info<T>, face_cuthho_info<T>, node_cuthho_info<T>>;

template<typename T>
using cuthho_quad_mesh = mesh<T, elem_quad, cell_cuthho_info<T>, face_cuthho_info<T>, node_cuthho_info<T>>;





