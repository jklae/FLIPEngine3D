# MIT License
# 
# Copyright (C) 2020 Ryan L. Guy
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from ctypes import c_void_p, c_char_p, c_int, c_float, c_double, byref

from .pyfluid import pyfluid as lib
from . import pybindings as pb
from . import method_decorators as decorators
from .trianglemesh import TriangleMesh_t

class MeshObject():
    
    def __init__(self, i, j, k, dx):
        libfunc = lib.MeshObject_new
        args = [c_int, c_int, c_int, c_double, c_void_p]
        pb.init_lib_func(libfunc, args, c_void_p)
        self._obj = pb.execute_lib_func(libfunc, [i, j, k, dx])
        
    def __del__(self):
        try:
            libfunc = lib.MeshObject_destroy
            pb.init_lib_func(libfunc, [c_void_p], None)
            libfunc(self._obj)
        except:
            pass

    def __call__(self):
        return self._obj

    def update_mesh_static(self, mesh):
        mesh_struct = mesh.to_struct()
        libfunc = lib.MeshObject_update_mesh_static
        args = [c_void_p, TriangleMesh_t, c_void_p]
        pb.init_lib_func(libfunc, args, c_void_p)
        pb.execute_lib_func(libfunc, [self(), mesh_struct])

    def update_mesh_animated(self, mesh_previous, mesh_current, mesh_next):
        mesh_struct_previous = mesh_previous.to_struct()
        mesh_struct_current = mesh_current.to_struct()
        mesh_struct_next = mesh_next.to_struct()
        libfunc = lib.MeshObject_update_mesh_animated
        args = [c_void_p, TriangleMesh_t, TriangleMesh_t, TriangleMesh_t, c_void_p]
        pb.init_lib_func(libfunc, args, c_void_p)
        pb.execute_lib_func(libfunc, [self(), mesh_struct_previous, 
                                              mesh_struct_current, 
                                              mesh_struct_next])

    @property
    def enable(self):
        libfunc = lib.MeshObject_is_enabled
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_int)
        return bool(pb.execute_lib_func(libfunc, [self()]))

    @enable.setter
    def enable(self, boolval):
        if boolval:
            libfunc = lib.MeshObject_enable
        else:
            libfunc = lib.MeshObject_disable
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], None)
        pb.execute_lib_func(libfunc, [self()])

    @property
    def inverse(self):
        libfunc = lib.MeshObject_is_inversed
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_int)
        return bool(pb.execute_lib_func(libfunc, [self()]))

    @inverse.setter
    def inverse(self, boolval):
        do_inverse = (boolval and not self.inverse) or (not boolval and self.inverse)
        if do_inverse:
            libfunc = lib.MeshObject_inverse
            pb.init_lib_func(libfunc, [c_void_p, c_void_p], None)
            pb.execute_lib_func(libfunc, [self()])

    @property
    def friction(self):
        libfunc = lib.MeshObject_get_friction
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @friction.setter
    @decorators.check_ge_zero
    @decorators.check_le(1.0)
    def friction(self, value):
        libfunc = lib.MeshObject_set_friction
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])

    @property
    def whitewater_influence(self):
        libfunc = lib.MeshObject_get_whitewater_influence
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @whitewater_influence.setter
    @decorators.check_ge_zero
    def whitewater_influence(self, value):
        libfunc = lib.MeshObject_set_whitewater_influence
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])

    @property
    def dust_emission_strength(self):
        libfunc = lib.MeshObject_get_dust_emission_strength
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @dust_emission_strength.setter
    @decorators.check_ge_zero
    def dust_emission_strength(self, value):
        libfunc = lib.MeshObject_set_dust_emission_strength
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])

    @property
    def sheeting_strength(self):
        libfunc = lib.MeshObject_get_sheeting_strength
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @sheeting_strength.setter
    @decorators.check_ge_zero
    def sheeting_strength(self, value):
        libfunc = lib.MeshObject_set_sheeting_strength
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])

    @property
    def mesh_expansion(self):
        libfunc = lib.MeshObject_get_mesh_expansion
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @mesh_expansion.setter
    def mesh_expansion(self, value):
        libfunc = lib.MeshObject_set_mesh_expansion
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])

    @property
    def enable_append_object_velocity(self):
        libfunc = lib.MeshObject_is_append_object_velocity_enabled
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_int)
        return bool(pb.execute_lib_func(libfunc, [self()]))

    @enable_append_object_velocity.setter
    def enable_append_object_velocity(self, boolval):
        if boolval:
            libfunc = lib.MeshObject_enable_append_object_velocity
        else:
            libfunc = lib.MeshObject_disable_append_object_velocity
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], None)
        pb.execute_lib_func(libfunc, [self()])

    @property
    def object_velocity_influence(self):
        libfunc = lib.MeshObject_get_object_velocity_influence
        pb.init_lib_func(libfunc, [c_void_p, c_void_p], c_float)
        return pb.execute_lib_func(libfunc, [self()])

    @object_velocity_influence.setter
    def object_velocity_influence(self, value):
        libfunc = lib.MeshObject_set_object_velocity_influence
        pb.init_lib_func(libfunc, [c_void_p, c_float, c_void_p], None)
        pb.execute_lib_func(libfunc, [self(), value])