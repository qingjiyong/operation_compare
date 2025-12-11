`timescale 1ns / 1ps
module spike_array_synth #(
    parameter N = 128,
    parameter BITWIDTH = 4   
)(
    input                        clk,
    input                        rst,
    input                        start,        
    
    input [N*4-1 : 0]            i_weights_flat, 
    input [N*BITWIDTH-1 : 0]     i_acts_flat,

    output reg                   done,         
                                               
    output reg [15:0]            result 
);

    localparam BATCH_NUM = 1024;

    wire signed [3:0]        w_wire [0:N-1];
    wire signed [BITWIDTH-1:0] a_wire [0:N-1];
    wire                     sign_wire [0:N-1];

    genvar t;
    generate
        for (t=0; t<N; t=t+1) begin : INPUT_LOGIC
            assign w_wire[t] = i_weights_flat[t*4 +: 4];
            wire [BITWIDTH-1:0] raw_act = i_acts_flat[t*BITWIDTH +: BITWIDTH];
            assign a_wire[t]    = raw_act[BITWIDTH-1] ? (~raw_act + 1) : raw_act;
            
            assign sign_wire[t] = raw_act[BITWIDTH-1] ? 1'b0 : 1'b1;
        end
    endgenerate

    wire signed [4:0] partial0 [0:N-1];
    wire signed [4:0] partial1 [0:N-1];
    wire signed [4:0] partial2 [0:N-1];
    wire signed [4:0] partial3 [0:N-1];
    
    genvar k;
    generate
        for (k=0; k<N; k=k+1) begin : PARTIAL_LOGIC
            assign partial0[k] = a_wire[k][0] ? (sign_wire[k] ? w_wire[k] : -w_wire[k]) : 5'sd0;
            assign partial1[k] = a_wire[k][1] ? (sign_wire[k] ? w_wire[k] : -w_wire[k]) : 5'sd0;
            assign partial2[k] = a_wire[k][2] ? (sign_wire[k] ? w_wire[k] : -w_wire[k]) : 5'sd0;
            assign partial3[k] = a_wire[k][3] ? (sign_wire[k] ? w_wire[k] : -w_wire[k]) : 5'sd0;
        end
    endgenerate

    reg signed [5:0] l1_tree0[0:63], l1_tree1[0:63], l1_tree2[0:63], l1_tree3[0:63];
    reg signed [6:0] l2_tree0[0:31], l2_tree1[0:31], l2_tree2[0:31], l2_tree3[0:31];
    reg signed [7:0] l3_tree0[0:15], l3_tree1[0:15], l3_tree2[0:15], l3_tree3[0:15];
    reg signed [8:0] l4_tree0[0:7],  l4_tree1[0:7],  l4_tree2[0:7],  l4_tree3[0:7];
    reg signed [9:0] l5_tree0[0:3],  l5_tree1[0:3],  l5_tree2[0:3],  l5_tree3[0:3];
    reg signed [10:0] l6_tree0[0:1],  l6_tree1[0:1],  l6_tree2[0:1],  l6_tree3[0:1];
    reg signed [15:0] sum_tree0, sum_tree1, sum_tree2, sum_tree3;

    integer i;
    always @(posedge clk) begin
        // --- Level 1 ---
        for(i=0; i<64; i=i+1) begin
            l1_tree0[i] <= partial0[2*i] + partial0[2*i+1];
            l1_tree1[i] <= partial1[2*i] + partial1[2*i+1];
            l1_tree2[i] <= partial2[2*i] + partial2[2*i+1];
            l1_tree3[i] <= partial3[2*i] + partial3[2*i+1];
        end
        // --- Level 2 ---
        for(i=0; i<32; i=i+1) begin
            l2_tree0[i] <= l1_tree0[2*i] + l1_tree0[2*i+1];
            l2_tree1[i] <= l1_tree1[2*i] + l1_tree1[2*i+1];
            l2_tree2[i] <= l1_tree2[2*i] + l1_tree2[2*i+1];
            l2_tree3[i] <= l1_tree3[2*i] + l1_tree3[2*i+1];
        end
        // --- Level 3 ---
        for(i=0; i<16; i=i+1) begin
            l3_tree0[i] <= l2_tree0[2*i] + l2_tree0[2*i+1];
            l3_tree1[i] <= l2_tree1[2*i] + l2_tree1[2*i+1];
            l3_tree2[i] <= l2_tree2[2*i] + l2_tree2[2*i+1];
            l3_tree3[i] <= l2_tree3[2*i] + l2_tree3[2*i+1];
        end
        // --- Level 4 ---
        for(i=0; i<8; i=i+1) begin
            l4_tree0[i] <= l3_tree0[2*i] + l3_tree0[2*i+1];
            l4_tree1[i] <= l3_tree1[2*i] + l3_tree1[2*i+1];
            l4_tree2[i] <= l3_tree2[2*i] + l3_tree2[2*i+1];
            l4_tree3[i] <= l3_tree3[2*i] + l3_tree3[2*i+1];
        end
        // --- Level 5 ---
        for(i=0; i<4; i=i+1) begin
            l5_tree0[i] <= l4_tree0[2*i] + l4_tree0[2*i+1];
            l5_tree1[i] <= l4_tree1[2*i] + l4_tree1[2*i+1];
            l5_tree2[i] <= l4_tree2[2*i] + l4_tree2[2*i+1];
            l5_tree3[i] <= l4_tree3[2*i] + l4_tree3[2*i+1];
        end
        // --- Level 6 ---
        for(i=0; i<2; i=i+1) begin
            l6_tree0[i] <= l5_tree0[2*i] + l5_tree0[2*i+1];
            l6_tree1[i] <= l5_tree1[2*i] + l5_tree1[2*i+1];
            l6_tree2[i] <= l5_tree2[2*i] + l5_tree2[2*i+1];
            l6_tree3[i] <= l5_tree3[2*i] + l5_tree3[2*i+1];
        end
        // --- Level 7 ---
        sum_tree0 <= l6_tree0[0] + l6_tree0[1];
        sum_tree1 <= l6_tree1[0] + l6_tree1[1];
        sum_tree2 <= l6_tree2[0] + l6_tree2[1];
        sum_tree3 <= l6_tree3[0] + l6_tree3[1];
    end


    reg signed [15:0] final_sum;
    always @(posedge clk) begin
        final_sum <= sum_tree0 + (sum_tree1 <<< 1) + (sum_tree2 <<< 2) + (sum_tree3 <<< 3);//shift
    end

    reg [7:0] valid_pipe;

    always @(posedge clk) begin
        if (rst) begin
            valid_pipe <= 0;
            result     <= 0;
            done       <= 0;
        end else begin
            valid_pipe <= {valid_pipe[6:0], start};

            if (valid_pipe[7]) begin
                result <= final_sum;
                done <= 1'b1; 
            end else begin
                done <= 1'b0;
            end
        end
    end

endmodule