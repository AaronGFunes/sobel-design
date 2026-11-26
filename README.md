# Sobel-design
Github Repository for Sobel Filter design implementation on FPGA\
Author : Aaron Funes\
Last revision : 11/25/26

- documents  
    Graduate Project Paper
    - Edge Detection in FPGA Sobel Filter Design.docx
    - Edge Detection in FPGA Sobel Filter Design.pdf
- hdl\
    SystemVerilog Design and Testbench files for Sobel Filter
    - sobel.sv : Top-level Sobel module
    - sobel_pkg.sv : Package file with saturation and overflow functions
    - sobel_tb.sv : Testbench file for Sobel module
    - sobel_x.sv : X-derivative Sobel submodule
    - sobel_y.sv : Y-derivative Sobel submodule
    - sqrt.sv : Square root submodule
- ip_repo
    - sobel_ip_1_0
        Vivado Sobel IP module
- references\
    References used for Design

- results
    - eye_bw.bmp :Original image
    - sobel.BMP : Result image from Sobel Filter operation
    - test.log : output log from C code testing for pixle indexing and accessing
    - vitis_serial_check_pixel_location.png : Vitis terminal showing pixel indexing operation
    - vitis_serial_output.png : Final Vitis terminal output
- sobel\
    Vivado Sobel filter project for SystemVerilog files
- sobel_soc\
    Vivado Sobel filter project for SOC Block Diagram
    - sobel_soc_vitis
        Vitis Platform Project
- soc_code\
    C code for Vitis platform
    - bmp.c
    - bmp.h
    - filter.c
    - filter.h
    - main.c
- src\
    C code for testing BMP format and indexing at PC platform level



