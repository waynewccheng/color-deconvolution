% script for training 
% okay, first of all, find out how many images for 1 svs
addpath(pwd); %close all;

workdir = '/Users/lun5/Research/color_deconvolution';
datadir = fullfile(workdir, 'TissueImages');

source_svs = 'tp10-867-1';
purple_source = load(fullfile(workdir,'results','140806',[source_svs 'training_purple.mat']));
training_data_purple_source =purple_source.training_data_purple;
pink_source = load(fullfile(workdir,'results','140806',[source_svs 'training_pink.mat']));
training_data_pink_source = pink_source.training_data_pink;
 %[ training_data_purple_source, training_data_pink_source] = wsi_get_training( workdir, source_imname);
target_svs = 'tp10-611';
%[ training_data_purple_target, training_data_pink_target] = wsi_get_training( workdir, target_imname);
purple_source = load(fullfile(workdir,'results','140806',[target_svs 'training_purple.mat']));
training_data_purple_target =purple_source.training_data_purple;
pink_source = load(fullfile(workdir,'results','140806',[target_svs 'training_pink.mat']));
training_data_pink_target = pink_source.training_data_pink;

%% get the rotation matrix 
% source image
training_data = [training_data_purple_source(:,1:2000) training_data_pink_source(:,1:8000)];
[U,~,~] = svd(training_data,0);
rotation_matrix_source = [-U(:,1) U(:,2:3)]';
% target image
training_data = [training_data_purple_target(:,1:2000) training_data_pink_target(:,1:8000)];
[U,~,~] = svd(training_data,0);
rotation_matrix_target = [-U(:,1) U(:,2:3)]';

%% calculate the OC coordinates for both images
% actually what images?
% need that as well

% original
%target_imname = 'tp10-611_22528_14336_2048_2048.tif';
source_imname = 'tp10-867-1_47104_22528_2048_2048.tif';
% test out
%target_imname = 'tp10-611_59392_10240_2048_2048.tif';
%target_imname = 'tp10-611_67584_10240_2048_2048.tif';
%target_imname = 'tp10-611_59392_18432_2048_2048.tif';
target_imname = 'tp10-611_53248_12288_2048_2048.tif';
%target_imname = 'tp10-611_61440_14336_2048_2048.tif';
%source_imname = 'tp10-867-1_26624_22528_2048_2048.tif';
% read in the image

% get the names of the files
target_im = imread(fullfile(datadir,target_imname));
target_rgb = raw2rgb(target_im);
figure;imshow(target_im);
source_im = imread(fullfile(datadir,source_imname));
source_rgb = raw2rgb(source_im);
%figure;imshow(source_im);
%options = struct('Normalize','off');
%mu_s = 4; sigma_s = 2; % values for normalization
[ target_oppCol, target_brightness, target_theta, target_rotated] = rgb2oppCol( target_rgb, rotation_matrix_target); 
[ source_oppCol, source_brightness, source_theta, source_rotated] = rgb2oppCol( source_rgb, rotation_matrix_source); 

% plot the opponent color space
% apply the classifier on images
nstep = 100;
% h1 = figure;
% scatter(target_oppCol(1,1:nstep:end),target_oppCol(2,1:nstep:end),20,target_rgb(:,1:nstep:end)'./255,'filled');
% axis([-1 1 -1 1]); axis square
% 
% 
% h2 = figure;
% scatter(source_oppCol(1,1:nstep:end),source_oppCol(2,1:nstep:end),20,source_rgb(:,1:nstep:end)'./255,'filled');
% axis([-1 1 -1 1]); axis square

%% Histogram equalization (matching) 
% first I need to convert them into something between 0 and 255?
% or is it necessary?
source_brightness_norm = source_brightness./range(source_brightness);
target_brightness_norm = target_brightness./range(target_brightness);
binranges = 0:0.01:1;
bincounts = histc(target_brightness_norm,binranges);
source_brightness_norm_eq = histeq(source_brightness_norm,bincounts);
%source_firstComp_eq = source_firstComp_norm_eq * range(source_firstComp);
source_brightness_eq = source_brightness_norm_eq * range(target_brightness);

figure;
subplot(1,3,1); hist(source_brightness_norm);
subplot(1,3,2); hist(target_brightness_norm);
subplot(1,3,3); hist(source_brightness_norm_eq);

figure;
subplot(1,3,1); hist(source_brightness);
subplot(1,3,2); hist(target_brightness);
subplot(1,3,3); hist(source_brightness_eq);

%% normalize the angle information
%source_theta = atan2(source_oppCol(2,:),source_oppCol(1,:))./(2*pi) + 0.5; % atan2(y,x)
%target_theta = atan2(target_oppCol(2,:),target_oppCol(1,:))./(2*pi) + 0.5;
target_theta_norm = target_theta./(2*pi) + 0.5;
source_theta_norm = source_theta./(2*pi) + 0.5;

% figure; hist(source_theta);
% figure; hist(target_theta);
binranges = 0:0.01:1;
bincounts = histc(target_theta_norm, binranges);
source_theta_eq = (histeq(source_theta_norm,bincounts)-0.5).*(2*pi);
% figure; hist(source_theta_eq);
% figure;
% subplot(1,3,1); hist((source_theta-0.5).*2*pi, -pi:pi/6:pi);
% subplot(1,3,2); hist((target_theta-0.5).*2*pi,-pi:pi/6:pi);
% subplot(1,3,3); hist(source_theta_eq, -pi:pi/6:pi);

%% calculate c2, c3, recover rotated coordinate
% then calculate new rgb. plot the thing and then we will see. lol :)
%theta = angle(source_rotated(2,:) + 1i*source_rotated(3,:));
radii = sqrt(source_rotated(2,:).^2 + source_rotated(3,:).^2);
source_rotated_eq = zeros(3,length(source_brightness));
source_rotated_eq(1,:) = source_brightness_eq;
source_rotated_eq(2,:) = radii.*cos(source_theta_eq);
source_rotated_eq(3,:) = radii.*sin(source_theta_eq);
source_rgb_eq = rotation_matrix_target\source_rotated_eq;
%source_rgb_eq = rotation_matrix_source\source_rotated_eq;
source_rgb_eq_uint8 = uint8(source_rgb_eq);

[xsize,ysize] = size(source_im(:,:,1));
r = reshape(source_rgb_eq_uint8(1,:),[xsize, ysize]);
g = reshape(source_rgb_eq_uint8(2,:),[xsize, ysize]);
b = reshape(source_rgb_eq_uint8(3,:),[xsize, ysize]);

source_eq_image = cat(3,r,g,b);
figure; imshow(source_eq_image);

%% Histogram equalization on the red, green, and blue channels
% target_red = target_im(:,:,1);
% target_green = target_im(:,:,2);
% target_blue = target_im(:,:,3);
% 
% hn_target_red = imhist(target_red,256);
% hn_target_green = imhist(target_green,256);
% hn_target_blue = imhist(target_blue,256);
% 
% source_red = source_im(:,:,1);
% source_green = source_im(:,:,2);
% source_blue = source_im(:,:,3);
% 
% % equalization
% source_red_2 = histeq(source_red, hn_target_red);
% source_green_2 = histeq(source_green, hn_target_green);
% source_blue_2 = histeq(source_blue, hn_target_blue);
% 
% source_2 = cat(3,source_red_2, source_green_2, source_blue_2); 
% figure;imshow(source_2);
