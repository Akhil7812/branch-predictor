find traces -name '*.trace.*' | sort | while read f; do
  printf "%-45s" "$f"
  ./src/predict "$f" | tail -1 | sed 's/ MPKI//'
done | awk '{print} {s+=$2; n++} END {printf "average MPKI: %.3f\n", s/n}'