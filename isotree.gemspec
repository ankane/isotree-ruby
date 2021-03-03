require_relative "lib/isotree/version"

Gem::Specification.new do |spec|
  spec.name          = "isotree"
  spec.version       = IsoTree::VERSION
  spec.summary       = "Outlier/anomaly detection for Ruby using Isolation Forest"
  spec.homepage      = "https://github.com/ankane/isotree"
  spec.license       = "BSD-2-Clause"

  spec.author        = "Andrew Kane"
  spec.email         = "andrew@ankane.org"

  spec.files         = Dir["*.{md,txt}", "{ext,lib}/**/*", "vendor/cereal/{LICENSE,README.md}", "vendor/cereal/include/**/*", "vendor/isotree/{LICENSE,README.md}", "vendor/isotree/src/**/*"]
  spec.require_path  = "lib"
  spec.extensions    = ["ext/isotree/extconf.rb"]

  spec.required_ruby_version = ">= 2.5"

  spec.add_dependency "rice", ">= 2.2"
end
