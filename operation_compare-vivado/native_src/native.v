`timescale 1ns / 1ps

module native_synth #(
    parameter N = 128,
    parameter WEIGHT_WIDTH = 4,
    parameter ACT_WIDTH = 4
)(
    input                        clk,
    input                        rst,
    input                        start,
    input [N*WEIGHT_WIDTH-1 : 0] i_weights_flat, 
    input [N*ACT_WIDTH-1 : 0]    i_acts_flat,
    
    output reg                   done,
    output reg [15:0]            result
);

    wire signed [WEIGHT_WIDTH-1:0] w_wire [0:N-1];
    wire signed [ACT_WIDTH-1:0]    a_wire [0:N-1];
    wire signed [7:0]             mul_wire [0:N-1];

    genvar i;
    generate
        for (i=0; i<N; i=i+1) begin : GEN_INPUT_MUL
            assign w_wire[i] = i_weights_flat[i*WEIGHT_WIDTH +: WEIGHT_WIDTH];
            assign a_wire[i] = i_acts_flat[i*ACT_WIDTH +: ACT_WIDTH];
            assign mul_wire[i] = $signed(w_wire[i]) * $signed(a_wire[i]);
        end
    endgenerate

    reg signed [8:0] add_stage1 [0:63];
    reg signed [9:0] add_stage2 [0:31];
    reg signed [10:0] add_stage3 [0:15];
    reg signed [11:0] add_stage4 [0:7];
    reg signed [12:0] add_stage5 [0:3];
    reg signed [13:0] add_stage6 [0:1];
    reg signed [14:0] add_stage7;

    integer k;
    always @(posedge clk) begin
        // Stage 1: 128 -> 64 
        for (k=0; k<64; k=k+1) 
            add_stage1[k] <= mul_wire[2*k] + mul_wire[2*k+1];

        // Stage 2: 64 -> 32
        for (k=0; k<32; k=k+1) 
            add_stage2[k] <= add_stage1[2*k] + add_stage1[2*k+1];

        // Stage 3: 32 -> 16
        for (k=0; k<16; k=k+1) 
            add_stage3[k] <= add_stage2[2*k] + add_stage2[2*k+1];

        // Stage 4: 16 -> 8
        for (k=0; k<8; k=k+1)  
            add_stage4[k] <= add_stage3[2*k] + add_stage3[2*k+1];

        // Stage 5: 8 -> 4
        for (k=0; k<4; k=k+1)  
            add_stage5[k] <= add_stage4[2*k] + add_stage4[2*k+1];

        // Stage 6: 4 -> 2
        for (k=0; k<2; k=k+1)  
            add_stage6[k] <= add_stage5[2*k] + add_stage5[2*k+1];

        // Stage 7: 2 -> 1
        add_stage7 <= add_stage6[0] + add_stage6[1];
    end

    reg [6:0] valid_pipe; 
    
    always @(posedge clk) begin
        if (rst) begin
            valid_pipe <= 0;
            done <= 0;
            result <= 0;
        end else begin
            valid_pipe <= {valid_pipe[5:0], start};

            if (valid_pipe[6]) begin
                done <= 1;
                result <= {add_stage7[14],add_stage7};
            end else begin
                done <= 0;
            end
        end
    end

endmodule