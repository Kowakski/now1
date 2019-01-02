'''
#用来生成<label> <index1>:<value1> <index2>:<value2> ...格式的，可以用于libsvm训练的数据
'''
import ast
import math
import numpy as np
import pdb
from svmutil import *

model_path = './raw_trained.model'
data_file = '../../WISDM_ar_v1.1_raw.txt'

FEATURE = ("mean", "max", "min", "std")
STATUS  = ("Sitting", "Walking", "Upstairs", "Downstairs", "Jogging", "Standing")

def preprocess(file_dir, Seg_granularity):
    gravity_data = []
    model = svm_load_model(model_path)
    with open(file_dir) as f:
        index = 0
#组合xyz三个轴
        for line in f:      #这个for循环就是为了把组合值算出来
            clear_line = line.strip().lstrip().rstrip(';')
            raw_list = clear_line.split(',')
            # print( raw_list )
            index = index + 1
            if len(raw_list) < 5:
                continue
            status  = raw_list[1]          #运动类别
            acc_x = float(raw_list[3])     #x轴
            acc_y = float(raw_list[4])     #y轴
            # print( index )
            acc_z = float(raw_list[5])     #z轴

            if acc_x == 0 or acc_y == 0 or acc_z == 0:
                continue

            gravity = math.sqrt(math.pow(acc_x, 2)+math.pow(acc_y, 2)+math.pow(acc_z, 2))     #组合值
            gravity_tuple = {"gravity": gravity, "status": status}
            gravity_data.append(gravity_tuple)

    # split data sample of gravity
    splited_data = []
    cur_cluster  = []
    counter      = 0
    last_status  = gravity_data[0]["status"]

#100组或者按照类别区分，划分成一组一组的值
    for gravity_tuple in gravity_data:         #每100组数据，或者不满100组但是类别不同了必须要结束这个buffer了
        # if not (counter < Seg_granularity and gravity_tuple["status"] == last_status):
        if not ( counter < Seg_granularity ):
            seg_data = {"status": last_status, "values": cur_cluster}
            # print seg_data
            splited_data.append(seg_data)
            cur_cluster = []
            counter = 0
        cur_cluster.append(gravity_tuple["gravity"])
        last_status = gravity_tuple["status"]
        counter += 1

#从前面划分的一组一组的值里面提取特征
    # compute statistics of gravity data
    statistics_data = []
    # pdb.set_trace(  )
    for seg_data in splited_data:    #在100个值里面，或者不满100( 数据是连续的，后面一个不一样了不能存一起 )
        np_values = np.array(seg_data.pop("values"))
        maxx = np.amax(np_values)
        minx  = np.amin(np_values)
        std  = np.std(np_values)
        mean = np.mean(np_values)
        feature_vector = {FEATURE.index('mean'):mean, FEATURE.index("max"):maxx, FEATURE.index('min'):minx, FEATURE.index("std"):std}
        p_labs, p_acc, p_vals = svm_predict([], [feature_vector], model)
        print(p_labs, feature_vector)

if __name__ == "__main__":
    preprocess(data_file, 100)
    pass