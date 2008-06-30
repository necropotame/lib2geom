/*
 * Elliptical Arc - implementation of the svg elliptical arc path element
 *
 * Authors:
 *      MenTaLguY <mental@rydia.net>
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2007-2008  authors
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


#ifndef _2GEOM_SVG_ELLIPTICAL_ARC_H_
#define _2GEOM_SVG_ELLIPTICAL_ARC_H_


#include "curve.h"
#include "angle.h"
#include "utils.h"
#include "bezier-curve.h"
#include "sbasis-curve.h"  // for non-native methods

#include <algorithm>



namespace Geom
{

class SVGEllipticalArc : public Curve
{
  public:
    SVGEllipticalArc(bool _svg_compliant = true)
        : m_initial_point(Point(0,0)), m_final_point(Point(0,0)),
          m_rx(0), m_ry(0), m_rot_angle(0),
          m_large_arc(true), m_sweep(true),
          m_svg_compliant(_svg_compliant)
    {
        m_start_angle = m_end_angle = 0;
        m_center = Point(0,0);
    }

    SVGEllipticalArc( Point _initial_point, double _rx, double _ry,
                      double _rot_angle, bool _large_arc, bool _sweep,
                      Point _final_point,
                      bool _svg_compliant = true
                    )
        : m_initial_point(_initial_point), m_final_point(_final_point),
          m_rx(_rx), m_ry(_ry), m_rot_angle(_rot_angle),
          m_large_arc(_large_arc), m_sweep(_sweep),
          m_svg_compliant(_svg_compliant)
    {
            calculate_center_and_extreme_angles();
    }

    void set( Point _initial_point, double _rx, double _ry,
              double _rot_angle, bool _large_arc, bool _sweep,
              Point _final_point
             )
    {
        m_initial_point = _initial_point;
        m_final_point = _final_point;
        m_rx = _rx;
        m_ry = _ry;
        m_rot_angle = _rot_angle;
        m_large_arc = _large_arc;
        m_sweep = _sweep;
        calculate_center_and_extreme_angles();
    }

    Curve* duplicate() const
    {
        return new SVGEllipticalArc(*this);
    }

    double center(unsigned int i) const
    {
        return m_center[i];
    }

    Point center() const
    {
        return m_center;
    }

    Point initialPoint() const
    {
        return m_initial_point;
    }

    Point finalPoint() const
    {
        return m_final_point;
    }

    double start_angle() const
    {
        return m_start_angle;
    }

    double end_angle() const
    {
        return m_end_angle;
    }

    double ray(unsigned int i) const
    {
        return (i == 0) ? m_rx : m_ry;
    }

    bool large_arc_flag() const
    {
        return m_large_arc;
    }

    bool sweep_flag() const
    {
        return m_sweep;
    }

    double rotation_angle() const
    {
        return m_rot_angle;
    }

    void setInitial( const Point _point)
    {
        m_initial_point = _point;
        calculate_center_and_extreme_angles();
    }

    void setFinal( const Point _point)
    {
        m_final_point = _point;
        calculate_center_and_extreme_angles();
    }

    void setExtremes( const Point& _initial_point, const Point& _final_point )
    {
        m_initial_point = _initial_point;
        m_final_point = _final_point;
        calculate_center_and_extreme_angles();
    }

    bool isDegenerate() const
    {
        return ( are_near(ray(X), 0) || are_near(ray(Y), 0) );
    }

    bool is_svg_compliant() const
    {
        return m_svg_compliant;
    }

    Rect boundsFast() const
    {
        return boundsExact();
    }

    Rect boundsExact() const;

    // TODO: native implementation of the following methods
    Rect boundsLocal(Interval i, unsigned int deg) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().boundsLocal(i, deg);
        else
            return SBasisCurve(toSBasis()).boundsLocal(i, deg);
    }

    std::vector<double> roots(double v, Dim2 d) const;

    std::vector<double>
    allNearestPoints( Point const& p, double from = 0, double to = 1 ) const;

    double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
    {
        if ( are_near(ray(X), ray(Y)) && are_near(center(), p) )
        {
            return from;
        }
        return allNearestPoints(p, from, to).front();
    }

    // TODO: native implementation of the following methods
    int winding(Point p) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().winding(p);
        else
            return SBasisCurve(toSBasis()).winding(p);
    }

    Curve *derivative() const;

    // TODO: native implementation of the following methods
    Curve *transformed(Matrix const &m) const
    {
        return SBasisCurve(toSBasis()).transformed(m);
    }

    std::vector<Point> pointAndDerivatives(Coord t, unsigned int n) const;

    D2<SBasis> toSBasis() const;

    bool containsAngle(Coord angle) const;

    double valueAtAngle(Coord t, Dim2 d) const;

    Point pointAtAngle(Coord t) const
    {
        double sin_rot_angle = std::sin(rotation_angle());
        double cos_rot_angle = std::cos(rotation_angle());
        Matrix m( ray(X) * cos_rot_angle, ray(X) * sin_rot_angle,
                 -ray(Y) * sin_rot_angle, ray(Y) * cos_rot_angle,
                  center(X),              center(Y) );
        Point p( std::cos(t), std::sin(t) );
        return p * m;
    }

    double valueAt(Coord t, Dim2 d) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().valueAt(t, d);

        Coord tt = map_to_02PI(t);
        return valueAtAngle(tt, d);
    }

    Point pointAt(Coord t) const
    {
        if (isDegenerate() && is_svg_compliant())
            return chord().pointAt(t);

        Coord tt = map_to_02PI(t);
        return pointAtAngle(tt);
    }

    std::pair<SVGEllipticalArc, SVGEllipticalArc>
    subdivide(Coord t) const
    {
        SVGEllipticalArc* arc1 = static_cast<SVGEllipticalArc*>(portion(0, t));
        SVGEllipticalArc* arc2 = static_cast<SVGEllipticalArc*>(portion(t, 1));
        assert( arc1 != NULL && arc2 != NULL);
        std::pair<SVGEllipticalArc, SVGEllipticalArc> arc_pair(*arc1, *arc2);
        delete arc1;
        delete arc2;
        return arc_pair;
    }

    Curve* portion(double f, double t) const;

    // the arc is the same but traversed in the opposite direction
    Curve* reverse() const
    {
        SVGEllipticalArc* rarc = new SVGEllipticalArc( *this );
        rarc->m_sweep = !m_sweep;
        rarc->m_initial_point = m_final_point;
        rarc->m_final_point = m_initial_point;
        rarc->m_start_angle = m_end_angle;
        rarc->m_end_angle = m_start_angle;
        return rarc;
    }

    double sweep_angle() const
    {
        Coord d = end_angle() - start_angle();
        if ( !sweep_flag() ) d = -d;
        if ( d < 0 )
            d += 2*M_PI;
        return d;
    }

    LineSegment chord() const
    {
        return LineSegment(initialPoint(), finalPoint());
    }

  private:
    Coord map_to_02PI(Coord t) const;
    Coord map_to_01(Coord angle) const;
    void calculate_center_and_extreme_angles();

  private:
    Point m_initial_point, m_final_point;
    double m_rx, m_ry, m_rot_angle;
    bool m_large_arc, m_sweep;
    double m_start_angle, m_end_angle;
    Point m_center;
    bool m_svg_compliant;

}; // end class SVGEllipticalArc

template< class charT >
inline
std::basic_ostream<charT> &
operator<< (std::basic_ostream<charT> & os, const SVGEllipticalArc & ea)
{
    os << "{ cx: " << ea.center(X) << ", cy: " <<  ea.center(Y)
       << ", rx: " << ea.ray(X) << ", ry: " << ea.ray(Y)
       << ", rot angle: " << decimal_round(rad_to_deg(ea.rotation_angle()),2)
       << ", start angle: " << decimal_round(rad_to_deg(ea.start_angle()),2)
       << ", end angle: " << decimal_round(rad_to_deg(ea.end_angle()),2)
       << " }";

    return os;
}

} // end namespace Geom




#endif /* _2GEOM_SVG_ELLIPTICAL_ARC_H_ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

