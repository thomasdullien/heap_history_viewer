import os
from conans import ConanFile, CMake

class HeapHistoryViewer(ConanFile):

    settings = "os", "compiler", "build_type", "arch"
    platform_qt = os.getenv("CMAKE_PREFIX_PATH")
    generators = "cmake", "qt" if not platform_qt else "cmake"

    def requirements(self):
        platform_qt = os.getenv("CMAKE_PREFIX_PATH")
        if not platform_qt:
            self.output.info("CMAKE_PREFIX_PATH not set")
            self.output.info("To use the Qt from your system, set the CMAKE_PREFIX_PATH env var")
            self.output.info("Trying to get Qt from Conan")
            self.output.info("If it fails try: 'conan remote add bincrafters \"https://api.bintray.com/conan/bincrafters/public-conan\"'")
            self.requires("qt/5.12.6@bincrafters/stable")
        else:
            self.output.info("Getting Qt from the system. CMAKE_PREFIX_PATH = " + platform_qt)
        self.requires("gflags/2.2.2")

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
