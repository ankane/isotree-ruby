require_relative "lib/isotree/version"

Gem::Specification.new do |spec|
  spec.name          = "isotree"
  spec.version       = IsoTree::VERSION
  spec.summary       = "Outlier/anomaly detection for Ruby using Isolation Forest"
  spec.homepage      = "https://github.com/ankane/isotree-ruby"
  spec.license       = "BSD-2-Clause"

  spec.author        = "Andrew Kane"
  spec.email         = "andrew@ankane.org"

  spec.files         = Dir["*.{md,txt}", "{ext,lib}/**/*", "vendor/isotree/{LICENSE,README.md}", "vendor/isotree/inst/COPYRIGHTS", "vendor/isotree/{include,src}/*.{cpp,hpp}", "vendor/isotree/src/robinmap/{LICENSE,README.md}", "vendor/isotree/src/robinmap/include/**/*"]
  spec.require_path  = "lib"
  spec.extensions    = ["ext/isotree/extconf.rb"]

  spec.required_ruby_version = ">= 2.7"

  spec.add_dependency "rice", ">= 4.0.2"
end
