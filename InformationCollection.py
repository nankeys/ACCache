import os
import json

def get_input(promt, default):
    value = input(promt + f" (default: {default}): \n")
    if value == '':
        return default
    else:
        return value
    
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
    return data

if __name__ == '__main__':
    config_file = 'src/config.json'
    
    print("="*50)
    print("Config Information Modification\n".center(50))
    print("Please select the config information you want to change.".center(50))
    print("1. Trace type".center(50))
    print("2. Trace No.".center(50))
    print("3. Path".center(50))
    print("4. Sliding window size during correlation analysis".center(50))
    print("5. i-th day for testing".center(50))
    print("6. The number of Memcached servers".center(50))
    print("7. Information of Memcached servers".center(50))
    print("="*50)
    print('')
    
    choice = int(get_input("Input the parameter you want to change", 0))
    
    if(choice != 0):
        data = read_json_file(config_file)
    
    if(choice == 1):
        trace_type = get_input('The type of trace, namely twitter or meta.', data["trace_type"])
        if(trace_type not in ['twitter', 'meta']):
            print('Error trace type, exiting...')
            exit(-1)
        data["trace_type"] = trace_type
    elif(choice == 2):
        traceno = int(get_input('The Trace No. you want to test.', data["trace_no"]))
        data["trace_no"] = traceno
    elif(choice == 3):
        path = get_input('The path to intermediate files.', data["path_prefix"])
        data["path_prefix"] = path
    elif(choice == 4):
        window_size = int(get_input('The sliding window size during correlation analysis.', data["window_size"]))
        data["window_size"] = window_size
    elif(choice == 5):
        day = int(get_input('i-th day for testing.', data["day"]))
        data["day"] = day
    elif(choice == 6):
        server_num = int(get_input('The number of Memcached servers.', data["server_num"]))
        data["server_num"] = server_num
    elif(choice == 7):
        data["server_info"].clear()
        for i in range(data["server_num"]):
            tmp = dict()
            while(True):
                ip = get_input('The IP address of the {}-th Memcached servers.'.format(i), '')
                if(ip == ''):
                    print('Wrong IP address. Retrying')
                else:
                    break
            port = int(get_input('The port of {} of the {}-th Memcached servers.'.format(ip, i), 11211))
            tmp['ip'] = ip
            tmp['port'] = port
            data["server_info"].append(tmp)
            
    if(choice != 0):
        data = read_json_file(config_file)
        with open(config_file, "w") as json_file:
            json.dump(data, json_file)