Gem::Specification.new do |s|
  s.name = %q{shapelib}
  s.version = "0.6.1"
  s.files = %w{depend extconf.rb Interface.html Interface.rd libtest.rb main.c README.txt sfcode.h sflist.h shapelib.gemspec shpplus.c shpplus.h spcode.h splist.h spwkt.h style.css valconv.h zsample1.rb ztest1.rb lib/shapelib.rb lib/shape_lib/shape.rb}
  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["prasinos@users.sourceforge.net", "Mike Mangino"]
  s.date = %q{2008-02-13}
  s.description = %q{== DESCRIPTION: A ruby wrapper around the shapefile library}
  s.email = %q{mmangino@elevatedrails.com}
  s.has_rdoc = false
  s.require_paths = ["lib"]
  s.extensions << "extconf.rb"
  s.summary = %q{Ruby wrapper around the libshapefile library}

  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 2
  end
end
