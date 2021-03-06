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


/**
   values._values

   Native implementation of a flexible values object

   author: Christopher O'Brien <obriencj@gmail.com>
   license: LGPL v.3
 */


#include <py3-values.h>


#define DOCSTR "Native Sibilant core types and functions"


#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif


#if (defined(__GNUC__) &&					\
     (__GNUC__ > 2 || (__GNUC__ == 2 && (__GNUC_MINOR__ > 95))))
  #define likely(x)   __builtin_expect(!!(x), 1)
  #define unlikely(x) __builtin_expect(!!(x), 0)
#else
  #define likely(x)   (x)
  #define unlikely(x) (x)
#endif


#if 1
#define DEBUGMSG(msg, obj) {                                    \
    printf("** " msg " ");                                      \
    (obj) && PyObject_Print(((PyObject *) (obj)), stdout, 0);   \
    printf("\n");                                               \
  }
#else
#define DEBUGMSG(msg, obj) {}
#endif


/* === util === */

static PyObject *_str_close_paren = NULL;
static PyObject *_str_comma_space = NULL;
static PyObject *_str_empty = NULL;
static PyObject *_str_equals = NULL;
static PyObject *_str_esc_quote = NULL;
static PyObject *_str_quote = NULL;
static PyObject *_str_values_paren = NULL;


static PyObject *quoted(PyObject *u) {
  PyObject *tmp, *result;

  tmp = PyUnicode_Replace(u, _str_quote, _str_esc_quote, -1);
  result = PyUnicode_FromFormat("\"%U\"", tmp);
  Py_DECREF(tmp);

  return result;
}


/* === ValuesType === */


static PyObject *values_new(PyTypeObject *type,
			    PyObject *args, PyObject *kwds) {

  return sib_values(args, kwds);
}


static void values_dealloc(PyObject *self) {
  PyValues *s = (PyValues *) self;

  if (s->weakrefs != NULL)
    PyObject_ClearWeakRefs(self);

  Py_XDECREF(s->args);
  Py_XDECREF(s->kwds);

  Py_TYPE(self)->tp_free(self);
}


static int values_traverse(PyObject *self, visitproc visit, void *arg) {
  PyValues *s = (PyValues *) self;
  Py_VISIT(s->args);
  if (s->kwds)
    Py_VISIT(s->kwds);
  return 0;
}


static int values_clear(PyObject *self) {
  PyValues *s = (PyValues *) self;
  Py_CLEAR(s->args);
  if (s->kwds)
    Py_CLEAR(s->kwds);
  return 0;
}


static PyObject *values_iter(PyObject *self) {
  PyValues *s = (PyValues *) self;
  return PyObject_GetIter(s->args);
}


static PyObject *values_args_getitem(PyObject *self, Py_ssize_t index) {
  PyValues *s = (PyValues *) self;
  return PySequence_GetItem(s->args, index);
}


static Py_ssize_t values_kwds_length(PyObject *self) {
  PyValues *s = (PyValues *) self;

  if (s->kwds) {
    return PyDict_Size(s->kwds);
  } else {
    return 0;
  }
}


static PyObject *values_kwds_getitem(PyObject *self, PyObject *key) {
  PyValues *s = (PyValues *) self;

  if (PyLong_CheckExact(key)) {
    return PySequence_GetItem(s->args, PyLong_AsSsize_t(key));

  } else {
    PyObject *result = NULL;

    if (s->kwds) {
      result = PyDict_GetItem(s->kwds, key);
    }

    if (result) {
      Py_INCREF(result);

    } else {
      // we do our own error handling because result could be NULL
      // either because there was a NULL keyword dict, or because of
      // an actual NULL result from GetItem. in either of those cases,
      // we want to emit the same KeyError
      PyErr_SetObject(PyExc_KeyError, quoted(key));
    }

    return result;
  }
}


