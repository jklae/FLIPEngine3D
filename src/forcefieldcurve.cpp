/*
MIT License

Copyright (C) 2020 Ryan L. Guy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "forcefieldcurve.h"

// DEBUG
#include <iostream>

#include <random>
#include <algorithm>

#include "levelsetsolver.h"
#include "forcefieldutils.h"
#include "interpolation.h"


ForceFieldCurve::ForceFieldCurve() {
}

ForceFieldCurve::~ForceFieldCurve() {
}

void ForceFieldCurve::update(double dt, double frameInterpolation) {
    MeshObjectStatus s = _meshObject.getStatus();
    bool isMeshStateChanged = s.isStateChanged || (s.isEnabled && s.isAnimated && s.isMeshChanged);
    if (isMeshStateChanged) {
        _isLevelsetUpToDate = false;
    }

    float eps = 1e-6;
    if (_isMaxDistanceEnabled && std::abs(_maxDistance - _lastMaxDistance) > eps) {
        _isLevelsetUpToDate = false;
    }

    if (_isLevelsetUpToDate) {
        return;
    }

    TriangleMesh mesh = _meshObject.getMesh(frameInterpolation);
    _updateGridDimensions(mesh);

    _curveTriangleMesh = _curveVerticesToTriangleMesh(mesh.vertices);
    _curveTriangleMesh.translate(-_offsetSDF);
    ForceFieldUtils::generateSurfaceVectorField(_sdf, _curveTriangleMesh, _vectorField);

    _lastMaxDistance = _isMaxDistanceEnabled ? _maxDistance : -1.0f;
    _isLevelsetUpToDate = true;
}

void ForceFieldCurve::addForceFieldToGrid(MACVelocityField &fieldGrid) {
    int U = 0; int V = 1; int W = 2;
    _addForceFieldToGridMT(fieldGrid, U);
    _addForceFieldToGridMT(fieldGrid, V);
    _addForceFieldToGridMT(fieldGrid, W);
}

void ForceFieldCurve::addGravityScaleToGrid(ForceFieldGravityScaleGrid &scaleGrid) {
    float scaleWidth = _gravityScaleWidth;
    if (_isMaxDistanceEnabled) {
        scaleWidth = std::min(scaleWidth, _maxDistance);
    }

    for (int k = 0; k < _ksizeSDF + 1; k++) {
        for (int j = 0; j < _jsizeSDF + 1; j++) {
            for (int i = 0; i < _isizeSDF + 1; i++) {
                float d = _sdf(i, j, k);
                if (d < scaleWidth) {
                    float factor = 1.0f - (d / scaleWidth);
                    float scale = factor * _gravityScale + (1.0f - factor);
                    scaleGrid.addScale(i + _ioffsetSDF, j + _joffsetSDF, k + _koffsetSDF, scale);
                }
            }
        }
    }
}

std::vector<vmath::vec3> ForceFieldCurve::generateDebugProbes() {
    std::mt19937 generator(0);
    std::uniform_real_distribution<float> uniform(-_jitterFactor * _dx, _jitterFactor * _dx);

    float minradius = _minRadiusFactor * _dx;
    float maxradius = _maxRadiusFactor * _dx;
    vmath::vec3 positionOffset(_ioffsetSDF * _dx, _joffsetSDF * _dx, _koffsetSDF * _dx);
    std::vector<vmath::vec3> candidatePointList;
    for (int k = 0; k < _ksizeSDF; k++) {
        for (int j = 0; j < _jsizeSDF; j++) {
            for (int i = 0; i < _isizeSDF; i++) {
                float d = std::abs(_sdf(i, j, k));
                if (d >= minradius && d < maxradius) {
                    vmath::vec3 jitter(uniform(generator), uniform(generator), uniform(generator));
                    vmath::vec3 p = Grid3d::GridIndexToPosition(i, j, k, _dx) + positionOffset + jitter;
                    candidatePointList.push_back(p);
                }
            }
        }
    }

    std::shuffle(candidatePointList.begin(), candidatePointList.end(), generator);
    int numProbes = std::min(_numDebugProbes, (int)candidatePointList.size());
    std::vector<vmath::vec3> probes(candidatePointList.begin(), candidatePointList.begin() + numProbes);

    return probes;
}

float ForceFieldCurve::getFlowStrength() {
    return _flowStrength;
}

void ForceFieldCurve::setFlowStrength(float s) {
    float eps = 1e-6;
    if (std::abs(s - _flowStrength) > eps) {
        _isSubclassStateChangedValue = true;
    }
    _flowStrength = s;
}

float ForceFieldCurve::getSpinStrength() {
    return _spinStrength;
}

void ForceFieldCurve::setSpinStrength(float s) {
    float eps = 1e-6;
    if (std::abs(s - _spinStrength) > eps) {
        _isSubclassStateChangedValue = true;
    }
    _spinStrength = s;
}

void ForceFieldCurve::enableEndCaps() {
    if (!_enableEndCaps) {
        _isSubclassStateChangedValue = true;
    }
    _enableEndCaps = true;
}

void ForceFieldCurve::disableEndCaps() {
    if (_enableEndCaps) {
        _isSubclassStateChangedValue = true;
    }
    _enableEndCaps = false;
}

bool ForceFieldCurve::isEndCapsEnabled() {
    return _enableEndCaps;
}

void ForceFieldCurve::_initialize() {
}

bool ForceFieldCurve::_isSubclassStateChanged() {
    return _isSubclassStateChangedValue;
}

void ForceFieldCurve::_clearSubclassState() {
    _isSubclassStateChangedValue = false;
}

void ForceFieldCurve::_addForceFieldToGridMT(MACVelocityField &fieldGrid, int dir) {
    int U = 0; int V = 1; int W = 2;

    int gridsize = 0;
    if (dir == U) {
        gridsize = (_isize + 1) * _jsize * _ksize;
    } else if (dir == V) {
        gridsize = _isize * (_jsize + 1) * _ksize;
    } else if (dir == W) {
        gridsize = _isize * _jsize * (_ksize + 1);
    }

    int numCPU = ThreadUtils::getMaxThreadCount();
    int numthreads = (int)fmin(numCPU, gridsize);
    std::vector<std::thread> threads(numthreads);
    std::vector<int> intervals = ThreadUtils::splitRangeIntoIntervals(0, gridsize, numthreads);
    for (int i = 0; i < numthreads; i++) {
        threads[i] = std::thread(&ForceFieldCurve::_addForceFieldToGridThread, this,
                                 intervals[i], intervals[i + 1], &fieldGrid, dir);
    }

    for (int i = 0; i < numthreads; i++) {
        threads[i].join();
    }
}

void ForceFieldCurve::_updateGridDimensions(TriangleMesh &mesh) {
    float eps = 1e-6;
    if (_isMaxDistanceEnabled) {
        AABB bbox(mesh.vertices);
        bbox.expand(eps + 2.0f * _maxDistance);
        vmath::vec3 pmin = bbox.getMinPoint();
        vmath::vec3 pmax = bbox.getMaxPoint();

        GridIndex gmin = Grid3d::positionToGridIndex(pmin, _dx);
        GridIndex gmax = Grid3d::positionToGridIndex(pmax, _dx);

        _ioffsetSDF = std::max(gmin.i, 0);
        _joffsetSDF = std::max(gmin.j, 0);
        _koffsetSDF = std::max(gmin.k, 0);
        _offsetSDF = vmath::vec3(_ioffsetSDF * _dx, _joffsetSDF * _dx, _koffsetSDF * _dx);
        gmax.i = std::min(gmax.i + 1, _isize - 1);
        gmax.j = std::min(gmax.j + 1, _jsize - 1);
        gmax.k = std::min(gmax.k + 1, _ksize - 1);
        _isizeSDF = gmax.i - _ioffsetSDF + 1;
        _jsizeSDF = gmax.j - _joffsetSDF + 1;
        _ksizeSDF = gmax.k - _koffsetSDF + 1;
    } else {
        _ioffsetSDF = 0;
        _joffsetSDF = 0;
        _koffsetSDF = 0;
        _offsetSDF = vmath::vec3(0.0f, 0.0f, 0.0f);
        _isizeSDF = _isize;
        _jsizeSDF = _jsize;
        _ksizeSDF = _ksize;
    }

    int si, sj, sk;
    _sdf.getGridDimensions(&si, &sj, &sk);
    if (si != _isizeSDF || sj != _jsizeSDF || sk != _ksizeSDF) {
        _sdf = MeshLevelSet(_isizeSDF, _jsizeSDF, _ksizeSDF, _dx);
        _sdf.disableVelocityData();
        _sdf.disableSignCalculation();

        _vectorField = Array3d<vmath::vec3>(_isizeSDF + 1, _jsizeSDF + 1, _ksizeSDF + 1);
    } else {
        _sdf.reset();
        _vectorField.fill(vmath::vec3());
    }
}

void ForceFieldCurve::_addForceFieldToGridThread(int startidx, int endidx, 
                                                   MACVelocityField *fieldGrid, int dir) {

    // TODO: Update for curve geometry

    int U = 0; int V = 1; int W = 2;

    float minDistance = -1.0f;
    float maxDistance = std::numeric_limits<float>::infinity();
    if (_isMinDistanceEnabled) {
        minDistance = _minDistance;
    }
    if (_isMaxDistanceEnabled) {
        maxDistance = _maxDistance;
    }

    float eps = 1e-6;

    if (dir == U) {

        for (int idx = startidx; idx < endidx; idx++) {
            GridIndex g = Grid3d::getUnflattenedIndex(idx, _isize + 1, _jsize);
            vmath::vec3 gp = Grid3d::FaceIndexToPositionU(g, _dx);
            if (!Grid3d::isPositionInGrid(gp - _offsetSDF, _dx, _isizeSDF, _jsizeSDF, _ksizeSDF)) {
                continue;
            }

            if (!_enableEndCaps && _isPositionNearEndCap(gp)) {
                continue;
            }

            vmath::vec3 vp = gp - _offsetSDF;
            vmath::vec3 vect = Interpolation::trilinearInterpolate(vp, _dx, _vectorField);

            float dist = vect.length();
            float r = std::max(dist, minDistance);
            if (dist < eps || r < eps || r > maxDistance) {
                continue;
            }

            vmath::vec3 attractionNormal = -vmath::normalize(vect);
            vmath::vec3 attractionForce = _calculateForceVector(r, attractionNormal);

            vmath::vec3 flowNormal = _getFlowDirection(gp);
            vmath::vec3 flowForce = _calculateForceVector(r, _flowStrength, flowNormal);

            vmath::vec3 spinForce;
            if (vmath::length(flowNormal) > eps) {
                vmath::vec3 spinNormal = vmath::cross(flowNormal, attractionNormal);
                spinForce = _calculateForceVector(r, _spinStrength, spinNormal);
            }

            vmath::vec3 force = attractionForce + flowForce + spinForce;
            fieldGrid->addU(g, force.x);
        }

    } else if (dir == V) {

        for (int idx = startidx; idx < endidx; idx++) {
            GridIndex g = Grid3d::getUnflattenedIndex(idx, _isize, _jsize + 1);
            vmath::vec3 gp = Grid3d::FaceIndexToPositionV(g, _dx);
            if (!Grid3d::isPositionInGrid(gp - _offsetSDF, _dx, _isizeSDF, _jsizeSDF, _ksizeSDF)) {
                continue;
            }

            if (!_enableEndCaps && _isPositionNearEndCap(gp)) {
                continue;
            }

            vmath::vec3 vp = gp - _offsetSDF;
            vmath::vec3 vect = Interpolation::trilinearInterpolate(vp, _dx, _vectorField);

            float dist = vect.length();
            float r = std::max(dist, minDistance);
            if (dist < eps || r < eps || r > maxDistance) {
                continue;
            }

            vmath::vec3 attractionNormal = -vmath::normalize(vect);
            vmath::vec3 attractionForce = _calculateForceVector(r, attractionNormal);

            vmath::vec3 flowNormal = _getFlowDirection(gp);
            vmath::vec3 flowForce = _calculateForceVector(r, _flowStrength, flowNormal);

            vmath::vec3 spinForce;
            if (vmath::length(flowNormal) > eps) {
                vmath::vec3 spinNormal = vmath::cross(flowNormal, attractionNormal);
                spinForce = _calculateForceVector(r, _spinStrength, spinNormal);
            }

            vmath::vec3 force = attractionForce + flowForce + spinForce;
            fieldGrid->addV(g, force.y);
        }

    } else if (dir == W) {

        for (int idx = startidx; idx < endidx; idx++) {
            GridIndex g = Grid3d::getUnflattenedIndex(idx, _isize, _jsize);
            vmath::vec3 gp = Grid3d::FaceIndexToPositionW(g, _dx);
            if (!Grid3d::isPositionInGrid(gp - _offsetSDF, _dx, _isizeSDF, _jsizeSDF, _ksizeSDF)) {
                continue;
            }

            if (!_enableEndCaps && _isPositionNearEndCap(gp)) {
                continue;
            }

            vmath::vec3 vp = gp - _offsetSDF;
            vmath::vec3 vect = Interpolation::trilinearInterpolate(vp, _dx, _vectorField);

            float dist = vect.length();
            float r = std::max(dist, minDistance);
            if (dist < eps || r < eps || r > maxDistance) {
                continue;
            }

            vmath::vec3 attractionNormal = -vmath::normalize(vect);
            vmath::vec3 attractionForce = _calculateForceVector(r, attractionNormal);

            vmath::vec3 flowNormal = _getFlowDirection(gp);
            vmath::vec3 flowForce = _calculateForceVector(r, _flowStrength, flowNormal);

            vmath::vec3 spinForce;
            if (vmath::length(flowNormal) > eps) {
                vmath::vec3 spinNormal = vmath::cross(flowNormal, attractionNormal);
                spinForce = _calculateForceVector(r, _spinStrength, spinNormal);
            }

            vmath::vec3 force = attractionForce + flowForce + spinForce;
            fieldGrid->addW(g, force.z);
        }

    }

}

TriangleMesh ForceFieldCurve::_curveVerticesToTriangleMesh(std::vector<vmath::vec3> vertices) {
    // At the moment, signed distance field generation only works on triangles, not vertices or
    // line segments. As a work around, the curve data (sequence of vertices) will be converted to a
    // triangle mesh for processing the signed distance field and force vector field. Each triangle
    // will contain duplicate vertices, making every triangle effectively a line segment.
    //
    // The first two triangle vertex indices will be duplicates. The third index will be the next
    // vertex in the original curve vertex list.

    TriangleMesh tmesh;
    if (vertices.size() < 2) {
        return tmesh;
    }

    tmesh.vertices = vertices;
    for (size_t i = 0; i < vertices.size() - 1; i++) {
        Triangle t;
        t.tri[0] = i;
        t.tri[1] = i;
        t.tri[2] = i + 1;
        tmesh.triangles.push_back(t);
    }

    return tmesh;
}

vmath::vec3 ForceFieldCurve::_getFlowDirection(vmath::vec3 p) {
    p -= _offsetSDF;
    GridIndex g = Grid3d::positionToGridIndex(p, _dx);
    vmath::vec3 gp = Grid3d::GridIndexToPosition(g, _dx);
    vmath::vec3 vect = _vectorField(g);
    vmath::vec3 cp = gp + vect;
    GridIndex cg = Grid3d::positionToGridIndex(cp, _dx);

    int tidx = _sdf.getClosestTriangleIndex(cg);
    if (tidx == -1) {
        return vmath::vec3(0.0f, 0.0f, 0.0f);
    }

    Triangle t = _curveTriangleMesh.triangles[tidx];
    int vidx1 = t.tri[0];
    if (vidx1 > 0 && vidx1 == (int)_curveTriangleMesh.vertices.size() - 1) {
        vidx1 = vidx1 - 1;
    }

    vmath::vec3 v1 = _curveTriangleMesh.vertices[vidx1];
    float eps = 1e-6;
    for (size_t vidx2 = vidx1 + 1; vidx2 < _curveTriangleMesh.vertices.size(); vidx2++) {
        vmath::vec3 v2 = _curveTriangleMesh.vertices[vidx2];
        vmath::vec3 dir = v2 - v1;
        if (vmath::length(dir) < eps) {
            continue;
        }

        return dir.normalize();
    }

    return vmath::vec3(0.0f, 0.0f, 0.0f);
}

bool ForceFieldCurve::_isPositionNearEndCap(vmath::vec3 p) {
    p -= _offsetSDF;
    GridIndex g = Grid3d::positionToGridIndex(p, _dx);
    vmath::vec3 gp = Grid3d::GridIndexToPosition(g, _dx);
    vmath::vec3 vect = _vectorField(g);
    vmath::vec3 cp = gp + vect;
    GridIndex cg = Grid3d::positionToGridIndex(cp, _dx);

    int tidx = _sdf.getClosestTriangleIndex(cg);
    if (tidx == -1) {
        return false;
    }

    if (tidx == 0 || tidx == (int)_curveTriangleMesh.triangles.size() - 1) {
        return true;
    }

    return false;
}