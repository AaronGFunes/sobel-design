package sobel_pkg;

function automatic signed [9:0] sat_check_mult(
    input logic signed [17:0] a, //result
    input logic signed [9:0] b, //upper bound
    input logic signed [9:0] c //lower bound
    );
    if(a > b)
    return b;
    else if (a <c)
    return c;
    else
    return a[9:0];
endfunction

function automatic signed [20:0] sat_check_mult_sq(
    input logic signed [20:0] a, //result
    input logic signed [20:0] b, //upper bound
    input logic signed [20:0] c //lower bound
    );
    if(a > b)
    return b;
    else if (a <c)
    return c;
    else
    return a;
endfunction

function automatic [10:0] overflow_check_add(
    input logic signed [10:0] a, //result
    input logic signed [10:0] b, // upper bound
    input logic signed [10:0] c // lower bound
    );
    if(a > b)
    return b;
    else if (a <c)
    return c;
    else
    return a;

endfunction

function automatic [21:0] overflow_check_add_s22(
    input logic signed [21:0] a, //result
    input logic signed [21:0] b, // upper bound
    input logic signed [21:0] c // lower bound
    );
    if(a > b)
    return b[21:0];
    else if (a <c)
    return c[21:0];
    else
    return a[21:0];

endfunction

    
endpackage