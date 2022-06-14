require "mkmf-rice"

$CXXFLAGS += " -std=c++17 $(optflags) -D_USE_XOSHIRO -DSUPPORTS_RESTRICT=1 -D_USE_ROBIN_MAP -DDONT_THROW_ON_INTERRUPT"

apple_clang = RbConfig::CONFIG["CC_VERSION_MESSAGE"] =~ /apple clang/i

# check omp first
if have_library("omp") || have_library("gomp")
  $CXXFLAGS += " -Xclang" if apple_clang
  $CXXFLAGS += " -fopenmp"
end

ext = File.expand_path(".", __dir__)
isotree_src = File.expand_path("../../vendor/isotree/src", __dir__)
isotree_inc = File.expand_path("../../vendor/isotree/include", __dir__)

exclude = %w(c_interface.cpp Rwrapper.cpp RcppExports.cpp)
$srcs = Dir["{#{ext},#{isotree_src}}/*.{cc,cpp}"].reject { |f| exclude.include?(File.basename(f)) }
$INCFLAGS << " -I#{isotree_inc}"
$VPATH << isotree_src

create_makefile("isotree/ext")
