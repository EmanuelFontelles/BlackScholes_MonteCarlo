# SDAccel creates a compile run or solution of the application per invocation of the tool
# Set the name of the solution to be used throughut the current script
set solution "solution"

# This script will copy the results of the experiment to a user defined Location
set solution_results "results"
# Create a solution
create_solution -name $solution -force

# Define the target devices for the current solution
add_device -vbnv xilinx:adm-pcie-7v3:1ddr:2.0

# Host Source Files
add_files "main.cpp"
add_files "../headers/stockData.cpp"
# Host Compiler Flags
set_property -name host_cflags -value "-g -Wall -D __CLIANG__" -objects [current_solution]


# Define a kernel to be compiled by SDAccel
# The kernel definition name must match the kernel name in the source code
# The kernel type is given by the source code type for the kernel
create_kernel -type c blackAsian

# Adding files for a kernel works in the same way as adding files for host code.
# User must associate source files to specific kernels using the -kernel option to the add_files command
add_files -kernel [get_kernels blackAsian] "blackAsian.cpp"
add_files -kernel [get_kernels blackAsian] "blackScholes.cpp"
add_files -kernel [get_kernels blackAsian] "../headers/RNG.cpp"
add_files -kernel [get_kernels blackAsian] "../headers/stockData.cpp"

# Create a binary container. Every SDAccel application has at least 1 binary container to hold the FPGA binary.
create_opencl_binary blackAsian1
# Depending on configuration, a target device may have 1 or more areas reserved for kernels compiled by SDAccel
# The user must tell SDAccel which area to target on the device. This sets the compilation parameters for the kernel.
set_property region OCL_REGION_0 [get_opencl_binary blackAsian1]
# Kernels are compiled into compute units. There is at least 1 compute unit per kernel in an FPGA binary.
create_compute_unit -opencl_binary [get_opencl_binary blackAsian1] -kernel [get_kernels blackAsian] -name K1



# Compile the design for CPU based emulation
#compile_emulation -flow cpu -opencl_binary [get_opencl_binary blackAsian]

# Run the design in CPU emulation mode
#run_emulation -flow cpu -args "blackAsian.xclbin"


# Compile the median filter for CPU emulation
compile_emulation -flow hardware -opencl_binary [get_opencl_binary blackAsian1]

# Run the application
run_emulation -flow hardware -args "blackAsian1.xclbin"

#Compile the application to run on an FPGA
#build_system

#Package_system
#package_system

# Run the application in hardware
#run_system -args "blackAsian1.xclbin"


# Compute the resource estimate for the application
report_estimate


