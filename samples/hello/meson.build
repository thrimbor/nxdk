project('hello', 'c')
exe = executable('hello.exe', 'main.c')
custom_target('xbe',
    input: exe,
    output: 'hello.xbe',
    command: ['cxbe', '-OUT:', '@OUTPUT@', '-TITLE:nxdk', '@INPUT@'],
    build_by_default: true)
