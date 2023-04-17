project('crelay', 'c', version : '0.14.1')

hidapi_dep = dependency('hidapi-libusb', required: get_option('hidapi'))
ftdi_dep = dependency('libftdi1', required: get_option('sainsmart'))
libusb_dep = dependency('libusb-1.0', required: get_option('conrad'))

sources = files('src/relay_drv.c')

if get_option('lib')
  add_project_arguments('-DBUILD_LIB', language: 'c')
else
  sources += files(
    'src/config.c',
    'src/crelay.c',
    'src/relay_drv_gpio.c',
  )
endif

if hidapi_dep.found()
  add_project_arguments('-DDRV_HIDAPI', language: 'c')
  add_project_arguments('-DDRV_SAINSMART16', language: 'c')
  sources += files('src/relay_drv_hidapi.c', 'src/relay_drv_sainsmart16.c',)
endif

if ftdi_dep.found()
  add_project_arguments('-DDRV_SAINSMART', language: 'c')
  sources += files('src/relay_drv_sainsmart.c')
endif

if libusb_dep.found()
  add_project_arguments('-DDRV_CONRAD', language: 'c')
  sources += files('src/relay_drv_conrad.c')
endif

crelay = build_target('crelay',
  sources,
  target_type: get_option('lib') ? 'library' : 'executable',
  dependencies: [ hidapi_dep, ftdi_dep, libusb_dep ],
  install: true,
)

if get_option('lib')
  executable('crelay_cli',
    'lib/app/crelay_cli.c',
    include_directories: 'src',
    link_with: crelay,
  )

  install_headers('src/relay_drv.h')
endif
