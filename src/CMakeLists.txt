
add_library(map class.c map.c binary_tree.c util.c)
add_executable(better-alternatives config.c fs.c map)
target_link_libraries(better-alternatives map fuse dl)
