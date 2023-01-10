set(SRCROOT ${PROJECT_SOURCE_DIR}/kernel/src/arch/amd64)

FILE(GLOB_RECURSE arch_asm_sources *.asm)
FILE(GLOB_RECURSE arch_cxx_sources *.cpp)

target_sources(hugos_core_kernel PRIVATE ${arch_asm_sources} ${arch_cxx_sources})
