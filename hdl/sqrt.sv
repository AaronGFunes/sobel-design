module square_root
    #(parameter N = 8)
    (   input clk,  //clk
        input rst_n,  //Asynchronous active high rst_n.
        input start,     
        input [N-1:0] num_in,   //this is the number for which we want to find square root.
        output logic done,     //This signal goes high when output is ready
        output logic [N/2-1:0] sq_root  //square root of 'num_in'
    );

    logic [N-1:0] a;   //original input.
    logic [N/2+1:0] left,right;     //input to adder/sub.r-remainder.
    logic signed [N/2+1:0] r;
    logic [N/2-1:0] q;    //result.
    integer i;   //index of the loop. 

    wire start_trig;
    logic start_d1;
    logic [5:0] state;

    localparam s1_idle = 3'b001;
    localparam s1_busy = 3'b010;
    localparam s1_done = 3'b100;
    
    assign start_trig = (start == 1'b1 && start_d1 == 1'b0) ? 1'b1 : 1'b0;


    always @(posedge clk or negedge rst_n) 
    begin
        if (!rst_n) begin   //rst_n the variables.
            done <= 0;
            sq_root <= 0;
            i = 0;
            a = 0;
            left = 0;
            right = 0;
            r = 0;
            q = 0;
            state<=s1_idle;
            start_d1 <= 1'b0;
        end    
        else begin
            start_d1 <= start;
            case(state)

            s1_idle: begin
                done <= 0;
                sq_root <= sq_root;
                i = i;
                a = a;
                left = left;
                right = right;
                r = r;
                q = q;
                if(start_trig)
                    state <= s1_busy;
                else 
                    state <= state;                                
            end

            s1_busy: begin
                //Before we start the first clk cycle get the 'input' to the variable 'a'.
                if(i == 0) begin  
                    a = num_in;
                    done <= 0;    //reset 'done' signal.
                    i = i+1;   //increment the loop index.
                end
                else if(i < N/2) begin //keep incrementing the loop index.
                    i = i+1;  
                end
                //These statements below are derived from the block diagram.
                right = {q,r[N/2+1],1'b1};
                left = {r[N/2-1:0], a[N-1:N-2]};
                a = {a[N-3:0], 2'b0};  //shifting left by 2 bit.
                if ( r[N/2+1] == 1)    //add or subtract as per this bit.
                    r = left + right;
                else
                    r = left - right;
                q = {q[N/2-2:0], ~r[N/2+1]};
                if(i == N/2)      //This means the max value of loop index has reached. 
                    state <= s1_done;
                else
                    state <= state;                                 
            end

            s1_done: begin
                done <= 1;    //make 'done' high because output is ready.
                i = 0; //reset loop index for beginning the next cycle.
                sq_root <= q;   //assign 'q' to the output port.
                //reset other signals for using in the next cycle.
                left = 0;
                right = 0;
                r = 0;
                q = 0;
                state <= s1_idle;
            end

            default: begin
                done <= 0;
                sq_root <= 0;
                i = 0;
                a = 0;
                left = 0;
                right = 0;
                r = 0;
                q = 0;
                state<=s1_idle;
                start_d1 <= 1'b0;
            end

            endcase
        end    
    end

endmodule