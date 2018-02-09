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

/*
 * This source file is part of EMT, the ElectroMagneticTool.
 *
 * Copyright (C) 2013-2015, Matteo Cicuttin - matteo.cicuttin@uniud.it
 * Department of Electrical Engineering, University of Udine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Udine nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(s) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR(s) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <time.h>
#include <sys/resource.h>

#include "bases.hpp"
#include "quadratures.hpp"


class hho_degree_info
{
    size_t  cell_deg, face_deg, reconstruction_deg;

public:
    hho_degree_info()
        : cell_deg(1), face_deg(1), reconstruction_deg(2)
    {}

    explicit hho_degree_info(size_t degree)
        : cell_deg(degree), face_deg(degree), reconstruction_deg(degree+1)
    {}

    hho_degree_info(size_t cd, size_t fd)
    {
        bool c1 = fd > 0  && (cd == fd-1 || cd == fd || cd == fd+1);
        bool c2 = fd == 0 && (cd == fd || cd == fd+1);
        if ( c1 || c2 )
        {
            cell_deg            = cd;
            face_deg            = fd;
            reconstruction_deg  = fd+1;

        }
        else
        {
            std::cout << "Invalid cell degree. Reverting to equal-order" << std::endl;
            cell_deg            = fd;
            face_deg            = fd;
            reconstruction_deg  = fd+1;
        }

        std::cout << cell_deg << " " << face_deg << " " << reconstruction_deg << std::endl;
    }

    size_t cell_degree() const
    {
        return cell_deg;
    }

    size_t face_degree() const
    {
        return face_deg;
    }

    size_t reconstruction_degree() const
    {
        return reconstruction_deg;
    }
};

template<typename Mesh, typename T = typename Mesh::coordinate_type>
Matrix<T, Dynamic, Dynamic>
make_mass_matrix(const Mesh& msh, const typename Mesh::cell_type& cl, size_t degree)
{
    cell_basis<Mesh,T> cb(msh, cl, degree);
    auto cbs = cb.size();

    Matrix<T, Dynamic, Dynamic> ret = Matrix<T, Dynamic, Dynamic>::Zero(cbs, cbs);

    auto qps = integrate(msh, cl, 2*degree);

    for (auto& qp : qps)
    {
        auto phi = cb.eval_basis(qp.first);
        ret += qp.second * phi * phi.transpose();
    }

    return ret;
}

template<typename Mesh, typename T = typename Mesh::coordinate_type>
Matrix<T, Dynamic, Dynamic>
make_mass_matrix(const Mesh& msh, const typename Mesh::face_type& fc, size_t degree)
{
    face_basis<Mesh,T> fb(msh, fc, degree);
    auto fbs = fb.size();

    Matrix<T, Dynamic, Dynamic> ret = Matrix<T, Dynamic, Dynamic>::Zero(fbs, fbs);

    auto qps = integrate(msh, fc, 2*degree);

    for (auto& qp : qps)
    {
        auto phi = fb.eval_basis(qp.first);
        ret += qp.second * phi * phi.transpose();
    }

    return ret;
}

template<typename Mesh, typename Function>
Matrix<typename Mesh::coordinate_type, Dynamic, 1>
make_rhs(const Mesh& msh, const typename Mesh::cell_type& cl,
         size_t degree, const Function& f)
{
    using T = typename Mesh::coordinate_type;

    cell_basis<Mesh,T> cb(msh, cl, degree);
    auto cbs = cb.size();

    Matrix<T, Dynamic, 1> ret = Matrix<T, Dynamic, 1>::Zero(cbs);

    auto qps = integrate(msh, cl, 2*degree);

    for (auto& qp : qps)
    {
        auto phi = cb.eval_basis(qp.first);
        ret += qp.second * phi * f(qp.first);
    }

    return ret;
}

template<typename Mesh, typename Function>
Matrix<typename Mesh::coordinate_type, Dynamic, 1>
make_rhs(const Mesh& msh, const typename Mesh::face_type& fc,
         size_t degree, const Function& f)
{
    using T = typename Mesh::coordinate_type;

    face_basis<Mesh,T> fb(msh, fc, degree);
    auto fbs = fb.size();

    Matrix<T, Dynamic, 1> ret = Matrix<T, Dynamic, 1>::Zero(fbs);

    auto qps = integrate(msh, fc, 2*degree);

    for (auto& qp : qps)
    {
        auto phi = fb.eval_basis(qp.first);
        ret += qp.second * phi * f(qp.first);
    }

    return ret;
}

template<typename Mesh, typename Function>
Matrix<typename Mesh::coordinate_type, Dynamic, 1>
project_function(const Mesh& msh, const typename Mesh::cell_type& cl,
                 hho_degree_info hdi, const Function& f)
{
    using T = typename Mesh::coordinate_type;

    auto cbs = cell_basis<Mesh,T>::size(hdi.cell_degree());
    auto fbs = face_basis<Mesh,T>::size(hdi.face_degree());

    Matrix<T, Dynamic, 1> ret = Matrix<T, Dynamic, 1>::Zero(cbs+4*fbs);

    Matrix<T, Dynamic, Dynamic> cell_mm = make_mass_matrix(msh, cl, hdi.cell_degree());
    Matrix<T, Dynamic, 1> cell_rhs = make_rhs(msh, cl, hdi.cell_degree(), f);
    ret.block(0, 0, cbs, 1) = cell_mm.llt().solve(cell_rhs);

    auto fcs = faces(msh, cl);
    for (size_t i = 0; i < 4; i++)
    {
        auto fc = fcs[i];
        Matrix<T, Dynamic, Dynamic> face_mm = make_mass_matrix(msh, fc, hdi.face_degree());
        Matrix<T, Dynamic, 1> face_rhs = make_rhs(msh, fc, hdi.face_degree(), f);
        ret.block(cbs+i*fbs, 0, fbs, 1) = face_mm.llt().solve(face_rhs);
    }

    return ret;
}

template<typename T>
T condition_number(const Matrix<T, Dynamic, Dynamic>& A)
{
    JacobiSVD<Matrix<T, Dynamic, Dynamic>> svd(A);
    T cond = svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size()-1);
    return cond;
}





class timecounter
{
    struct rusage m_start, m_stop;

public:
    timecounter()
    {}

    void tic()
    {
        getrusage(RUSAGE_SELF, &m_start);
    }

    void toc()
    {
        getrusage(RUSAGE_SELF, &m_stop);
    }

    double get_usertime() const
    {
        double start, stop;
        start = m_start.ru_utime.tv_sec + double(m_start.ru_utime.tv_usec)/1e6;
        stop = m_stop.ru_utime.tv_sec + double(m_stop.ru_utime.tv_usec)/1e6;
        return stop - start;
    }

    double get_systime() const
    {
        double start, stop;
        start = m_start.ru_stime.tv_sec + double(m_start.ru_stime.tv_usec)/1e6;
        stop = m_stop.ru_stime.tv_sec + double(m_stop.ru_stime.tv_usec)/1e6;
        return stop - start;
    }

    double to_double() const
    {
        return /*get_systime() +*/ get_usertime();
    }
};

