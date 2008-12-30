#!/usr/bin/ruby
require 'mkmf'

dir_config('shapelib')
have_header('shapefil.h') or raise 'shapefil.h not found'
have_library('shp') or raise 'libshp not found'
create_makefile('shapelib')
