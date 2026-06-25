#!/usr/bin/env python3
import shutil
from setuptools import find_packages, setup

with open("README.txt") as f:
    readme = f.read()

shutil.copy("cppcheck-htmlreport", "cppcheck_htmlreport/run.py")

setup(
    name="cppcheck-htmlreport",
    description=(
        "Python script to parse the XML (version 2) output of "
        "cppcheck and generate an HTML report using Pygments for syntax "
        "highlighting."
    ),
    long_description=readme,
    author="Cppcheck Team",
    url="https://github.com/cppcheck-opensource/cppcheck",
    license="GPL",
    packages=find_packages(exclude=("tests", "docs")),
    use_scm_version={"root": ".."},
    entry_points={
        "console_scripts": [
            "cppcheck-htmlreport = cppcheck_htmlreport:run.main",
        ]
    },
    install_requires=["Pygments"],
    # Required by setuptools-scm 7.0 for Python 3.7+
    setup_requires=["setuptools>=60", "setuptools-scm>=7.0"],
)
