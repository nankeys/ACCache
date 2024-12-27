import os
import json
import time

def get_input(promt, default):
    value = input(promt + f" (default: {default}): \n")
    if value == '':
        return default
    else:
        return value

# load JSON file
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
    return data
    

class GroupDivision:
    def __init__(self):
        self.config_file = 'src/config.json'
        self.data = read_json_file(self.config_file)
        self.traceno = self.data["trace_no"]
        self.filepath = self.data["path_prefix"]
        self.nodenum = self.data["server_num"]
        
        self.execpath = get_input('The path to the directory louvain-genetic, prefix only.', self.data["path_prefix"])
        if(self.execpath == ''):
            print("Error getting the path, exiting...")
            exit(-1)
            
        freqkeys_file = "freqkeys{:02d}".format(self.traceno)
        with open(self.filepath + "/" + freqkeys_file, 'r') as fin:
            self.freqkeys = fin.readlines()
        for i in range(len(self.freqkeys)):
            self.freqkeys[i] = self.freqkeys[i].strip()
            
        stat_file = "stat{}".format(self.traceno)
        with open(self.filepath + "/" + stat_file, 'r') as fin:
            keystat = fin.readlines()
        self.key_info = dict()
        self.tsize = 0
        self.tfreq = 0
        for info in keystat:
            key, size, freq = info.strip().split("\t")
            self.key_info[key] = (int(size), int(freq))
            
        for fkey in self.freqkeys:
            self.tsize += self.key_info[fkey][0]
            self.tfreq += self.key_info[fkey][1]
            
    def graph2group(self, is_sub:bool, subno:int):
        if(is_sub == False):
            status = os.system("{0}/louvain-generic/convert -i {1}/louvain{2} -o {1}/graph{2}.b -w {1}/graph{2}.w".format(self.execpath, self.filepath, self.traceno))
            time.sleep(1)
            status = os.system("{0}/louvain-generic/louvain {1}/graph{2}.b -l -1 -q id_qual -w {1}/graph{2}.w > {1}/graph{2}.t".format(self.execpath, self.filepath, self.traceno))
            time.sleep(1)
            status = os.system("{0}/louvain-generic/hierarchy {1}/graph{2}.t -m > {1}/graph{2}".format(self.execpath, self.filepath, self.traceno))
            time.sleep(1)
        else:
            status = os.system("{0}/louvain-generic/convert -i {1}/louvain{2}_{3} -o {1}/graph{2}_{3}.b -w {1}/graph{2}_{3}.w".format(self.execpath, self.filepath, self.traceno, subno))
            time.sleep(1)
            status = os.system("{0}/louvain-generic/louvain {1}/graph{2}_{3}.b -l -1 -q id_qual -w {1}/graph{2}_{3}.w > {1}/graph{2}_{3}.t".format(self.execpath, self.filepath, self.traceno, subno))
            time.sleep(1)
            status = os.system("{0}/louvain-generic/hierarchy {1}/graph{2}_{3}.t -m > {1}/graph{2}_{3}".format(self.execpath, self.filepath, self.traceno, subno))
            time.sleep(1)
        
    def merge_group(self, subno: int, is_sub = True):
        is_subgraph = is_sub
        subno = subno
        self.parent_graph = "graph{}_agg".format(self.traceno)

        if is_subgraph:
            self.gprefix = "graph{}_{}".format(self.traceno, subno)
        else:
            self.gprefix = "graph{}".format(self.traceno)
        group_file = self.gprefix
        print("group_file =", group_file)

        if is_subgraph:
            with open("{}/{}".format(self.filepath, self.parent_graph), 'r') as fin:
                pcnts = fin.readlines()

            pgroups = list()
            for i in range(0, len(pcnts), 2):
                l, s, f = pcnts[i].strip().split('\t')
                tmp = pcnts[i + 1].strip().split('\t')
                for j in range(len(tmp)):
                    tmp[j] = int(tmp[j])
                pgroups.append(tmp)

        with open("{}/{}".format(self.filepath, group_file), 'r') as fin:
            cnts = fin.readlines()

        dgroups = dict() #[list() for i in range(group_num)]
        gsize = list()
        gfreq = list()
        glen = list()

        for line in cnts:
            kn, n =  line.strip().split()
            if is_subgraph:
                if int(kn) not in pgroups[subno]:
                    continue
            if n not in dgroups:
                dgroups[n] = list()
            dgroups[n].append(int(kn))

        groups = list()
        for d in dgroups:
            groups.append(dgroups[d])

        total_size = 0
        total_freq = 0

        for i in range(len(groups)):
            groups[i].sort()
            size = 0
            freq = 0
            for node in groups[i]:
                key = self.freqkeys[node]
                size += self.key_info[key][0]
                freq += self.key_info[key][1]
            total_size += size
            total_freq += freq
            gsize.append(size)
            gfreq.append(freq)
            glen.append(len(groups[i]))

        # print(sum(glen))
        #print(len(pgroups[subno]))
        
        if(is_sub == False):
            self.gropnum = len(groups)

        with open("{}/{}_agg".format(self.filepath, self.gprefix), 'w') as fout:
            for i in range(len(groups)):
                if(glen[i] == 0): continue
                fout.write("%d\t%d\t%d\n" % (glen[i], gsize[i], gfreq[i]))
                for node in groups[i]:
                    fout.write(str(node)+'\t')
                fout.write('\n')
                
    def split(self, g):
        source = "{}/{}".format(self.filepath, self.parent_graph)
        favg = self.tfreq / self.nodenum
        with open(source, 'r') as fin:
            cnts = fin.readlines()

        sgroups = list()
        sgsize = list()
        sgfreq = list()

        dgroups =  list()
        for i in range(0, len(cnts), 2):
            if g == i/2:
                n, s, f = cnts[i].strip().split('\t')
                dgroups = cnts[i + 1].strip().split('\t')

        tmp = list()
        stsize = 0
        stfreq = 0
        for item in dgroups:
            key = self.freqkeys[int(item)]
            isize = self.key_info[key][0]
            ifreq = self.key_info[key][1]
            if(stfreq + ifreq < favg):
                stfreq += ifreq
                stsize += isize
                tmp.append(item)
            else:
                sgroups.append(tmp)
                sgsize.append(stsize)
                sgfreq.append(stfreq)
                tmp = list()
                stsize = 0
                stfreq = 0
                stfreq += ifreq
                stsize += isize
                tmp.append(item)

        sgroups.append(tmp)
        sgsize.append(stsize)
        sgfreq.append(stfreq)
        # print(sgsize)
        # print(sgfreq)
        # print(sgroups)

        with open(source, 'w') as fout:
            for i in range(len(sgroups)):
                fout.write("%d\t%d\t%d\n" % (len(sgroups[i]), sgsize[i], sgfreq[i]))
                for item in sgroups[i]:
                    fout.write(str(item)+'\t')
                fout.write('\n')
    
    def find_overweighted_group(self):
        with open("{}/{}".format(self.filepath, self.parent_graph), 'r') as fin:
            cnts = fin.readlines()

        groups = list()

        for i in range(0, len(cnts), 2):
            n, s, f = cnts[i].strip().split('\t')
            tmp = cnts[i+1].strip().split('\t')
            groups.append([int(i) for i in tmp])

        for i in range(self.gropnum):
            groups[i].sort()
            #print(i, groups[i])

        total_size = 0
        total_freq = 0
        size_stat = list()
        freq_stat = list()
        group_len = list()

        for i in range(self.gropnum):
            size = 0
            freq = 0
            for node in groups[i]:
                key = self.freqkeys[node]
                size += self.key_info[key][0]
                freq += self.key_info[key][1]
            total_size += size
            total_freq += freq
            size_stat.append(size)
            freq_stat.append(freq)
            group_len.append(len(groups[i]))

        Favg = self.tfreq / self.nodenum
        # print("Favg =", Favg)
        # print("Total items =", sum(group_len))
        
        for j in range(5):
            divided_group = list()
            for i in range(self.gropnum):
                if freq_stat[i] > Favg:
                    print(i, freq_stat[i])
                    divided_group.append(i)
                    g = set(groups[i])
                    with open("{}/louvain{}_{}".format(self.filepath, self.traceno, i), 'w') as fout, open("{}/louvain{}".format(self.filepath, self.traceno), 'r') as fin:
                        line = fin.readline()
                        while line != "":
                            first, second, tmp = line.strip().split('\t')
                            if int(first) in g and int(second) in g:
                                fout.write(line)
                            line = fin.readline()
                else:
                    continue
        return divided_group
    
    def agg_file(self, group: list):
        tgroupf = self.parent_graph

        with open("{}/{}".format(self.filepath, self.parent_graph), 'r') as fin:
            cnts = fin.readlines()

        groups = list()
        for i in range(0, len(cnts), 2):
            n, s, f = cnts[i].strip().split('\t')
            no = int(i/2)
            if no in group:
                with open("{}/graph{}_{}_agg".format(self.filepath, self.traceno, no),'r') as fin:
                    cntss = fin.readlines()
                groups += cntss
            else:
                stri = "%s\t%s\t%s\n" % (n, s, f)
                groups.append(stri)
                i = i + 1
                groups.append(cnts[i])

        with open("{}/{}".format(self.filepath, self.parent_graph), 'w') as fout:
            for line in groups:
                fout.write(line)

    def get_group_num(self):
        with open("{}/{}".format(self.filepath, self.parent_graph), 'r') as fin:
            line_count = len(fin.readlines())

        print("line_count =",line_count)
        self.group_num = int(line_count/2)
        return self.group_num
        
    def clean_files(self):
        self.data["group_num"] = gd.get_group_num()
        with open(self.config_file, "w") as json_file:
            json.dump(self.data, json_file)
            
        # os.remove("{}/louvain{}".format(self.filepath, self.traceno))
        delete_files_with_prefix_or_suffix(self.filepath, prefix="louvain{}_".format(self.traceno))
        delete_files_with_prefix_or_suffix(self.filepath, suffix=".b")
        delete_files_with_prefix_or_suffix(self.filepath, suffix=".w")
        delete_files_with_prefix_or_suffix(self.filepath, suffix=".t")
        # delete_files_with_prefix_or_suffix(self.filepath, prefix="graph{}_*".format(self.traceno))
        # os.remove("{}/louvain{}_*".format(self.filepath, self.traceno))
        # os.remove("{}/*.b".format(self.filepath, self.traceno))
        # os.remove("{}/*.w".format(self.filepath, self.traceno))
        # os.remove("{}/*.t".format(self.filepath, self.traceno))
        # os.remove("{}/graph{}_*".format(self.filepath, self.traceno))
        
