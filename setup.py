#! /usr/bin/env python3


# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, see
# <http://www.gnu.org/licenses/>.


"""
python-values

author: Christopher O'Brien <obriencj@gmail.com>
license: LGPL v.3
"""


from setuptools import setup, Extension


TROVE_CLASSIFIERS = (
    "Development Status :: 4 - Beta",
    "Intended Audience :: Developers",
    "License :: OSI Approved"
    " :: GNU Lesser General Public License v3 or later (LGPLv3+)",
    "Operating System :: OS Independent",
    "Programming Language :: Python :: 2.7",
    "Programming Language :: Python :: 3.5",
    "Programming Language :: Python :: 3.6",
    "Programming Language :: Python :: Implementation :: CPython",
    "Topic :: Software Development :: Libraries :: Python Modules",
)


ext_values = Extension(
    name = "values._values",
    sources = ["values/_values.c"],
    include_dirs = ["include"],
    extra_compile_args=["--std=c99"],
)


setup(
    name = "values",
    version = "0.9.0",

    packages = [
        "values",
    ],

    test_suite = "tests",

    ext_modules = [
        ext_values,
    ],

    headers = [
        "include/py3-values.h",
    ],

    zip_safe = True,

    # PyPI information
    author = "Christopher O'Brien",
    author_email = "obriencj@gmail.com",
    url = "https://github.com/obriencj/python-values",
    license = "GNU Lesser General Public License v3",

    description = "Python values type",

    classifiers = TROVE_CLASSIFIERS,
)


#
# The end.
