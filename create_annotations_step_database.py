# coding:utf-8

from matplotlib.font_manager import FontProperties
from matplotlib.widgets import RectangleSelector
import numpy as np
import matplotlib.pyplot as plt
import json
from glob import glob
import os
import pandas as pd
from datetime import datetime
import shutil

from matplotlib.font_manager import FontProperties
font = FontProperties(fname='/System/Library/Fonts/STHeiti Light.ttc')

SEGMENT_ID_GENERATOR = iter(range(1000000))

RECORD_MOVE_DIR = None
REFERENCE_MOVE_DIR = None
ANNOTATION_SAVE_DIR = None
VALUE_DESCRIPTIONS = None

# global ACTIVITY
# global NON_ACTIVITY
# ACTIVITY = 0
# NON_ACTIVITY = 0


def str2id(s):
    return int.from_bytes(s.encode('utf-8'), 'little')


def line_select_callback(eclick, erelease):
    'eclick and erelease are the press and release events'
    x1, y1 = eclick.xdata, eclick.ydata
    x2, y2 = erelease.xdata, erelease.ydata
    print("(%3.2f, %3.2f) --> (%3.2f, %3.2f)" % (x1, y1, x2, y2))
    print(" The button you used were: %s %s" %
          (eclick.button, erelease.button))
    return eclick, erelease


def toggle_selector(event):
    print(' Key pressed.')
    if event.key in ['Q', 'q'] and toggle_selector.RS.active:
        print(' RectangleSelector deactivated.')
        toggle_selector.RS.set_active(False)
    if event.key in ['A', 'a'] and not toggle_selector.RS.active:
        print(' RectangleSelector activated.')
        toggle_selector.RS.set_active(True)
    if event.key in ['E', 'e']:
        # start and end
        toggle_selector.dict_segment['start'] = max(
            int(toggle_selector.RS.corners[0][0]), 0)
        toggle_selector.dict_segment['end'] = min(
            int(toggle_selector.RS.corners[0][2]), toggle_selector.length)
        # segment id
        toggle_selector.dict_segment['id'] = next(
            SEGMENT_ID_GENERATOR) + int(datetime.now().timestamp())
        # value category id
        toggle_selector.dict_segment['value_category_id'] = \
            toggle_selector.value_category_id
        # activity id
        toggle_selector.dict_segment['activity_category_id'] = \
            toggle_selector.activity_category_id
        print("x-> 保存!")
    if event.key in ['X', 'x']:
        json_dict = {'record': toggle_selector.dict_record,
                     'segment': toggle_selector.dict_segment}
        json_dict = json.dumps(json_dict, ensure_ascii=False)  # 确保中文文件名编码
        json_name = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        json_name = json_name.replace(' ', '(') + ').json'
        json_path = os.path.join(ANNOTATION_SAVE_DIR, json_name)
        with open(json_path, 'w') as f:
            f.write(json_dict)


def annotate_segment(record_path, manual_info, toggle_selector):
    record = np.loadtxt(record_path, delimiter=' ', dtype=np.int32)

    # 将record汇总到records文件夹中.
    record = pd.DataFrame({'AccX': record[:, 0],
                           'AccY': record[:, 1],
                           'AccZ': record[:, 2]})
    record.to_csv(os.path.join(RECORD_MOVE_DIR, manual_info['file_name']),
                  index=False)

    # 标注矩形框
    toggle_selector.value_category_id = 2
    toggle_selector.activity_category_id = 0 if 'StepCounting' in record_path else 1
    toggle_selector.manual_info = manual_info
    toggle_selector.dict_record = annotate_record(manual_info)
    toggle_selector.length = len(record)
    toggle_selector.dict_segment = {
        'record_id': toggle_selector.dict_record['id']}

    # plot data
    fig, ax_arr = plt.subplots(3, 1, sharex=True, figsize=(15, 6))

    ax_arr[0].set_title(record_path)

    ax_arr[0].plot(record['AccX'], 'r')
    ax_arr[0].set_xlim(0, toggle_selector.length - 1)
    ax_arr[0].set_ylabel('AccX')

    ax_arr[1].plot(record['AccY'], 'b')
    ax_arr[1].set_xlim(0, toggle_selector.length - 1)
    ax_arr[1].set_ylabel('AccY')

    ax_arr[2].plot(record['AccZ'], 'g')
    ax_arr[2].set_xlim(0, toggle_selector.length - 1)
    ax_arr[2].set_ylabel('AccZ')

    toggle_selector.RS = RectangleSelector(ax_arr[0], line_select_callback,
                                           drawtype='box', useblit=True,
                                           # don't use middle button
                                           button=[1, 3],
                                           minspanx=5, minspany=5,
                                           spancoords='pixels',
                                           interactive=True)

    plt.connect('key_press_event', toggle_selector)
    plt.show()


def annotate_record(manual_info):
    return dict({'name': manual_info['file_name'],
                 'id': str2id(manual_info['file_name']),
                 'creation_date': manual_info['creation_date']})


RECORD_MOVE_DIR = \
    '/Users/liuziyi/Documents/Lifesense/data/step-count/results-new/records'
ANNOTATION_SAVE_DIR = \
    '/Users/liuziyi/Documents/Lifesense/data/step-count/results-new/jsons'
with open('/Users/liuziyi/Documents/Lifesense/data/step-count/raw/value_descriptions.json') as f:
    VALUE_DESCRIPTIONS = json.load(f)

record_paths = glob(
    '/Users/liuziyi/Documents/Lifesense/data/step-count/step_database-new/**/*.csv', recursive=True)
record_paths.sort()
print(len(record_paths))

start = 0
for rp in record_paths:
    # if rp == '/Users/liuziyi/Documents/Lifesense/data/step-count/raw/walk/5_LJF/LJF_10_摆臂_走路200步快.csv':
    #     start = 1
    # if start != 1:
    #     continue
    if ('raw' in rp) or ('Raw' in rp):
        continue
    manual_info = {
        'file_name': rp.split('/')[-1],
        'creation_date': '2020/5/22'
    }

    print(rp)

    annotate_segment(rp, manual_info, toggle_selector)

    finish = input("继续下一个样本: y/n")

# print(NON_ACTIVITY, ACTIVITY)
