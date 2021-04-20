import numpy as np
import pandas as pd
import c2py
from glob import glob

manual_infos = pd.read_csv("/Users/liuziyi/manual_infos_435.csv")
file_paths = glob("/Users/liuziyi/计步数据采集/435/dat/decrypt_files/*.dat")

filenames = [
    "心率-A01_日常_静坐-余嘉敏-手_3-20210317101857-F01C394A348D",
    "心率-A01_日常_静坐-余嘉敏-手_4-20210317102214-F01C394A348D",
    "心率-A01_日常_静坐-余嘉敏-手_5-20210317102606-F01C394A348D",
    "心率-A01_日常_静坐-余嘉敏-手_6-20210317103836-F01C394A348D",
    "心率-A06_运动_行走_室外_平地-李晨阳-左手-20210318172121-F01C394A348D",
    "心率-A06_运动_行走_室外_平地-罗惠文-左手-20210318154712-C6A03D904920",
    "心率-A06_运动_行走_室外_平地-伍远方-左手-20210318171253-F01C394A348D",
    "心率-A06_运动_行走_室外_平地-何荣荣-左手-20210318170945-C80F3D908534",
    "心率-A07_运动_行走_室外_上下楼梯-李晨阳-左手-20210318172857-F01C394A348D",
    "心率-A06_运动_行走_室外_平地-李琴-左手-20210319111657-C6A03D904920",
    "心率-A06_运动_行走_室外_平地-何荣荣-左手-20210319111653-C38C3D904935",
    "心率-A06_运动_行走_室外_平地-张城-左手-20210319144016-C6A03D904920",
    "心率-A06_运动_行走_室外_平地-陈维洁-左手-20210319145351-F01C394A348D"
]

filedir = "/Users/liuziyi/计步数据采集/437/dat/decrypt_files/"
filepaths = [filedir + fn + ".dat" for fn in filenames]

savedir = "/Users/liuziyi/Documents/Lifesense/Data/StepCount/raw/augmented/20210420"

for fp in filepaths:
    record = pd.read_csv(fp,
                         header=None,
                         error_bad_lines=False,
                         warn_bad_lines=False,
                         verbose=False)
    record = record.loc[record[0] != "APP_MSG"]
    record[0] = record[0].astype(np.int32)
    accs = record.loc[record[0] == 2, [1, 2, 3]].values.astype(np.int16) << 3
    index = np.ones((len(accs), 1)) * 2
    record_acc = pd.DataFrame(np.c_[index, accs])
    record_acc.to_csv(savedir + "/" + fp.split("/")
                      [-1].replace(".dat", ".csv"), index=None, header=None)
