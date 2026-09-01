import sobel_pkg::*;

module sobel(
input clk,
input rst_n,
input start,
input logic [7:0] pixel_base_in [9],
output logic [7:0] pixel_out,
output logic done
);

wire start_trig;
logic start_d1;
logic [5:0] state;

wire signed [10:0] gx_pixel;
wire signed [10:0] gy_pixel;
logic signed [20:0] gx_pixel_squared;
logic signed [20:0] gy_pixel_squared;
logic signed [21:0] g_sum;
wire signed [10:0] g_sq_root;

logic  start_sqrt;
logic  start_filter;
logic done_filter_x;
logic done_filter_y;
wire done_filter;
logic done_sq_root;
logic reg_done_filter;
logic reg_done_sq_root;
wire done_filter_trig;
wire done_sq_root_trig;

localparam s1_idle      = 6'b000001;
localparam s1_filter    = 6'b000010;
localparam s1_mult      = 6'b000100;
localparam s1_add       = 6'b001000;
localparam s1_sqr_root  = 6'b010000;
localparam s1_done      = 6'b100000;


sobel_x u_sobel_x (
.clk(clk),
.rst_n(rst_n),
.start(start_filter),
.pixel_base_in(pixel_base_in),
.pixel_out(gx_pixel),
.done(done_filter_x)
);

sobel_y u_sobel_y (
.clk(clk),
.rst_n(rst_n),
.start(start_filter),
.pixel_base_in(pixel_base_in),
.pixel_out(gy_pixel),
.done(done_filter_y)
);

square_root #(.N(22)) u_square_root (
.clk(clk),
.rst_n(rst_n),
.start(start_sqrt),
.num_in(g_sum),
.sq_root(g_sq_root),
.done(done_sq_root)
);


assign start_trig = (start == 1'b1 && start_d1 == 1'b0) ? 1'b1 : 1'b0;
assign done_filter = done_filter_x && done_filter_y;
assign done_filter_trig = (done_filter == 1'b1 && reg_done_filter == 1'b0) ? 1'b1 : 1'b0;

assign done_sq_root_trig = (done_sq_root == 1'b1 && reg_done_sq_root == 1'b0) ? 1'b1 : 1'b0;


always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        done <= 1'b0;
        start_d1 <= 1'b0;
        state <= s1_idle; 
        start_sqrt <= 1'b0;
        start_filter <= 1'b0;
        gx_pixel_squared <= 18'h0;
        gy_pixel_squared <= 18'h0;
        g_sum <= 18'h0;
        pixel_out <= 8'h0;
        reg_done_filter <= 1'b0;
        reg_done_sq_root <= 1'b0;
    end
    else begin
        start_d1 <= start;
        reg_done_filter <= done_filter;
        reg_done_sq_root <= done_sq_root;
        case (state)
        s1_idle : begin

            gx_pixel_squared <= gx_pixel_squared;
            gy_pixel_squared <= gy_pixel_squared;
            g_sum <= g_sum;
            start_sqrt <= start_sqrt;
            done <= 1'b0;
            
            if(start_trig) begin
                state <= s1_filter;
                start_filter <= 1'b1;
                pixel_out <= 8'h0;
            end else begin
                start_filter <= start_filter;
                state <= s1_idle;
                pixel_out <= pixel_out;
            end  

        end

        s1_filter: begin
            gx_pixel_squared <= gx_pixel_squared;
            gy_pixel_squared <= gy_pixel_squared;
            g_sum <= g_sum;
            start_sqrt <= start_sqrt;
            done <= done;
            start_filter <= 1'b0;
            pixel_out <= pixel_out;
            if(done_filter_trig) begin
                state <= s1_mult;
            end
            else begin
                state <= state;                
            end 
        end

        s1_mult: begin
            g_sum <= g_sum;
            gx_pixel_squared <= sat_check_mult_sq(gx_pixel * gx_pixel,21'h0FE010,21'h101FF0);
            gy_pixel_squared <= sat_check_mult_sq(gy_pixel * gy_pixel,21'h0FE010,21'h101FF0);
            done <= done;
            start_filter <= start_filter;
            pixel_out <= pixel_out;
            start_sqrt <= start_sqrt;
            state <= s1_add;
        end

        s1_add: begin
            g_sum <= overflow_check_add_s22(gx_pixel_squared+gy_pixel_squared,22'h1FC020,22'h203FE0);
            gx_pixel_squared <= gx_pixel_squared;
            gy_pixel_squared <= gy_pixel_squared;
            done <= done;
            start_filter <= start_filter;
            pixel_out <= pixel_out;
            start_sqrt <= 1'b1;
            state <= s1_sqr_root;
        end

        s1_sqr_root: begin
            gx_pixel_squared <= gx_pixel_squared;
            gy_pixel_squared <= gy_pixel_squared;
            g_sum <= g_sum;
            start_sqrt <= 1'b0;
            done <= done;
            start_filter <= start_filter;
            pixel_out <= pixel_out;             
            if(done_sq_root_trig) begin
                state <= s1_done;
            end
            else begin
                state <= state;   
            end 
        end

        s1_done: begin
            gx_pixel_squared <= gx_pixel_squared;
            gy_pixel_squared <= gy_pixel_squared;
            g_sum <= g_sum;
            start_sqrt <= start_sqrt;
            done <= 1'b1;;
            start_filter <= start_filter;  
            state <= s1_idle;          
            if(g_sq_root > 8'hFF)
                pixel_out <= 8'hFF;
            else
                pixel_out <= g_sq_root[7:0];
        end

        default: begin
            done <= 1'b0;
            state <= s1_idle; 
            start_sqrt <= 1'b0;
            start_filter <= 1'b0;
            gx_pixel_squared <= 18'h0;
            gy_pixel_squared <= 18'h0;
            g_sum <= 18'h0;
            pixel_out <= 8'h0;
        end
                
        endcase
    end
end

endmodule