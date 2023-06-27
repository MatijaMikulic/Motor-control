clc
clear all
close all

load('data.mat')
rpm=data.data(:,1);
prbs=data.data(:,2);

%sampling time = 0.05 [s]
Ts = 0.05;

data_obj=iddata(rpm,prbs,Ts);
get(data_obj)
data_obj.InputName='PRBS';
data_obj.outputName='Rotation per minute';
data_obj.TimeUnit='seconds';

%700 data points will be used for model estimation
ze=data_obj(1:700);

figure(1)
plot(ze)

%making data zero mean
ze=detrend(ze)

figure(2)
plot(ze)

%estimate model
%model=arx(ze,[2 2 0]);
model = armax(ze,[2 2 0 1])

%validating model
figure(3)
zv=data_obj(1200:1600);
zv=detrend(zv);
set(gcf,'DefaultLegendLocation','best')
compare(zv,model)

%poles and zeros
figure(4)
h=iopzplot(model)

