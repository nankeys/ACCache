traceno=25
for i in 10 50 100 300 500 
do
    echo "louvain"$traceno"_"$i"k"
    /usr/bin/louvain-convert -i ../"louvain"$traceno"_"$i't' -o "graph"$traceno"_"$i.b -w "graph"$traceno"_"$i.w
    /usr/bin/louvain-louvain "graph"$traceno"_"$i.b -l -1 -q id_qual -w "graph"$traceno"_"$i.w > "graph"$traceno"_"$i.t
    /usr/bin/louvain-hierarchy "graph"$traceno"_"$i.t -m > "graph"$traceno"_"$i
done
