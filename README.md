# AC-Cache: A Memory-Efficient Caching System for Small Objects via Exploiting Access Correlations
AC-Cache consists of two main processes: correlation analysis and KV object distribution.
Accordingly, the source code is structured into two execution phases.
Data synchronization between these phases must be manually managed by the tester.
Due to the extended duration of the test process and the inability to automate it via shell scripts, we provide a detailed description of how to test AC-Cache.
The tester is required to manually modify and update certain scripts and source files during the process.

## Requirement
* platform: Linux
* build tools: `cmake (>=3.20)`
* compiler: `gcc (>=4.8)`
* python: `python3 (==3.10)`
* library: `jsoncpp`, `libisal`, `libmemcached`, `libfmt`, `python-prtpy`

## Build
```shell
git clone https://github.com/nankeys/AC-Cache.git
cd CorrelationAnalysis && mkdir _build && cd _build
cmake ..
make -j
```

## Trace process
### Download
* Twitter: Refer to [Twitter cache trace](https://github.com/twitter/cache-trace)
  + `https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/open_source/`
* Meta: Refer to [Running cachebench with the trace workload](https://cachelib.org/docs/Cache_Library_User_Guides/Cachebench_FB_HW_eval)
    + `kvcache/202206`: `aws s3 cp --no-sign-request --recursive s3://cachelib-workload-sharing/pub/kvcache/202206/ ./`
    + `kvcache/202401`: `aws s3 cp --no-sign-request --recursive s3://cachelib-workload-sharing/pub/kvcache/202401/ ./`

### Preprocess
1. Uncompress the trace, for example
```shell
zstd -d cluster2.sort.zst
```
2. Split the trace in days
```shell
# change the fname in split_in_days.py
python3 split_in_days.py
```
3. Generate the stat file
```shell
# change the workload name and stat name in 02.stats.py
python3 02.stats.py
```
4. Split the traces in threads
```shell
# change the traceno in 03.thread_split.py
python3 03.thread_split.py
```
5. For each trace, extract the position of hot objects
```shell
# change the information in advance
python3 FreqExtraction.py
```
6. Put the informaion into `parameter.h`. Put the variations into `flimit`.
7. Change the information in `config.json`.

## Correalation Analysis
1. Change the information in main_correlation.cpp
2. Change the dir and rebuild
```shell
cd _build
make -j
```
3. Run `correlation` to generate the correlation graph.
**Note**: the generation of the correlation graph could take a long time. It will generate a file whose name is `louvaion_node_{trace_no}_{flimit}`
```shell
./correlation
```

## Graph partition
1. Download and compile the `louvain`.
<!-- ```shell
git clone https://github.com/jlguillaume/louvain.git
cd louvain
make
```
-->
```shell
wget https://master.dl.sourceforge.net/project/louvain/louvain-generic.tar.gz?viasf=1
tar -zvxf louvain-generic.tar.gz
make
```

2. Generate initial groups
```shell
bash initial.sh
```

3. merge the group
```shell
# change the infromation of trace
python3 merge.py
```

4. Execute Algorithm 1: Partition correlation graph
```shell
python3 divided_graph.py
```

5. Then we get the graph file of the CGroups.

## Objects Distribution
1. Put the generated information of CGroups to `parameter.h`.
2. Set up the experiments you want to test and changes the variation in `cache.h`.
3. Setup the `Memcached` Nodes.
4. Record information of `Memcached` nodes to `config.json`.
```json
"server_info": [
    {
      "ip": "172.18.96.10",
      "port": 11211
    },{
      "ip": "172.18.96.11",
      "port": 11211
    }
]
```
5. Change the dir and rebuild
```shell
cd _build
make -j
```
6. Run the executable file
``` shell
./CorAna
```
7. The result will be write to `result.txt`.

## Various evaluations
To adopt to a new evaluation, one should change the file `config.h` to get the parameter from the self-defined source file but not the `config.json`.

## Plot
All the scripts for ploting the graph is under directory `plot`.
The testers need to record the result as the format depicted in `*.csv`.
Then the testers can run the python scripts.

## Notes
1. All the paths in the scripts and source code should be carefully checked.
2. Preprocesses is important and would take a long time.
3. The process of correlation analysis takes a long time.