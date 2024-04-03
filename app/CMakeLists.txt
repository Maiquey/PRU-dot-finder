# Build the app, using the HAL

include_directories(../pru-as4/)
include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(pru-dot-finder ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(pru-dot-finder LINK_PRIVATE hal)

# Copy executable to final location (change `pru-dot-finder` to project name as needed)
add_custom_command(TARGET pru-dot-finder POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:pru-dot-finder>"
     "~/cmpt433/public/myApps/pru-dot-finder" 
  COMMENT "Copying ARM executable to public NFS directory")
