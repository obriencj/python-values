/*
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 3 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see
  <http://www.gnu.org/licenses/>.
*/


#ifndef VALUES_TYPE_H
#define VALUES_TYPE_H

#include <Python.h>


typedef struct PyValues {
  PyObject_HEAD

  PyObject *args;
  PyObject *kwds;
  PyObject *weakrefs;
  Py_uhash_t hashed;
} PyValues;

PyTypeObject PyValuesType;


PyObject *sib_values(PyObject *args, PyObject *kwds);


#define PyValues_Check(obj)					\
  ((obj) && PyType_IsSubtype((obj)->ob_type, &PyValuesType))

#define PyValues_CheckExact(obj)			\
  ((obj) && ((obj)->ob_type == &PyValuesType))


#endif


/* The end. */
