import os
import re
import sys
import platform
import subprocess

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]
        blocksci_dir = os.environ.get('BLOCKSCI_DIR')
        if not blocksci_dir:
            repo_root = os.path.abspath(os.path.join(ext.sourcedir, os.pardir))
            candidate = os.path.join(repo_root, 'release', 'src')
            if os.path.exists(os.path.join(candidate, 'blocksci-config.cmake')) or os.path.exists(os.path.join(candidate, 'blocksciConfig.cmake')):
                blocksci_dir = candidate
        if blocksci_dir:
            cmake_args.append('-Dblocksci_DIR=' + blocksci_dir)
        pybind11_found = False
        try:
            import pybind11  # noqa: F401
            pybind11_dir = subprocess.check_output(
                [sys.executable, '-m', 'pybind11', '--cmakedir'],
                text=True
            ).strip()
            if pybind11_dir:
                cmake_args.append('-Dpybind11_DIR=' + pybind11_dir)
                pybind11_found = True
        except Exception:
            pybind11_found = False

        if not pybind11_found and sys.version_info >= (3, 11):
            raise RuntimeError(
                "Python 3.11+ requires external pybind11>=2.12. "
                "Install it in the build environment (for example: "
                "`python -m pip install pybind11>=2.12`)."
            )

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j4']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

setup(
    name='blocksci',
    version='0.7.0',
    author='Harry Kalodner',
    author_email='blocksci@cs.princeton.edu',
    description='BlockSci: A high-performance tool for blockchain science and exploration',
    long_description='',
    ext_modules=[CMakeExtension('blocksci/blocksci')],
    packages = find_packages(),
    include_package_data = True,
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
    install_requires=[
        'multiprocess>=0.70.5',
        'psutil>=5.4.2',
        'pycryptodome>=3.0.0',
        'pandas>=0.22.0',
        'dateparser>=0.6.0',
        'requests>=2.19.1'
    ]
)
