from tsfresh.examples.har_dataset import download_har_dataset, load_har_dataset, load_har_classes
import seaborn as sns
from tsfresh import extract_features, extract_relevant_features, select_features
from tsfresh.utilities.dataframe_functions import impute
from tsfresh.feature_extraction import ComprehensiveFCParameters
import pandas as pd
import numpy as np
import ar


def tsfresh_featurs(df_objects, cols2extract):
    '''extract tsfresh features
    '''
    extraction_settings = ComprehensiveFCParameters()
    ids = df_objects['id'].unique()
    df_features = []
    for c in cols2extract:
        df = df_objects.loc[:, ['id', c]].copy()
        X = extract_features(df, column_id='id', impute_function=impute,
                             default_fc_parameters=extraction_settings, n_jobs=12)
        df_features.append(X)
    df_features = pd.concat(df_features, axis=1)
    df_features['activity_category_id'] = list(
        map(lambda x: df_objects[df_objects['id'] == x]['activity_category_id'].values[0], ids))
    df_features['segment_id'] = list(
        map(lambda x: df_objects[df_objects['id'] == x]['segment_id'].values[0], ids))
    df_features['object_id'] = ids
    return df_features


def make_pairs(elements):
    pairs = []
    for i in range(1, len(elements)):
        pairs.extend(list(zip(elements[i:], elements[:-i])))
    return pairs


def angle_feature(df_objects, cols2extract):
    def cosine(a, b):
        cos_angle = np.dot(a, b) / (np.sqrt(np.dot(a, a))
                                    * np.sqrt(np.dot(b, b)))
        return np.arccos(cos_angle)
    print(cols2extract)
    assert len(cols2extract) > 1, 'num of cols2extract < 2.'
    assert 'id' in df_objects, 'id missing.'

    pairs = make_pairs(cols2extract)
    cos_angle_list = []
    for name, group in df_objects.groupby('id'):
        cos_angles = [cosine(group[x], group[y]) for x, y in pairs]
        cos_angle_list.append(cos_angles)
    return pd.DataFrame(cos_angle_list, columns=[str(x) + '/' + str(y) for x, y in pairs])


# def iqrs_feature(df_objects, cols2extract):
#     df_features = []
#     for c in cols2extract:
#         df = df_objects.loc[:, ['id', c]].copy()
#         X = df_objects.groupby('id')[c].quantile(
#             q=0.75) - df_objects.groupby('id')[c].quantile(q=0.25)
#         df_features.append(X)
#     df_features = pd.concat(df_features, axis=1)
#     return pd.DataFrame({'ppg__iqrs': iqrs})


def ar_feature(df_objects):
    all_coefficients = []
    for obj_id in df_objects['id'].unique():
        obj = list(df_objects[df_objects['id'] ==
                              obj_id]['Resultant'].values)
        # print("=======")
        # print(obj)
        inputseries = ar.float_array(125)
        for i in range(125):
            inputseries[i] = float(obj[i])
        coefficients = ar.float_array(10)
        coefficients_list = []
        ar.AutoRegressionPy(inputseries, 125, 10, coefficients, 1)
        for i in range(10):
            coefficients_list.append(coefficients[i])
        # print(coefficients_list)
        all_coefficients.append(coefficients_list)
    all_coefficients = np.array(all_coefficients)
    print(all_coefficients.shape)
    df_ar = pd.DataFrame({})
    for i in range(10):
        # print("aaaa", all_coefficients[:, i])
        df_ar['Resultant__ar_{}'.format(i)] = all_coefficients[:, i]
    # print(df_ar)
    return df_ar


if __name__ == '__main__':
    '''
    usage:
    python3 ./extract_features.py --object_path /data/workspace/data/objects.csv \
                                  --feature_path /data/workspace/data/features.csv
    python3 ./extract_features.py --object_path /data/workspace/data/objects_hp_0.4hz.csv \
                                  --feature_path /data/workspace/data/features_hp_0.4hz.csv
    python3 ./extract_features.py --object_path /Users/liuziyi/Workspace/data/objects.csv \
                                  --feature_path /Users/liuziyi/Workspace/data/features.csv
    python3 ./extract_features.py --object_path /data/workspace/data/objects_ir_hp_0.4hz.csv \
                                  --feature_path /data/workspace/data/features_ir_hp_0.4hz.csv  
    python3 ./extract_features.py --object_path /data/workspace/data/objects_ir-resample_25hz-hp_0.4hz.csv \
                                  --feature_path /data/workspace/data/features_ir-resample_25hz-hp_0.4hz.csv
    python3 ./extract_features.py --object_path /data/workspace/data/objects_ir-resample_25hz.csv \
                                  --feature_path /data/workspace/data/features_ir-resample_25hz.csv
    python3 ./extract_features.py --object_path /data/workspace/data/objects_ir-resample_25hz_log2.csv \
                                  --feature_path /data/workspace/data/features_ir-resample_25hz_log2.csv
    python3 ./extract_features.py --object_path /data/workspace/data/objects_ir-resample_25hz_scale.csv \
                                  --feature_path /data/workspace/data/features_ir-resample_25hz_scale.csv
    python3 ./extract_features.py --object_path /data/workspace/data/step-count/df_objects__object_length_125__scale.csv \
                                  --feature_path /data/workspace/data/step-count/df_features__object_length_125__ar__scale.csv
    '''
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--object_path', type=str, help='')
    parser.add_argument('--feature_path', type=str, help='')
    args = parser.parse_args()

    df_objects = pd.read_csv(args.object_path)

    # ids = df_objects['id'].unique()
    # df_objects = df_objects[np.isin(df_objects['id'], ids[0:40000])]
    # df_objects = df_objects[df_objects['id'] < 50]
    # object_columns = list(df_objects.columns)

    df_tsfresh_featurs = tsfresh_featurs(
        df_objects, ['AccX', 'AccY', 'AccZ', 'Resultant'])  # tsfresh特征

    # cols2angle = ['Accelerometer X', 'Accelerometer Y', 'Accelerometer Z']
    # df_angle_features = angle_feature(df_objects, cols2angle)

    # df_iqrs = iqrs_feature(df_objects, ['ppg'])

    df_ar = ar_feature(df_objects)

    df_features = pd.concat([df_tsfresh_featurs, df_ar], axis=1)
    # df_tsfresh_featurs.to_csv(args.feature_path)
    df_features.to_csv(args.feature_path)
    print('Done.')
