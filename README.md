# Overview of python-values

An immutable sequence and mapping obejct for [Python]

[python]: http://python.org "Python"


## Using values

```python
values(1, 2, 3, foo=4, bar=5)
```


## Requirements

* [Python] 3.5 or later

In addition, the following tools are used in building, testing, or
generating documentation from the project sources.

* [Setuptools]
* [Coverage.py]

These are all available in most linux distributions (eg. [Fedora]), and
for OSX via [MacPorts].

[setuptools]: http://pythonhosted.org/setuptools/

[fedora]: http://fedoraproject.org/

[macports]: http://www.macports.org/


## Building

This module uses [setuptools], so simply run the following to build
the project.

```bash
python setup.py build
```


### Testing

Tests are written as `unittest` test cases. If you'd like to run the
tests, simply invoke:

```bash
python setup.py test
```


## TODO

* Use this values to avoid starting completely from scratch


## Author

Christopher O'Brien <obriencj@gmail.com>

If this project interests you, you can read about more of my hacks and
ideas on [on my blog](http://obriencj.preoccupied.net).


## License

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
