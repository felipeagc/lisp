project('lisp', 'c')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

sources = [
  'src/main.c',
  'src/tokens.c',
  'src/parser.c',
  'src/env.c',
  'src/eval.c'
]

executable(
  'lisp',
  sources,
  dependencies: m_dep
)
