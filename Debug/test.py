import numpy as np
import pandas as pd
import c2py
from glob import glob

# manual_infos = pd.read_csv("/Users/liuziyi/manual_infos_435.csv")
file_paths = glob(
    "/Users/liuziyi/Documents/Lifesense/Data/StepCount/results-new-new-new/records/*.csv")
for fp in file_paths:
    if "余嘉敏" not in fp.split("/")[-1]:
        continue

    record = pd.read_csv(fp,
                         header=None,
                         error_bad_lines=False,
                         warn_bad_lines=False,
                         verbose=False)
    record = record.loc[record[0] != "APP_MSG"]
    record[0] = record[0].astype(np.int32)
    accs = record.loc[record[0] == 2, [1, 2, 3]].values.astype(np.int16)

    import matplotlib.pyplot as plt
    fig, axes = plt.subplots(3, 1)
    axes[0].plot(np.array(accs[:, 0], np.int16) >> 6)
    axes[1].plot(np.array(accs[:, 1], np.int16) >> 6)
    axes[2].plot(np.array(accs[:, 2], np.int16) >> 6)
    plt.show()

    steps = 0
    c2py.Step_Init()
    for acc in accs:
        # print("Py: ", tuple(acc))
        step = c2py.Step_Detection(tuple(acc))
        steps += step

    # manual_info = manual_infos.loc[manual_infos["file_name"] == fp.split(
        # "/")[-1].split(".dat")[0], ["手环步数", "计步器步数"]]
    print(fp.split("/")[-1], ": ", steps)
