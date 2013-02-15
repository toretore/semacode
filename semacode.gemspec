# require 'rubygems'
# require 'rake/gempackagetask'

Gem::Specification.new do |s| 
  s.name = "semacode"
  s.version = "0.7.5"
  s.author = "Guido Sohne"
  s.email = "guido@sohne.net"
  s.homepage = "http://sohne.net/projects/semafox/"
  s.platform = Gem::Platform::RUBY
  s.summary = "Create semacodes (2D barcodes) using Ruby."
  s.rubyforge_project = "semacode"
  s.description = <<DESC
  This Ruby extension implements a DataMatrix encoder for Ruby. It is typically
  used to create semacodes, which are barcodes, that contain URLs. This encoder
  does not create image files or visual representations of the semacode. This is
  because it can be used for more than creating images, such as rendering
  semacodes to HTML, SVG, PDF or even stored in a database or file for later
  use.
DESC

  s.extensions << 'ext/extconf.rb'
  s.add_dependency('rake', '>= 0.7.0')
  s.files = Dir[
    "{lib,ext}/**/*.rb", 
    "ext/**/*.c", 
    "ext/**/*.h",
    "tests/**/*.rb",
    "README",
    "CHANGELOG",
    "Rakefile"]
  s.require_path = "lib"
  s.autorequire = "semacode"
  s.test_files = Dir["{tests}/**/*test.rb"]
  s.has_rdoc = true
  s.extra_rdoc_files = ["README"]
end
