import os
import sys
from setuptools import setup, find_packages
from setuptools.dist import Distribution

# Get version from environment or default
version = os.environ.get('PACKAGE_VERSION', '3.1.2.1')

# Read README
readme_path = os.path.join(os.path.dirname(__file__), "README.md")
long_description = ""
if os.path.exists(readme_path):
    with open(readme_path, "r", encoding="utf-8") as fh:
        long_description = fh.read()


class BinaryDistribution(Distribution):
    """Mark this as a binary distribution."""
    def has_ext_modules(self):
        return True


setup(
    name="opendnp3",
    version=version,
    description="Python bindings for opendnp3 (IEEE-1815 DNP3 protocol)",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="f0rw4rd",
    url="https://github.com/f0rw4rd/opendnp3",
    packages=find_packages(),
    package_data={
        "opendnp3": ["*.so", "*.pyd", "*.dll"],
    },
    include_package_data=True,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: System :: Networking",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: Python :: Implementation :: CPython",
    ],
    python_requires=">=3.9",
    keywords="dnp3 ieee-1815 scada protocol power-systems ics industrial-control outstation master",
    project_urls={
        "Bug Reports": "https://github.com/f0rw4rd/opendnp3/issues",
        "Source": "https://github.com/f0rw4rd/opendnp3",
    },
    license="Apache-2.0",
    data_files=[("", ["LICENSE", "NOTICE"])],
    distclass=BinaryDistribution,
)
