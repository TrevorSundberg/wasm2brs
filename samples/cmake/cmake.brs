Function Start()
    ' The Init function is generated by wasm2brs and sets up the
    ' memory object as well as the imports, exports, data-sections, etc.
    w2bInit__()

    ' WASI is the WebAssembly System Interface, or how WebAssembly talks
    ' to the operating system, or in this case Roku. Here you can pass
    ' environment variables, command line arguments, etc. The m.w2b_memory
    ' is the name of the roByteArray created by w2bInit__.
    wasi_init(m.w2b_memory, "cmake.wasm", {})

    ' Most compilers such as clang generate a wrapper around main() called
    ' _start(), which is responsible for all pre-main initialialization, and
    ' it eventually calls main itself.
    w2b__start()

    ' Shutdown WASI, which in our case flushes the output, etc.
    wasi_shutdown()
End Function

Function GetSettings()
    Return { PauseOnExit: True }
End Function
