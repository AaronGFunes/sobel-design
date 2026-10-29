import sobel_pkg::*;

module sobel_x(
input clk,
input rst_n,
input start,
input logic [7:0] pixel_base_in [9],
output logic [10:0] pixel_out,
output logic done
);

wire start_trig;
logic start_d1;
logic [5:0] state;
logic signed [9:0] pixel_0_0; //x,y from top left
logic signed [9:0] pixel_0_1;
logic signed [9:0] pixel_0_2;
logic signed [9:0] pixel_2_0; 
logic signed [9:0] pixel_2_1;
logic signed [9:0] pixel_2_2;

logic signed [10:0] s_pixel_sum_1;
logic signed [10:0] s_pixel_sum_2;
logic signed [10:0] s_pixel_sum_3;

logic signed [10:0] s_pixel_out;



localparam s1_idle   = 6'b000001;
localparam s1_mult   = 6'b000010;
localparam s1_add_1  = 6'b000100;
localparam s1_add_2  = 6'b001000;
localparam s1_add_3  = 6'b010000;
localparam s1_done   = 6'b100000;

localparam signed [8:0] const_1 = 1;
localparam signed [8:0] const_2 = 2;

localparam signed [8:0]  const_neg_1 = -1;
localparam signed [8:0]  const_neg_2 = -2;

