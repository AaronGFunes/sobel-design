`timescale 1ns/1ps

module sobel_tb();

bit rst_n = 1'b1;
bit clk = 1'b0;
bit start = 1'b0;
bit [7:0] pixel_base_in [9] = '{default : 8'h0};
bit [7:0] pixel_out = 8'b0;
bit [7:0] exp_pixel_out = 8'b0;
bit done = 1'b0;

int  g_x;
int  g_y;
real  g_real;
int g;

assign g_x = (-1*pixel_base_in[0]) + (-2*pixel_base_in[1]) + (-1*pixel_base_in[2]) + 1*pixel_base_in[6] 
            + 2*pixel_base_in[7] + 1*pixel_base_in[8];
assign g_y = (-1*pixel_base_in[0]) + (-2*pixel_base_in[3]) + (-1*pixel_base_in[6]) + 1*pixel_base_in[2] 
            + 2*pixel_base_in[5] + 1*pixel_base_in[8];
assign g_real = $sqrt((g_x * g_x)+(g_y * g_y));
assign g = $floor(g_real);




always #10.4ns clk = ~clk;

sobel u_dut(
    .clk(clk),
    .rst_n(rst_n),
    .start(start),
    .pixel_base_in(pixel_base_in),
    .pixel_out(pixel_out),
    .done(done)
);


initial begin
    rst_n = 1'b0;
    pixel_base_in = '{default : 8'hFF};
    exp_pixel_out = sobel_out(g);
    #12ns;
    rst_n = 1'b1;
    @(negedge clk);
    start = 1'b1;
    repeat(2) @(posedge clk);
    start = 1'b0;
    @(posedge done);
    #1;
    assert(pixel_out == exp_pixel_out) else
        $error("actual value %0x != expected value %0x",pixel_out,exp_pixel_out);
    #20ns;
    pixel_base_in[0:2] = '{default : 8'hFF};
    pixel_base_in[3:5] = '{default : 8'h00};
    pixel_base_in[6:8] = '{default : 8'h7F};
    repeat(2) @(posedge clk);
    @(negedge clk);
    exp_pixel_out = sobel_out(g);
    start = 1'b1;
    repeat(2) @(posedge clk);
    start = 1'b0;
    @(posedge done);
    #1;
    assert(pixel_out == exp_pixel_out) else
        $error("actual value %0x != expected value %0x",pixel_out,exp_pixel_out);
    #20ns;
    repeat(100) begin
        randomize(pixel_base_in);
        #12ns;
        exp_pixel_out = sobel_out(g);
        @(negedge clk);
        start = 1'b1;
        repeat(2) @(posedge clk);
        start = 1'b0;
        @(posedge done);
        #1;
        assert(pixel_out == exp_pixel_out) else
            $error("actual value %0x != expected value %0x",pixel_out,exp_pixel_out);
        #20ns;
    end

    $finish;
end


function bit [7:0] sobel_out(
    input int g
);
if(g>255)
    return 8'(255);
else
    return 8'(g);
endfunction

endmodule