static PyObject *values_call(PyObject *self,
			     PyObject *args, PyObject *kwds) {

  PyValues *s = (PyValues *) self;
  PyObject *call_args, *call_kwds;
  PyObject *work = NULL, *tmp = NULL;

  if (unlikely(! PyTuple_GET_SIZE(args))) {
    PyErr_SetString(PyExc_TypeError, "values objects must be called with at"
		    " least one argument, the function to apply");
    return NULL;
  }

  work = PyTuple_GET_ITEM(args, 0);

  if (PyTuple_GET_SIZE(args) > 1) {
    // if we have more positionals beyond just the callable work item,
    // we'll need to add those the invocation of work

    tmp = PySequence_GetSlice(args, 1, PyTuple_GET_SIZE(args));

    if (PyTuple_GET_SIZE(s->args)) {
      // merge the existing positionals with the invocation ones
      call_args = PySequence_Concat(s->args, tmp);
      Py_DECREF(tmp);

    } else {
      // there were no positionals in the values, so just use the
      // invocation ones
      call_args = tmp;
    }

  } else {
    // no additional positionals given at invocation, so we'll just be
    // using our existing ones.
    call_args = s->args;
    Py_INCREF(call_args);
  }

  if (kwds && PyDict_Size(kwds)) {
    // if keyword arguments were supplied, we'll need to add those to
    // the work invocation.

    if (s->kwds && PyDict_Size(s->kwds)) {
      // if the values already had keywords, we'll need to create a
      // new dict and merge these two sets of keyword arguments
      // together

      call_kwds = PyDict_Copy(s->kwds);
      PyDict_Update(call_kwds, kwds);

    } else {
      // the values had no keywords, so let's just use the ones supplied
      // by the invocation

      call_kwds = kwds;
      Py_INCREF(call_kwds);
    }

  } else {
    // no extra keyword arguments were supplied, so we only need to
    // use the ones from the values
    call_kwds = s->kwds;
    Py_XINCREF(call_kwds);
  }

  tmp = PyObject_Call(work, call_args, call_kwds);
  Py_DECREF(call_args);
  Py_XDECREF(call_kwds);

  return tmp;
}


static PyObject *values_repr(PyObject *self) {
  PyValues *s = (PyValues *) self;
  PyObject *col = PyList_New(0);
  PyObject *tmp = NULL;
  Py_ssize_t count = 0, limit = 0;
  PyObject *key = NULL, *value = NULL;

  // "values()"
  // "values(1, 2, 3)"
  // "values(foo=4, bar=5)"
  // "values(1, 2, 3, foo=4, bar=5)"

  PyList_Append(col, _str_values_paren);

  limit = PyTuple_GET_SIZE(s->args);
  for (count = 0; count < limit; count++) {
    tmp = PyObject_Repr(PyTuple_GET_ITEM(s->args, count));
    PyList_Append(col, tmp);
    PyList_Append(col, _str_comma_space);
    Py_DECREF(tmp);
  }

  count = 0;
  if (s->kwds && PyDict_Size(s->kwds)) {
    while (PyDict_Next(s->kwds, &count, &key, &value)) {
      tmp = PyObject_Repr(value);
      PyList_Append(col, key);
      PyList_Append(col, _str_equals);
      PyList_Append(col, tmp);
      PyList_Append(col, _str_comma_space);
      Py_DECREF(tmp);
    }
  }

  if (limit || count) {
    // we'll have a trailing _str_comma_space if we added
    // anything. Re-use its index for the close paren
    Py_INCREF(_str_close_paren);
    PyList_SetItem(col, PyList_GET_SIZE(col) - 1, _str_close_paren);
  } else {
    // otherwise, just close off as "values()"
    PyList_Append(col, _str_close_paren);
  }

  tmp = PyUnicode_Join(_str_empty, col);
  Py_DECREF(col);

  return tmp;
}


static Py_hash_t values_hash(PyObject *self) {
  PyValues *s = (PyValues *) self;
  Py_uhash_t result = s->hashed, khash;
  PyObject *tmp, *frozen;

  if (result == 0) {
    result = PyObject_Hash(s->args);
    if (result == (Py_uhash_t) -1)
      return -1;

    if (s->kwds && PyDict_Size(s->kwds)) {
      tmp = _PyDictView_New(s->kwds, &PyDictItems_Type);
      frozen = PyFrozenSet_New(tmp);
      Py_DECREF(tmp);

      if (! frozen)
	return -1;

      khash = PyObject_Hash(frozen);
      Py_DECREF(frozen);

      if (khash == (Py_uhash_t) -1)
	return -1;

      // I stole these magic numbers from tuplehash
      result = (result ^ khash) * _PyHASH_MULTIPLIER;
      result += 97531UL;

      if (result == (Py_uhash_t) -1)
	result = -2;
    }

    s->hashed = result;
  }

  return result;
}


