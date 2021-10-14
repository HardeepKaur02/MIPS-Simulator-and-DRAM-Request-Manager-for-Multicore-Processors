#TestCase 1
This testcase shows the MRM delay corresponding to case 2.2.1.
In the cycle of column access by a dRAM request, a DRAM request from buffer is sent to DRAM, so that it doesn't appear as a delay.
If there is no request with same row as active row but there is indeed such a request sent by processor to MRM i that cycle only, then MRM would choose to prefetch it and send to DRAM in 1 cycle.
This leads to a visible delay of 1 since the prefetching was done when the DRAM was still executing the previous instruction. In total, MRM took 2 cycles to prefetch the request and send it to DRAM.
