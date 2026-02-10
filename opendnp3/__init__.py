"""
opendnp3 - Python bindings for the opendnp3 (IEEE-1815 DNP3) protocol library.

This package provides Python bindings for the opendnp3 C++ library, the de facto
reference implementation of the DNP3 protocol used in SCADA systems for electric
utility, water, and wastewater industries.

Usage:
    import opendnp3

    manager = opendnp3.DNP3Manager(1)
    # ... create channels, masters, outstations
    manager.Shutdown()
"""
import os
import sys

_package_dir = os.path.dirname(os.path.abspath(__file__))

# Load the compiled pybind11 extension module (_opendnp3).
# The .so/.pyd file is placed in the package directory by the build system.
# We import it and re-export all its symbols at the package level.
try:
    from . import _opendnp3 as _native
except ImportError as e:
    raise ImportError(
        f"Failed to import opendnp3 native extension (_opendnp3). "
        f"Ensure the C++ extension is built and installed. "
        f"Package dir: {_package_dir}. Error: {e}"
    ) from e

# Re-export all public symbols from the native module
_public_attrs = [name for name in dir(_native) if not name.startswith('_')]
for _attr in _public_attrs:
    globals()[_attr] = getattr(_native, _attr)

# Also export submodules (flags, levels)
if hasattr(_native, 'flags'):
    flags = _native.flags
if hasattr(_native, 'levels'):
    levels = _native.levels

__all__ = _public_attrs