assign start_trig = (start == 1'b1 && start_d1 == 1'b0) ? 1'b1 : 1'b0;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        done <= 1'b0;
        state <= s1_idle; 
        pixel_0_0 <= 9'b0; 
        pixel_0_1 <= 9'b0; 
        pixel_0_2 <= 9'b0; 
        pixel_2_0 <= 9'b0; 
        pixel_2_1 <= 9'b0; 
        pixel_2_2 <= 9'b0;
        s_pixel_sum_1 <= 9'b0;
        s_pixel_sum_2 <= 9'b0;
        s_pixel_sum_3 <= 9'b0;                         
        s_pixel_out <= 9'b0;
        pixel_out <= 9'b0;
        start_d1 <= 1'b0;
    end
    else begin
        start_d1 <= start;
        case (state)
        s1_idle : begin
            s_pixel_out <= s_pixel_out;
            pixel_out <= pixel_out;
            s_pixel_sum_1 <= s_pixel_sum_1;
            s_pixel_sum_2 <= s_pixel_sum_2;
            s_pixel_sum_3 <= s_pixel_sum_3;
            if(start_trig) begin
                done <= 1'b0;
                state <= s1_mult;
                pixel_0_0 <= {2'b0,pixel_base_in[0]};
                pixel_0_1 <= {2'b0,pixel_base_in[1]};
                pixel_0_2 <= {2'b0,pixel_base_in[2]};
                pixel_2_0 <= {2'b0,pixel_base_in[6]};
                pixel_2_1 <= {2'b0,pixel_base_in[7]};
                pixel_2_2 <= {2'b0,pixel_base_in[8]};                
            end else begin
                done <= done;
                state <= state;
                pixel_0_0 <= pixel_0_0; 
                pixel_0_1 <= pixel_0_1; 
                pixel_0_2 <= pixel_0_2; 
                pixel_2_0 <= pixel_2_0; 
                pixel_2_1 <= pixel_2_1; 
                pixel_2_2 <= pixel_2_2;                 
            end

        end

        s1_mult: begin
            state <= s1_add_1;
            pixel_0_0 <= sat_check_mult(pixel_0_0 * const_neg_1,10'h1FE,10'hE02);
            pixel_0_1 <= sat_check_mult(pixel_0_1 * const_neg_2,10'h1FE,10'hE02);
            pixel_0_2 <= sat_check_mult(pixel_0_2 * const_neg_1,10'h1FE,10'hE02);
            pixel_2_0 <= pixel_2_0; 
            pixel_2_1 <= sat_check_mult(pixel_2_1 * const_2,10'h1FE,10'hE02);        
            pixel_2_2 <= pixel_2_2;                 
            s_pixel_sum_1 <= s_pixel_sum_1;
            s_pixel_sum_2 <= s_pixel_sum_2;
            s_pixel_sum_3 <= s_pixel_sum_3;
            s_pixel_out <= s_pixel_out;
            pixel_out <= pixel_out;
        end

        s1_add_1: begin
            state <= s1_add_2 ;
            pixel_0_0 <= pixel_0_0; 
            pixel_0_1 <= pixel_0_1; 
            pixel_0_2 <= pixel_0_2; 
            pixel_2_0 <= pixel_2_0; 
            pixel_2_1 <= pixel_2_1; 
            pixel_2_2 <= pixel_2_2;
            s_pixel_sum_1 <=  overflow_check_add(pixel_0_0 + pixel_0_1,11'h3FC,11'hC04); //check for overflow
            s_pixel_sum_2 <=  overflow_check_add(pixel_0_2 + pixel_2_0,11'h3FC,11'hC04);
            s_pixel_sum_3 <=  overflow_check_add(pixel_2_1 + pixel_2_2,11'h3FC,11'hC04);                
            s_pixel_out <= s_pixel_out;
            pixel_out <= pixel_out;
        end

        s1_add_2: begin
            state <= s1_add_3;
            pixel_0_0 <= pixel_0_0; 
            pixel_0_1 <= pixel_0_1; 
            pixel_0_2 <= pixel_0_2; 
            pixel_2_0 <= pixel_2_0; 
            pixel_2_1 <= pixel_2_1; 
            pixel_2_2 <= pixel_2_2;
            s_pixel_sum_1 <= s_pixel_sum_1;
            s_pixel_sum_2 <= s_pixel_sum_2;
            s_pixel_out <= overflow_check_add(s_pixel_sum_1 + s_pixel_sum_2,11'h3FC,11'hC04); //check for overflow
            s_pixel_sum_3 <= s_pixel_sum_3;                 
            pixel_out <= pixel_out;
        end

        s1_add_3: begin
            state <= s1_done;
            pixel_0_0 <= pixel_0_0; 
            pixel_0_1 <= pixel_0_1; 
            pixel_0_2 <= pixel_0_2; 
            pixel_2_0 <= pixel_2_0; 
            pixel_2_1 <= pixel_2_1; 
            pixel_2_2 <= pixel_2_2;                 
            s_pixel_sum_1 <= s_pixel_sum_1;
            s_pixel_sum_2 <= s_pixel_sum_2;
            s_pixel_sum_3 <= s_pixel_sum_3;
            s_pixel_out <= overflow_check_add(s_pixel_out + s_pixel_sum_3,11'h3FC,11'hC04); //check for overflow;
            pixel_out <= pixel_out;
        end

        s1_done: begin
            state <= s1_idle;
            done <= 1'b1;
            pixel_0_0 <= pixel_0_0; 
            pixel_0_1 <= pixel_0_1; 
            pixel_0_2 <= pixel_0_2; 
            pixel_2_0 <= pixel_2_0; 
            pixel_2_1 <= pixel_2_1; 
            pixel_2_2 <= pixel_2_2;
            s_pixel_sum_1 <= s_pixel_sum_1;
            s_pixel_sum_2 <= s_pixel_sum_2;
            s_pixel_sum_3 <= s_pixel_sum_3;
            s_pixel_out <= s_pixel_out;                 
            pixel_out <= unsigned'(s_pixel_out);
        end

        default: begin
            done <= 1'b0;
            state <= s1_idle; 
            pixel_0_0 <= 9'b0; 
            pixel_0_1 <= 9'b0; 
            pixel_0_2 <= 9'b0; 
            pixel_2_0 <= 9'b0; 
            pixel_2_1 <= 9'b0; 
            pixel_2_2 <= 9'b0;                 
            s_pixel_sum_1 <= 9'b0;
            s_pixel_sum_2 <= 9'b0;
            s_pixel_sum_3 <= 9'b0;
            s_pixel_out <= 9'b0;
            pixel_out <= 9'b0;
        end

        endcase
    end
end

endmodule