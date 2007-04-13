/*
 *  linear.h - Linear fragment function class
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   Michael Sloan <mgsloan@gmail.com>
 *
 * Copyright (C) 2006-2007 authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef SEEN_LINEAR_H
#define SEEN_LINEAR_H
#include "isnan.h"
#include "interval.h"

namespace Geom{

class SBasis;

class Hat{
public:
    Hat () {}
    Hat(double d) :d(d) {}
    operator double() const { return d; }
    double d;
};

class Tri{
public:
    Tri () {}
    Tri(double d) :d(d) {}
    operator double() const { return d; }
    double d;
};

class Linear{
public:
    double a[2];
    Linear() {}
    Linear(double aa, double b) {a[0] = aa; a[1] = b;}
    Linear(Hat h, Tri t) {
        a[0] = double(h) - double(t)/2; 
        a[1] = double(h) + double(t)/2;
    }

    Linear(Hat h) {
        a[0] = double(h); 
        a[1] = double(h);
    }

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }

    //Fragment implementation
    inline bool isZero() const { return a[0] == 0 && a[1] == 0; }
    inline bool isFinite() const { return is_finite(a[0]) && is_finite(a[1]); }

    inline double at0() const { return a[0]; }
    inline double at1() const { return a[1]; }

    inline double pointAt(double t) const { 
        return a[0]*(1-t) + a[1]*t;
    }
    inline double operator()(double t) const { return pointAt(t); }

    inline SBasis toSBasis() const;

    inline Interval boundsExact() const { return Interval(a[0], a[1]); }
    inline Interval boundsFast() const { return boundsExact(); }

    operator Tri() const {
        return a[1] - a[0];
    }
    operator Hat() const {
        return (a[1] + a[0])/2;
    }
};

inline Linear reverse(Linear const &a) { return Linear(a[1], a[0]); }

inline Linear operator-(Linear const &a) {
    return Linear(-a[0], -a[1]);
}
inline Linear operator+(Linear const & a, Linear const & b) {
    return Linear(a[0] + b[0], a[1] + b[1]);
}
inline Linear operator-(Linear const & a, Linear const & b) {
    return Linear(a[0] - b[0], a[1] - b[1]);
}
inline Linear& operator+=(Linear & a, Linear const & b) {
    a[0] += b[0]; a[1] += b[1];
    return a;
}
inline Linear& operator-=(Linear & a, Linear const & b) {
    a[0] -= b[0]; a[1] -= b[1];
    return a;
}
inline Linear operator+(Linear const & a, double b) {
    return Linear(a[0] + b, a[1] + b);
}
inline Linear operator-(Linear const & a, double b) {
    return Linear(a[0] - b, a[1] - b);
}
inline Linear& operator+=(Linear & a, double b) {
    a[0] += b; a[1] += b;
    return a;
}
inline Linear& operator-=(Linear & a, double b) {
    a[0] -= b; a[1] -= b;
    return a;
}
inline bool operator==(Linear const & a, Linear const & b) {
    return a[0] == b[0] && a[1] == b[1];
}
inline bool operator!=(Linear const & a, Linear const & b) {
    return a[0] != b[0] || a[1] != b[1];
}
inline Linear operator*(double const a, Linear const & b) {
    return Linear(a*b[0], a*b[1]);
}
inline Linear operator*=(Linear & a, double b) {
    a[0] *= b; a[1] *= b;
    return a;
}

};

#endif //SEEN_LINEAR_H