# mips_simulator

Two mips instruction simulators, one implementing the MIPS R10000 CPU and the other more basic to create the more basic
data structures.  The basic simulator includes a tournament style branch predictor, multi level cache, write back buffer, 
miss status holding registers, variable cache replacement policies, anda next line prefetcher. The MIPS R10000 builds on 
the basic simulator and adds a reservation station, reorder buffer, load/store queue, map table and architectural map table.