def delete_files_with_prefix_or_suffix(directory, prefix=None, suffix=None):
    """
    Delete files with a specific prefix or suffix in the specified directory.
    
    :param directory: Path to the target directory
    :param prefix: File name prefix (optional)
    :param suffix: File name suffix (optional)
    """
    if not os.path.exists(directory):
        print(f"The directory {directory} does not exist!")
        return

    if not os.path.isdir(directory):
        print(f"{directory} is not a directory!")
        return

    try:
        # Iterate through all files in the directory
        for filename in os.listdir(directory):
            file_path = os.path.join(directory, filename)
            # Check if the file matches the prefix or suffix and is a file
            if os.path.isfile(file_path) and (
                (prefix and filename.startswith(prefix)) or
                (suffix and filename.endswith(suffix))
            ):
                os.remove(file_path)  # Delete the file
                print(f"Deleted file: {file_path}")
    except Exception as e:
        print(f"Error while deleting files: {e}")
        
if __name__ == '__main__':
    gd = GroupDivision()
    gd.graph2group(False, 0)
    gd.merge_group(0, is_sub=False)
    for i in range(3):
        divided_group = gd.find_overweighted_group()
        print(divided_group)
        if(len(divided_group) == 0):
            break
        for g in divided_group:
            gd.graph2group(True, g)
            gd.merge_group(g, True)
        gd.agg_file(divided_group)
        
    if(len(divided_group) != 0):
        for g in divided_group:
            gd.split(g)
            
    gd.clean_files()