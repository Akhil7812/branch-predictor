# Pentium M-style Hybrid Branch Predictor

Implements `pm_predictor` in `src/my_predictor.h`: a bimodal predictor plus a 2-way tagged global predictor (loop predictor and target prediction skipped, per the assignment).

## Design
- **Bimodal:** 4096 2-bit counters, indexed by the low 12 bits of the branch address.
- **Global:** 512 sets x 2 ways, each entry = valid bit, 6-bit tag, 2-bit counter. LRU replacement per set.
  - Hash = branch address XOR 15-bit global history register; the top 9 bits pick the set, the low 6 bits are the tag.
  - Global hit: use the global counter. Miss: use the bimodal prediction.
  - On a miss, allocate a new entry (counter starts at 2 if taken, 1 if not taken).

## Build and run
```
cd src && make && cd ..
./src/predict traces/164.gzip/gzip.trace.bz2   # one trace
bash runall.sh                                  # all traces + average MPKI
```

## Results (average MPKI over 20 traces)
- Bimodal only: 10.299
- Hybrid: 7.259