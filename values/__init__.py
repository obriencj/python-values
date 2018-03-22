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
values

author: Christopher O'Brien <obriencj@gmail.com>
license: LGPL v.3
"""


__ALL__ = ("values", )


# we'll implement most of these features in pure Python first. Then
# we'll attempt to import the _values extension, and if successful
# we'll just use that instead.


class pyvalues(object):

    def __init__(self, *args, **kwds):
        self.__args = args
        self.__kwds = kwds
        self.__hashed = None


    def __repr__(self):

        members = []
        members.extend(map(repr, self.__args))
        members.extend(map("%s=%r".__mod__, self.__kwds.items()))
        members = ", ".join(members)

        return "values(" + members + ")"


    def __hash__(self):
        result = self.__hashed
        if result is None:
            result = (hash((self.__args, frozenset(self.__kwds.items())))
                      if self.__kwds else hash(self.__args))
            self.__hashed = result
        return result


    def __eq__(self, other):
        if self is other:
            return True

        elif isinstance(other, type(self)):
            return ((self.__args == other.__args) and
                    (self.__kwds == other.__kwds))

        elif isinstance(other, tuple):
            return ((not self.__kwds) and
                    (self.__args == other))

        elif isinstance(other, dict):
            return ((not self.__args) and
                    (self.__kwds == other))

        else:
            return False


    def __ne__(self, other):
        return not self.__eq__(other)


    def __bool__(self):
        return bool(self.__args or self.__kwds)


    def __iter__(self):
        return iter(self.__args)


    def __getitem__(self, key):
        if isinstance(key, (slice, int)):
            return self.__args[key]
        else:
            return self.__kwds[key]


    def keys(self):
        return self.__kwds.keys()


    def __call__(self, function, *args, **kwds):
        if args:
            if self.__args:
                args = self.__args + args
        else:
            args = self.__args

        if kwds:
            if self.__kwds:
                tmp = dict(self.__kwds)
                tmp.update(kwds)
                kwds = tmp
        else:
            kwds = self.__kwds

        return function(*args, **kwds)


try:
    # let's see if we're on a platform that supports extensions
    from ._values import cvalues

except ImportError:
    # nope! that's fine, we have the plain ol' python one ready to go
    values = pyvalues

else:
    # we prefer the native one though
    values = cvalues


#
# The end.
