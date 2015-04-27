
add_library(classes class.c oolist.c map.c binary_tree.c util.c)
add_executable(better-alternatives config.c fs.c)
target_link_libraries(better-alternatives classes fuse dl)
