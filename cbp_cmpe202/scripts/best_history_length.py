
import random
import subprocess
import re

rounds = 1
ameans = [0] * 100
for k in range(1, 100):
    history_length = [0] * 16
    history_length[1] = random.randint(0, 15)
    history_length[2] = random.randint(history_length[1] + 1, 17)
    history_length[3] = random.randint(history_length[2] + 1, 31)
    history_length[4] = random.randint(history_length[3] + 1, 38)
    history_length[5] = random.randint(history_length[4] + 1, 63)
    history_length[6] = random.randint(history_length[5] + 1, 90)
    history_length[7] = random.randint(history_length[6] + 1, 128)
    history_length[8] = random.randint(history_length[7] + 1, 180)
    history_length[9] = random.randint(history_length[8] + 1, 256)
    history_length[10] = random.randint(history_length[9] + 1, 388)
    history_length[11] = random.randint(history_length[10] + 1, 512)
    history_length[12] = random.randint(history_length[11] + 1, 666)
    history_length[13] = random.randint(history_length[12] + 1, 888)

    history_length[14] = random.randint(history_length[13] + 1, 1023)
    history_length[15] = random.randint(history_length[14] + 1, 2000)

    for i in range(0, len(history_length)):
        history_length[i] = str(history_length[i])

    #args = ['/home/yzhu29/cmpe202/test_cpp/a.out', history_length[1], history_length[2],
    #        history_length[3],history_length[4],history_length[5],history_length[6],
    #        history_length[7],history_length[8],history_length[9],history_length[10],
    #        history_length[11],history_length[12],history_length[13],history_length[14],
    #        history_length[15]]

    #cmd = ' '.join(args)
    #subprocess.call(cmd, shell = True)

    num_parallel_jobs=8
    output_dir = '../results/new_traces' + str(rounds)
    args = ['./runall.pl', '-s' , '../sim/predictor', '-w', 'cmpe202', '-f',
            str(num_parallel_jobs), '-d' , output_dir, '-randomize',
            history_length[1], history_length[2], history_length[3], history_length[4],
            history_length[5], history_length[6], history_length[7], history_length[8],
            history_length[9], history_length[10], history_length[11], history_length[12],
            history_length[13], history_length[14], history_length[15] ]

    #./runall.pl -s ../sim/predictor -w cmpe202 -f  $num_parallel_jobs -d ../results/new_traces
    cmd = ' '.join(args)
    print(cmd)
    subprocess.call(cmd, shell = True)

    args = ['./getdata.pl', '-w', 'cmpe202', '-d', output_dir]
    cmd = ' '.join(args)
    print(cmd)
    process = subprocess.run(cmd, shell = True, stdout=subprocess.PIPE)
    text = process.stdout.decode('utf-8')
    text = text.splitlines()
    #print(text)

    fh = open("results", "a")
    fh.write(', '.join(history_length) + '\n')

    for line in text:
        matchObj = re.match(r'AMEAN(.*)', line)
        if matchObj:
            amean_ = re.findall('[\d\.]+', line)
            amean = amean_[0]
            ameans[k] = float(amean)
            fh.write( 'Amean is: ' + amean + '\n')
    #print(process.communicate())
    fh.close()

    rounds+=1
min_amean = min(ameans)
print("Best Amean is : " + str(min_amean))
fh = open("results", "a")
fh.write("Best Amean is : " + str(min_amean))
fh.close()
