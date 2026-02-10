Experimental Fork
========

This is an experimental fork of opendnp3. The upstream project has reached end-of-life as of September 1st, 2022. This fork is not officially supported and is intended for experimentation and security research only.

**For commercial or production use**, please consider [Step Function I/O's DNP3 library](https://stepfunc.io/products/libraries/dnp3/) — a modern, commercially supported Rust implementation of the DNP3 protocol with official C, C++, Java, and .NET bindings.

Overview
========

Opendnp3 is a portable, scalable, and rigorously tested implementation
of the [DNP3](https//www.dnp.org) protocol stack written in C++11. The library
is designed for high-performance applications like many concurrent TCP
sessions or huge device simulations. It also embeds with a small footprint on Linux.

Build status
============

| Branch       | Build | Code coverage | Quality |
| ------------ | ----- | ------------- | ------- |
| release-2.x  | [![CI 2.x](https://github.com/dnp3/opendnp3/workflows/CI/badge.svg?branch=release-2.x)](https://github.com/dnp3/opendnp3/actions?query=branch%3Arelease-2.x) | [![Codecov](https://codecov.io/gh/dnp3/opendnp3/branch/release-2.x/graph/badge.svg)](https://codecov.io/gh/dnp3/opendnp3/branch/release-2.x) | - |
| develop      | [![CI 2.x](https://github.com/dnp3/opendnp3/workflows/CI/badge.svg?branch=develop)](https://github.com/dnp3/opendnp3/actions?query=branch%3Adevelop) | [![Codecov](https://codecov.io/gh/dnp3/opendnp3/branch/develop/graph/badge.svg)](https://codecov.io/gh/dnp3/opendnp3/branch/develop) | [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/dnp3/opendnp3.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/dnp3/opendnp3/context:cpp) |

Documentation
=============

The documentation can be found on the [project homepage](http://dnp3.github.io/#documentation).

If you want to help contribute to the official guide its in [this repo](https://github.com/dnp3/opendnp3-guide).

License
=============

Licensed under the terms of the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0.html).

Copyright (c) 2010, 2011 Green Energy Corp

Copyright (c) 2013 - 2020 Step Function I/O LLC

Copyright (c) 2020 - 2022 Step Function I/O LLC

Copyright (c) 2010 - 2022 various contributors