std::ostream&
operator<<(std::ostream& os, const timecounter& tc)
{
    os << tc.to_double();

    return os;
}


/* Maybe not the best place for the I/O manipulators, but for now they can
 * stay there.
 */

/* COLORS */
std::ostream& red(std::ostream& os) { os << "\x1b[31m"; return os; }
std::ostream& green(std::ostream& os) { os << "\x1b[32m"; return os; }
std::ostream& yellow(std::ostream& os) { os << "\x1b[33m"; return os; }
std::ostream& blue(std::ostream& os) { os << "\x1b[34m"; return os; }
std::ostream& magenta(std::ostream& os) { os << "\x1b[35m"; return os; }
std::ostream& cyan(std::ostream& os) { os << "\x1b[36m"; return os; }
std::ostream& nocolor(std::ostream& os) { os << "\x1b[39m"; return os; }

/* BACKGROUND COLORS */
std::ostream& bgred(std::ostream& os) { os << "\x1b[41m"; return os; }
std::ostream& bggreen(std::ostream& os) { os << "\x1b[42m"; return os; }
std::ostream& bgyellow(std::ostream& os) { os << "\x1b[43m"; return os; }
std::ostream& bgblue(std::ostream& os) { os << "\x1b[44m"; return os; }
std::ostream& bgmagenta(std::ostream& os) { os << "\x1b[45m"; return os; }
std::ostream& bgcyan(std::ostream& os) { os << "\x1b[46m"; return os; }
std::ostream& nobg(std::ostream& os) { os << "\x1b[49m"; return os; }

struct text_tag {};
struct background_tag {};

template<typename tag>
struct terminal_rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;

    terminal_rgb(unsigned char pr, unsigned char pg, unsigned char pb)
        : r(pr), g(pg), b(pb)
    {}
};

using text_rgb = terminal_rgb<text_tag>;
using bg_rgb = terminal_rgb<background_tag>;

std::ostream& operator<<(std::ostream& os, const text_rgb& rgb)
{
    os << "\x1b[38;2;";
    os << int(rgb.r) << ";" << int(rgb.g) << ";" << int(rgb.b) << "m"; 
    return os;
}

std::ostream& operator<<(std::ostream& os, const bg_rgb& rgb)
{
    os << "\x1b[48;2;";
    os << int(rgb.r) << ";" << int(rgb.g) << ";" << int(rgb.b) << "m"; 
    return os;
}

/* BOLD (nobold widely unsupported!) */
std::ostream& bold(std::ostream& os) { os << "\x1b[1m"; return os; }
std::ostream& nobold(std::ostream& os) { os << "\x1b[21m"; return os; }

/* UNDERLINE */
std::ostream& underline(std::ostream& os) { os << "\x1b[4m"; return os; }
std::ostream& nounderline(std::ostream& os) { os << "\x1b[24m"; return os; }

/* BLINK */
std::ostream& blink(std::ostream& os) { os << "\x1b[5m"; return os; }
std::ostream& noblink(std::ostream& os) { os << "\x1b[25m"; return os; }

/* RESET */
std::ostream& reset(std::ostream& os) { os << "\x1b[0m"; return os; }

std::ostream& erase_line(std::ostream& os) { os << "\x1b[0K"; return os; }

/* TIME */
std::ostream& time_now(std::ostream& os)
{
    time_t      rawtime;
    struct tm   *timeinfo;
    char        buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime (buffer,80,"[%D %T] ",timeinfo);

    os << buffer;
    return os;
}




