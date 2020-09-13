# -*- coding: utf-8 -*-

import numpy as np
import os
import json
import pandas as pd
from datetime import datetime
import time
from scipy import signal


object_id_iter = iter(range(10000000))


# def resample(record):
#     n_sample = int(record.shape[0] / 4)
#     record = record.iloc[::4, :]
#     return record


def resample(record):
    '''简单降采样
       TODO: 需要基于最小公倍数作采样
    '''
    record = record.iloc[::2, :]
    return record


def hp(record):
    sos = signal.butter(4, 0.4, 'high', fs=25, output='sos')
    record['ppg'] = signal.sosfilt(sos, record['ppg'])
    record = record[200:]
    return record


def log_transform(record):
    # print('before log_transform: ', list(record['ppg'][1000:1064]))
    record['ppg'] = np.around(np.log2(np.abs(record['ppg']) + 1), decimals=6)
    # print('after log_transform: ', list(record['ppg'][1000:1064]))
    return record


def scale(data):
    data = (data - 5000000) / 1000.0
    return data


preprocess_func_map = {'resample': resample,
                       'hp': hp,
                       'log_transform': log_transform,
                       'scale': scale}


def local2utc(localtime):
    localtime = datetime.strptime(localtime, "%Y-%m-%d %H:%M:%S")
    utc = time.mktime(localtime.timetuple())
    return utc


def parse_annotation(annotations):
    annotation_index = {}
    # record_id: segment键值对
    for segment_annotation in annotations['segment_annotations']:
        record_id = segment_annotation['record_id']
        if record_id not in annotation_index:
            annotation_index[record_id] = []
        annotation_index[record_id].append(segment_annotation)
    return annotation_index


def replace_timestamp(timestamp):
    return timestamp.replace('T', ' ').split('.')[0]


def segment2object(segment, object_length):
    objects = []
    for i in range(0, segment.shape[0] - object_length, int(object_length / 2)):
        object = segment.iloc[i:i+object_length, :].copy()
        # object.rename(columns={'id': 'segment_id'}, inplace=True)
        object['id'] = next(object_id_iter)
        objects.append(object)
    return objects


def create_object(record_annotation,
                  annotation_list,
                  value_categories,
                  record_dir,
                  object_length=256,
                  target_fs=25,
                  preprocess_func_s=None):
    for value_category in value_categories:
        if value_category['id'] == annotation_list[0]['value_category_id']:
            break
    record = pd.read_csv(os.path.join(record_dir, record_annotation['name']),
                         header=None, error_bad_lines=False)
    record = record.loc[record[0] == value_category['id'], [1, 2, 3]]
    print(record)
    record.rename(columns={1: 'AccX', 2: 'AccY', 3: 'AccZ'}, inplace=True)

    print(os.path.join(record_dir, record_annotation['name']))
    # record = pd.read_csv(os.path.join(
    #     record_dir, record_annotation['name']))  # , engine='python'
    # record['AccX'] = record['AccX'].values >> 6
    # record['AccY'] = record['AccY'].values >> 6
    # record['AccZ'] = record['AccZ'].values >> 6
    record['AccX'] = np.array(record['AccX'], np.int16) >> 6
    record['AccY'] = np.array(record['AccY'], np.int16) >> 6
    record['AccZ'] = np.array(record['AccZ'], np.int16) >> 6
    record['Resultant'] = np.power(
        record['AccX'], 2) + np.power(record['AccY'], 2) + np.power(record['AccZ'], 2)

    # record.rename(columns={0: 'AccX', 1: 'AccY',
    #                        2: 'AccZ', 3: 'Resultant'}, inplace=True)
    # record = record[['ppg']]
    # print(record.columns)
    # record = record['ppg']

    # preprocess
    # if preprocess_func_s:
    #     print(preprocess_func_s)
    #     for pfs in preprocess_func_s.split(','):
    #         record['ppg'] = preprocess_func_map[pfs](record.values)

    object_list = []
    for segment_annotation in annotation_list:
        if segment_annotation['activity_category_id'] == 1:
            print(segment_annotation)
        start = int(segment_annotation['start'])
        end = int(segment_annotation['end'])
        segment = record.iloc[start:end].copy()
        segment['activity_category_id'] = segment_annotation['activity_category_id']
        segment['record_id'] = str(segment_annotation['record_id'])
        segment['segment_id'] = segment_annotation['id']
        objects = segment2object(segment, object_length)
        # print(objects)
        object_list.extend(objects)
    return object_list


if __name__ == '__main__':
    '''
    usage:
    python3 ./create_dataset.py --annotations_path /data/workspace/data/step-count/annotations.json \
                                --record_dir /data/workspace/data/step-count/results/records \
                                --object_length 50 \
                                --save_path /data/workspace/data/step-count/df_objects.csv
    python3 ./create_dataset.py --annotations_path /Users/liuziyi/Documents/Lifesense/data/step-count/results/annotations.json \
                                --record_dir /Users/liuziyi/Documents/Lifesense/data/step-count/results/records \
                                --object_length 125 \
                                --save_path /Users/liuziyi/Documents/Lifesense/data/step-count/results/df_objects__object_length_125__rightshift_6.csv
    '''
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--annotations_path', type=str, help='')
    # parser.add_argument('--value_descriptions_path', type=str, help='')
    parser.add_argument('--record_dir', type=str, help='')
    parser.add_argument('--object_length', type=int, default=256, help='')
    parser.add_argument('--preprocess_func_s', type=str, default=None, help='')
    parser.add_argument('--save_path', type=str, help='')
    args = parser.parse_args()

    with open(args.annotations_path, 'r') as f:
        annotations = json.load(f)

    annotation_index = parse_annotation(annotations)
    record_annotations = annotations['record_annotations']

    # value_categories = annotations['value_categories']

    object_list = []
    for record_annotation in record_annotations:
        if record_annotation['id'] not in annotation_index:
            continue
        annotation_list = annotation_index[record_annotation['id']]
        print(record_annotation)
        objects = create_object(
            record_annotation,
            annotation_list,
            annotations['value_categories'],
            args.record_dir,
            object_length=args.object_length,
            target_fs=25,
            preprocess_func_s=args.preprocess_func_s)
        object_list.extend(objects)
    df_objects = pd.concat(object_list)
    df_objects.to_csv(args.save_path, index=False)
    print("Done.")