static long values_eq(PyObject *self, PyObject *other) {
  PyValues *s = (PyValues *) self;
  long answer = 0;

  if (self == other) {
    // identity is equality, yes
    answer = 1;

  } else if (PyValues_CheckExact(other)) {
    PyValues *o = (PyValues *) other;

    // when comparing two values against each other, we'll just
    // compare their positionals and keywords. We'll actually do the
    // keywords check first, because it has a quick NULL-check

    if (s->kwds && o->kwds) {
      answer = PyObject_RichCompareBool(s->kwds, o->kwds, Py_EQ);
    } else {
      answer = (s->kwds == o->kwds);
    }

    answer = answer && \
      PyObject_RichCompareBool(s->args, o->args, Py_EQ);

  } else if (PyTuple_CheckExact(other)) {
    // comparing against a tuple is fine, so long as keywords either
    // are NULL or empty.

    answer = ((! s->kwds) || (! PyDict_Size(s->kwds))) &&	\
      PyObject_RichCompareBool(s->args, other, Py_EQ);

  } else if (PyDict_CheckExact(other)) {
    // comparing against a dict is fine, so long as positionals is
    // empty.

    if (s->kwds) {
      answer = (! PyTuple_GET_SIZE(s->args)) &&			\
	PyObject_RichCompareBool(s->kwds, other, Py_EQ);

    } else {
      // we'll say a NULL keywords is equal to an empty dict
      answer = (! PyTuple_GET_SIZE(s->args)) && \
	(! PyDict_Size(other));
    }
  }

  return answer;
}


static PyObject *values_richcomp(PyObject *self, PyObject *other, int op) {
  if (op == Py_EQ) {
    return PyBool_FromLong(values_eq(self, other));

  } else if (op == Py_NE) {
    return PyBool_FromLong(! values_eq(self, other));

  } else {
    PyErr_SetString(PyExc_TypeError, "unsupported values comparison");
    return NULL;
  }
}


static int values_bool(PyObject *self) {
  PyValues *s = (PyValues *) self;

  return !!((s->kwds && PyDict_Size(s->kwds)) || PyTuple_GET_SIZE(s->args));
}


static PyObject *values_add(PyObject *left, PyObject *right) {
  PyValues *result = NULL;
  PyObject *args = NULL, *kwds = NULL, *tmp;

  if (PyValues_CheckExact(left)) {
    PyValues *s = (PyValues *) left;

    if (PyValues_CheckExact(right)) {
      PyValues *o = (PyValues *) right;

      args = PySequence_Concat(s->args, o->args);
      if (! args)
	return NULL;

      if (o->kwds) {
	kwds = s->kwds? PyDict_Copy(s->kwds): PyDict_New();
	PyDict_Update(kwds, o->kwds);
      } else {
	kwds = s->kwds;
	Py_XINCREF(kwds);
      }

    } else if (PyDict_Check(right)) {
      args = s->args;
      Py_INCREF(args);

      kwds = s->kwds? PyDict_Copy(s->kwds): PyDict_New();
      PyDict_Update(kwds, right);

    } else {
      tmp = PySequence_Tuple(right);
      if (! tmp)
	return NULL;

      args = PySequence_Concat(s->args, tmp);
      Py_DECREF(tmp);

      if (! args)
	return NULL;

      kwds = s->kwds;
      Py_XINCREF(kwds);
    }

  } else if (PyValues_CheckExact(right)) {
    PyValues *s = (PyValues *) right;

    if(PyDict_Check(left)) {
      args = s->args;
      Py_INCREF(args);

      if (s->kwds) {
	kwds = PyDict_Copy(left);
	PyDict_Update(kwds, s->kwds);

      } else {
	kwds = PyDict_Copy(left);
      }

    } else {
      tmp = PySequence_Tuple(left);
      if (! tmp)
	return NULL;

      args = PySequence_Concat(tmp, s->args);
      Py_DECREF(tmp);

      if (! args)
	return NULL;

      kwds = s->kwds;
      Py_XINCREF(kwds);
    }

  } else {
    PyErr_SetString(PyExc_TypeError, "values_add invoked with no values");
    return NULL;
  }

  result = (PyValues *) sib_values(args, NULL);
  if (result)
    result->kwds = kwds;  // just to avoid another copy
  Py_DECREF(args);

  return (PyObject *) result;
}


