from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class PotatoAlert(ConanFile):
    author = "github.com/razaqq"
    homepage = "https://github.com/razaqq/PotatoAlert"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    default_options = {
        "date/*:header_only": True,
        "spdlog/*:no_exceptions": True,
        "spdlog/*:header_only": True,
        "spdlog/*:use_std_fmt": False,
        "qt*:shared": "True",
    }

    def requirements(self):
        self.requires("catch2/3.8.0")
        self.requires("ctre/3.9.0")
        self.requires("date/3.0.3")
        self.requires("fmt/11.1.1", override=True)
        self.requires("kuba-zip/0.3.2")
        self.requires("openssl/3.3.2")
        # self.requires("qt/6.7.1")
        self.requires("rapidjson/cci.20230929")
        self.requires("spdlog/1.15.0")
        self.requires("sqlite3/3.47.2")
        self.requires("tinyxml2/10.0.0")
        self.requires("zlib/1.3.1")

    def layout(self):
        cmake_layout(self)
