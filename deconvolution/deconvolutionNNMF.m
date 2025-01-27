function [ stain1_nnmf, stain2_nnmf, saturation_mat_nnmf] = deconvolutionNNMF(imname,datadir,resultdir, options, varargin)
% deconvolutionNNMF used non negative matrix factorization 
% 
defaultopt = struct('PlotResults','on',...
    'filterOD',0.15,'CropImage','on','ColorSpace','OD'); % flag for plotting the results

if nargin < 4
    options = [];
    if nargin < 3
          error('Need at least 3 inputs: image name, data directory, and result directory')
    end
end

raw_image = imread([datadir filesep imname]);
imshow(raw_image);
% get the extreme cutoff and filter optical density and plot flag
filterOD = optimget(options,'filterOD',defaultopt,'fast');
plotflag = optimget(options,'PlotResults',defaultopt,'fast');
cropflag = optimget(options,'CropImage',defaultopt,'fast');
colorspace = optimget(options,'ColorSpace',defaultopt,'fast');

% crop/not crop the image
if strcmpi(cropflag,'on')
    disp('Crop a region of image to input into NNMF');
    rect = getrect;
    raw_image = imcrop(raw_image,rect);
    imshow(raw_image); title(['Cropped image of' imname]);
end

[xsize, ysize] = size(raw_image(:,:,1));
%calculate optical density
%opticalDensity = raw2rgb(raw_image)./255;
% sample 10K of the pixels
rgb_image_whole = raw2rgb(raw_image); % flatten RGB image
nsamples = 10000;
indx_pixel = randperm(xsize*ysize,nsamples);
rgb_image = rgb_image_whole(:,indx_pixel);

% so in this part we just need to get the stain vectors
% decomposition of image from the stain vectors, there is a function
calculate_optical_density 
[stain_mat_nnmf,~] = nnmf(opticalDensity,2);
% keep the stain vectors in RGB form
stain1_nnmf = od2rgb(stain_mat_nnmf(:,1),1,1);
stain2_nnmf = od2rgb(stain_mat_nnmf(:,2),1,1);

disp('deconvole image by nnmf selected stain vectors...')
disp('recalculate optical density from the whole image, not just the sample')
rgb_image = rgb_image_whole;
calculate_optical_density;
saturation_mat_nnmf = pinv(stain_mat_nnmf)*opticalDensity;
stain1_rgb = stainvec2rgb(stain_mat_nnmf(:,1),saturation_mat_nnmf(1,:),xsize,ysize);
stain2_rgb = stainvec2rgb(stain_mat_nnmf(:,2),saturation_mat_nnmf(2,:),xsize,ysize);
remain_rgb = raw_image - stain1_rgb - stain2_rgb;

if strcmpi(plotflag,'on')
    plot_nnmf_results;
end

end

% give nnmf an input with equal purple and pink -> results
% histogram of that patch -> get that part of the histogram
% put manual stain vectors on the clouds. 
