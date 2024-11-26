with open("louvain_node_34", 'r') as fin, open('louvain_data_34', 'w') as fout:
    line = fin.readline()
    while(line != ""):
        cnts = int(line.strip().split('\t')[2])
        if(cnts != 0):
            fout.write(line)
        line = fin.readline()