static PyObject *values_keys(PyObject *self, PyObject *_noargs) {
  PyValues *s = (PyValues *) self;
  PyObject *result = NULL, *tmp;

  if (s->kwds) {
    // this is what the default keys() impl on dict does. The
    // PyDict_Keys API creates a list, which we don't want to do.
    result = _PyDictView_New(s->kwds, &PyDictKeys_Type);

  } else {
    // a cheap empty iterator
    tmp = PyTuple_New(0);
    result = PyObject_GetIter(tmp);
    Py_DECREF(tmp);
  }

  return result;
}


static PyMethodDef values_methods[] = {
  { "keys", (PyCFunction) values_keys, METH_NOARGS,
    "V.keys()" },

  { NULL, NULL, 0, NULL },
};


static PyNumberMethods values_as_number = {
  .nb_bool = (inquiry) values_bool,
  .nb_add = values_add,
};


static PySequenceMethods values_as_sequence = {
  .sq_item = values_args_getitem,
};


static PyMappingMethods values_as_mapping = {
  .mp_length = values_kwds_length,
  .mp_subscript = values_kwds_getitem,
};


PyTypeObject PyValuesType = {
  PyVarObject_HEAD_INIT(NULL, 0)

  "values",
  sizeof(PyValues),
  0,

  .tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC,
  .tp_methods = values_methods,
  .tp_new = values_new,
  .tp_dealloc = values_dealloc,
  .tp_weaklistoffset = offsetof(PyValues, weakrefs),
  .tp_traverse = values_traverse,
  .tp_clear = values_clear,

  .tp_iter = values_iter,
  .tp_hash = values_hash,
  .tp_as_number = &values_as_number,
  .tp_as_sequence = &values_as_sequence,
  .tp_as_mapping = &values_as_mapping,

  .tp_call = values_call,
  .tp_repr = values_repr,
  .tp_richcompare = values_richcomp,
};


PyObject *sib_values(PyObject *args, PyObject *kwds) {
  PyValues *self = NULL;

  if (! args) {
    PyErr_SetString(PyExc_TypeError, "values require arguments");
    return NULL;
  }

  self = PyObject_GC_New(PyValues, &PyValuesType);
  if (unlikely(! self))
    return NULL;

  Py_INCREF(args);
  self->args = args;
  self->kwds = kwds? PyDict_Copy(kwds): NULL;
  self->weakrefs = NULL;
  self->hashed = 0;

  PyObject_GC_Track((PyObject *) self);
  return (PyObject *) self;
}


static struct PyModuleDef cvalues = {
  .m_base = PyModuleDef_HEAD_INIT,
  .m_name = "values._values",
  .m_doc = DOCSTR,
  .m_size = -1,
  .m_methods = NULL,
  .m_slots = NULL,
  .m_traverse = NULL,
  .m_clear = NULL,
  .m_free = NULL,
};


#define STR_CONST(which, val) {			\
    if (! (which))				\
      which = PyUnicode_FromString(val);	\
  }


PyMODINIT_FUNC PyInit__values(void) {

  PyObject *mod, *dict;

  if (PyType_Ready(&PyValuesType) < 0)
    return NULL;

  STR_CONST(_str_close_paren, ")");
  STR_CONST(_str_comma_space, ", ");
  STR_CONST(_str_empty, "");
  STR_CONST(_str_esc_quote, "\\\"");
  STR_CONST(_str_equals, "=");
  STR_CONST(_str_quote, "\"");
  STR_CONST(_str_values_paren, "values(");

  mod = PyModule_Create(&cvalues);
  if (! mod)
    return NULL;

  dict = PyModule_GetDict(mod);
  PyDict_SetItemString(dict, "cvalues", (PyObject *) &PyValuesType);

  return mod;
}


/* The end. */
