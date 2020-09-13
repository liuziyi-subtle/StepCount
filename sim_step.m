clear; clc; close all
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/fastwalk_o95_h106.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/fastrun_o103_h105.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/walk_stop_O71_H110.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/watch_O29_H101.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/slowwalk_o101_h86.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/hold_phone_100.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/upstairs_100.csv');


% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/LYX_20200421/LYX_24_插衣兜_上坡200步.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/work.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/5_LJF/LJF_13_摆臂_跑步200步快.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/old/bike2.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/Car/zwx_car_56_steps.csv');
% acc = dlmread('~/Desktop/手环震动_0_steps3.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/手环震动_0_steps.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/LYX/LYX_1_插兜裤子上坡走路200.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/guobiao_zero1.csv');
% acc = dlmread('~/Downloads/walk/walk_0_steps4.csv');
% acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/new/Bus/bus3.csv');
acc = dlmread('~/Desktop/Lifesense/Gitlab/Step_algorithm/data/Benchmark/StepCounting/ML_20200610/ML_01_跑步机正常摆臂_走路200步慢.csv');

b1 = typecast(uint16(acc(:, 1)), 'int16');
b2 = typecast(uint16(acc(:, 2)), 'int16');
b3 = typecast(uint16(acc(:, 3)), 'int16');
acc = double([b1 b2 b3]);

N = 4;
b = ones(N, 1) / N;
acc_f  = filter(b, 1, floor(acc / 64));
m = acc_f .^ 2;
m_f = filter(b, 1, sum(m ,2));

index = dlmread('c/result.txt');
h(1) = subplot(211);
plot(acc_f)
hold on
% X
v_x = index(index(:, end) == 4, :);
p_x = index(index(:, end) == 5, :);
plot(v_x(:, 2), v_x(:, 3), 'kx')
plot(p_x(:, 2), p_x(:, 3), 'ro')

% Y
v_y = index(index(:, end) == 6, :);
p_y = index(index(:, end) == 7, :);
plot(v_y(:, 2), v_y(:, 3), 'kx')
plot(p_y(:, 2), p_y(:, 3), 'ro')

% Z
v_z = index(index(:, end) == 8, :);
p_z = index(index(:, end) == 9, :);
plot(v_z(:, 2), v_z(:, 3), 'kx')
plot(p_z(:, 2), p_z(:, 3), 'ro')
hold off
vline(index(index(:, end) == -1, 1))
legend('x', 'y', 'z')
title(['Nx = ', num2str(length(v_x)), '  Ny = ', num2str(length(v_y)), '  Nz = ', num2str(length(v_z))]);

h(2) = subplot(212);
plot(m_f)
hold on
plot(index(index(:, end) == 1, 2), index(index(:, end) == 1, 3), 'o')
plot(index(index(:, end) == 0, 2), index(index(:, end) == 0, 3), 'kx')
vline(index(index(:, end) == -1, 1))
title(['N = ', num2str(sum(index(:, end) == 1))])
hold off
linkaxes(h, 'x')