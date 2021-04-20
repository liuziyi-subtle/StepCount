'''Aggregate json annotations into an integrated json file.
'''
from glob import glob
import json
import pandas as pd


def set_dict(dict_list):
    ''' return a list containing unique dicts
    '''
    unique_dicts = []
    for d in dict_list:
        if d not in unique_dicts:
            unique_dicts.append(d)
        else:
            continue
    return unique_dicts


def rename_segment_id(segment_annotation):
    ''' reassign simple id to segment_annotation
    '''
    segment_annotation['id'] = next(rename_segment_id.ITER)


if __name__ == '__main__':
    '''
    usage:
    python3 aggregate_annotations.py --json_dir /Users/liuziyi/Documents/Lifesense/Data/StepCount/results/jsons \
                                     --record_field record \
                                     --segment_field segment \
                                     --value_description_path /Users/liuziyi/Documents/Lifesense/Data/StepCount/raw/value_descriptions.json \
                                     --save_path /Users/liuziyi/Documents/Lifesense/Data/StepCount/results/annotations.json
    '''
    import argparse
    import os
    parser = argparse.ArgumentParser()
    parser.add_argument('--json_dir', type=str, help='')
    parser.add_argument('--record_field', type=str, help='')
    parser.add_argument('--segment_field', type=str, help='')
    parser.add_argument('--value_description_path', type=str, help='')
    parser.add_argument('--save_path', type=str, help='')

    args = parser.parse_args()
    json_paths = glob(os.path.join(args.json_dir, '*.json'))
    record_annotations = []
    segment_annotations = []
    category_annotations = []
    # rename_segment_id.ITER = iter(range(1000000))
    for jp in json_paths:
        with open(jp, 'r') as f:
            annotation = json.load(f)
            record_annotations.append(annotation[args.record_field])
            segment_annotations.append(annotation[args.segment_field])
    record_annotations = set_dict(record_annotations)

    with open(args.value_description_path, 'r') as f:
        value_descriptions = json.load(f)

    annotations = {'record_annotations': record_annotations,
                   'segment_annotations': segment_annotations,
                   'value_categories': value_descriptions['value_categories']
                   }
    annotations = json.dumps(annotations, ensure_ascii=False)
    with open(args.save_path, 'w') as f:
        f.write(annotations)
        print("Done.")
