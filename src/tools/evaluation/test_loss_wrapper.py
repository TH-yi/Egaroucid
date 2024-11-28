import subprocess

''' # 7.0
N_PHASES = 60
data_root_dir = './../../../train_data/bin_data/20240223_1/'
exe = 'test_loss.out'
eval_file = 'trained/eval.egev'
data_nums = []
for _ in range(12):
    data_nums.append([36])
for _ in range(12, 61):
    data_nums.append([38])
'''
''' # 7.1 not used
N_PHASES = 60
data_root_dir = './../../../train_data/bin_data/20240525_1/'
exe = 'test_loss_20240525_1.out'
eval_file = 'trained/eval.egev'
data_nums = []
for _ in range(12):
    data_nums.append([36])
for _ in range(12, 61):
    data_nums.append([38])
'''
''' # 7.0 light
N_PHASES = 60
data_root_dir = './../../../train_data/bin_data/20240622_1_7_0_light/'
exe = 'test_loss_20240622_1_7_0_light.out'
eval_file = 'trained/eval.egev'
data_nums = []
for _ in range(12):
    data_nums.append([36])
for _ in range(12, 61):
    data_nums.append([38])
'''

''' # 7.4
N_PHASES = 60
data_root_dir = './../../../train_data/bin_data/20240925_1/'
exe = 'test_loss_20240925_1_7_4.out'
eval_file = 'trained/eval.egev'
data_nums = []
for _ in range(12):
    data_nums.append([36])
for _ in range(12, 61):
    data_nums.append([38])
#'''
#''' # 7.5
N_PHASES = 60
data_root_dir = './../../../train_data/bin_data/20241125_1/'
exe = 'test_loss_20241125_1_7_5.out'
eval_file = 'trained/eval.egev'
data_nums = []
for _ in range(12):
    data_nums.append([36])
for _ in range(12, 61):
    data_nums.append([38])
#'''

res = ''
for phase in range(N_PHASES):
    cmd = exe + ' ' + eval_file + ' ' + str(phase)
    for num in data_nums[phase]:
        cmd += ' ' + data_root_dir + str(phase) + '/' + str(num) + '.dat'
    print(cmd)
    p = subprocess.run(cmd, stdout=subprocess.PIPE)
    out = p.stdout.decode().replace('\r', '').replace('\n', '')
    res += out + '\n'
print('')
print('all done')
print(res)
with open('trained/test.txt', 'w') as f:
    f.write(res)