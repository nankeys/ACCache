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
cd src && mkdir _build && cd _build
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

### Uncompress
Uncompress the trace, for example
```shell
cd /path/to/cluster02.sort.zst
zstd -d cluster02.sort.zst
```

**Notice**: The path to store the trace should be carefully selected with enough disk space. And all intermediate generated files will be stored under this path. Please remember this `path` for further usage.

### Information Collection
At first, the scheme will collect some informations about this test. You can select the item you want to change. You can re-run this script at any time you want to change the configure files.
```shell
python3 InformationCollection.py
```
**Notice**: You will go back many times as you want to change the parameters. The specific parameters should be changed before your test.

### Preprocess
```shell
# All the steps work under directory `Preproccess/`
cd ACCache/Preprocess
```

1. Pre-process the trace files.
```shell
# change the fname in preprocess.py
python3 preprocess.py
```

3. Split the traces of specific day into threads
```shell
# change the traceno in thread_split.py
python3 thread_split.py
```

## Correalation Analysis
1. Change the dir and rebuild
```shell
cd ACCache/src/_build
make -j
```

2. Run `correlation` to generate the correlation graph.
**Note**: the generation of the correlation graph could take a long time. It will generate a file whose name is `louvaion{trace_no}`
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
wget https://master.dl.sourceforge.net/project/louvain/louvain-generic.tar.gz
tar -zvxf louvain-generic.tar.gz
cd louvain-generic/
make
```

2.  Generate the graph file of the CGroup under the `path`. Please provide the path to directory `louvain-generic`.
```shell
cd ACCache/
python3 GroupDivision.py
```

## Objects Distribution
1. Change the dir and rebuild
```shell
cd ACCache/src/_build
make -j
```
2. Run the executable file
``` shell
./CorAna
```
3. The result will be write to `result.txt`.

## Various evaluations
To adopt to a new evaluation, one should re-run `InformationCollection` to change the parameter.

## Plot
All the scripts for ploting the graph is under directory `plot`.
The testers need to record the result as the format depicted in `*.csv`.
Then the testers can run the python scripts.

## Notes
1. All the paths in the scripts and source code should be carefully checked.
2. Preprocesses is important and would take a long time.
3. The process of correlation analysis takes a long time.
