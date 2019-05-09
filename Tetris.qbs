import qbs

Project {
    minimumQbsVersion: "1.7.1"
    
    CppApplication {
        consoleApplication: false
        files: [
            "eventSystem.h",
            "fshader.glsl",
            "header.h",
            "inputSystem.h",
            "main.c",
            "vshader.glsl",
        ]
        
        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
        }
        Depends{
            name: "cpp"
        }
        cpp.includePaths: [".","C:\gnuwin32\mingw\include"]
        cpp.libraryPaths: [".","C:\gnuwin32\mingw\lib"]
        cpp.dynamicLibraries: ["glew32","SDL2main","SDL2","opengl32","mingw32"]
    }
}
