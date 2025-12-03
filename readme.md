# Performance Comparison between W4A4 and W4ASpike
* Aiming to provide a comparative analysis of two different methods, W4A4 and W4ASpike (with different encoding schemes), for a specific computational workload, implemented using High-Level Synthesis (HLS) for FPGA deployment.

## WorkLoad
*   **Vector Dot Product**: executing 128-dimensional vector dot product 1024 times
  
## HLS Comparison 
* compare latency, resource usage and timing. Performance is evaluated based on the post-synthesis results from Vitis HLS. Component part is xczu9eg-ffvb1156-2-e. Target clock is 200MHz with a clock_uncertainty of 27%.
### result
| design      | clock  | co-sim latency | post_syn timing (ns) | LUT   | FF    | DSP | BRAM | SRL  | CLB  |
|-------------|--------|----------------|----------------------|-------|-------|-----|------|------|------|
| naive dot   | 200MHz | 1138           | 2.299                | 6731  | 10131 | 58  | 30   | 407  | 1523 |
| s-bin-par   | 200MHz | 1170           | 2.430                | 16919 | 14940 | 0   | 30   | 1847 | 3380 |