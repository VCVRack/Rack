#pragma once

#include <assert.h>
#include <map>
#include <vector>
#include "LookupTable.h"

using Spline = std::vector< std::pair<double, double> >;



class NonUniformLookup
{
public:
    void add(double x, double y)
    {
        data[x] = y;
    }
    double lookup(double x)
    {
        //  printf("lookup %f\n", x);
        auto l = data.lower_bound(x);
        assert(l != data.end());
        // printf("lower = %f, %f\n", l->first, l->second);
        auto p = l;
        p--;
        if (p == data.end()) {
            assert(l->first == x);
            return l->second;
        }
        assert(p != data.end());
        // printf("p = %f, %f\n", p->first, p->second);

        // construct line  y = y0 +   (y1 -y0)/(x1 - x0) * x-x0;
        // = b + a(x -b);
        const double b = p->second;
        const double a = (l->second - p->second) / (l->first - p->first);

        const double ret = b + a * (x - p->first);
        // printf("ret = %f\n", ret);

        return ret;
    }
private:
    std::map<double, double> data;
};

class AsymWaveShaper
{
public:
    const static int iNumPoints = 256;
    const static int iSymmetryTables = 16;
private:
    LookupTableParams<float> tables[iSymmetryTables];
public:

    AsymWaveShaper();
  
    float lookup(float x, int index) const
    {
        float x_scaled = 0;
        if (x >= 1) {
            x_scaled = iNumPoints - 1;
        } else if (x < -1) {
            x_scaled = 0;
        } else {
            x_scaled = (x + 1) * iNumPoints / 2;
        }

        assert(index >= 0 && index < iSymmetryTables);
        const LookupTableParams<float>& table = tables[index];
        // TODO: we are going outside of domain!.
        const float y = LookupTable<float>::lookup(table, x_scaled, true);
       // printf("lookup %f -> %f ret %f\n", x, x_scaled, y);
        return y;
    }

    static void genTableValues(const Spline& spline, int numPoints);
    static void genTable(int index, double symmetry);
    static Spline makeSplineRight(double symmetry);
    static Spline makeSplineLeft(double symmetry);
    static std::pair<double, double> calcPoint(const Spline& spline, double t);
};

