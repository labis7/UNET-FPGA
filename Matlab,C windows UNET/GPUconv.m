% %clear all
% ch=256;
% f_num=128;
% dim=32;
% 
% tic
% %gpuDevice(1);
% img = single(rand(dim,dim,ch,'gpuArray'));
% 
% f = single(rand(3,3,ch,f_num,'gpuArray'));
% 
% bias = single(rand(f_num,1,'gpuArray'));
% 
% imgdl =dlarray(img,'SSC');
% 
% dlY = dlconv(imgdl,f,bias,'padding','same');
% 
% %rescpu = gather(res);
% rescpu = gather(dlY);
% wait(gpuDevice)
% toc
%size(dlY)
%clear all
%gpuDevice(1);
%clearvars -global
%%% CPU

%clear all
%clearvars -global
ch=1;
dim=128;
f_num=16;
start = tic;
skips={};
for i = 1:5
    
    if i ~= 1
        f_num=f_num*2;%16,32,64,128
    end
    res = conv_block(f_num,ch,dim,i);
    ch=f_num;

    
    %MAXPOOL
    if i == 5
        continue;
    else
        st=tic;
        skips{i} = res; %skip neuron
        res = maxpool(res,2,'Stride',2);
        dim=dim/2;
        res = gather(res);
        wait(gpuDevice);
        el=toc(st);
        sprintf("Max-pool %d : %0.2f ms",i,el*1000)
    end
end


%ar={};
%ar{1}=res;


for i = 6:9
    st=tic;
    %transposed conv
    f_num=f_num/2;%128,64,32,16

    f_t =single(rand(2,2,f_num,ch, 'gpuArray'));
    b_t = single(rand(f_num,1, 'gpuArray'));

    tconv = dltranspconv(res,f_t,b_t,'Stride',2);

    
    tconv = gather(tconv);
    wait(gpuDevice);
    
    
    dim = dim*2;
    el=toc(st);
    sprintf("TConv %d : %0.2f ms",i,el*1000)
    
    C = cat(3,tconv,skips{10-i});
    
    
    res = conv_block(f_num,ch,dim,i);
    ch=f_num;
    
    %MAXPOOL
end


%%%%%%%%%% Final 1x1 Convolution %%%%%%%%%%%%
f_num=1;
f = single(rand(1,1,ch,f_num, 'gpuArray'));

bias = single(rand(f_num,1, 'gpuArray'));

out = dlconv(res,f,bias, 'padding','same');

out = gather(out);
wait(gpuDevice);

elapsed=toc(start);
fprintf("Overall Time : %.2f ms",elapsed*1000);

%start = tic; someCode; elapsed = toc(start);



%rescpu = gather(res);
%rescpu = gather(dlY);
%wait(gpuDevice


function res = conv_block(f_num,ch,dim,i)
    st=tic;

    %CONV1
    img = single(rand(dim,dim,ch, 'gpuArray'));

    f = single(rand(3,3,ch,f_num, 'gpuArray'));

    bias = single(rand(f_num,1, 'gpuArray'));

    imgdl =dlarray(img,'SSC');

    conv1_1 = dlconv(imgdl,f,bias, 'padding','same');
    
    
    %ReLu
    conv1_1 = relu(conv1_1);
    
    conv1_1 = gather(conv1_1);
    wait(gpuDevice);
    
    el=toc(st);
    sprintf("Conv %d.1 : %0.2f ms",i,el*1000)
    st=tic;
    %CONV2--------------------------------------------------
    
    ch=f_num;
    img = single(rand(dim,dim,ch, 'gpuArray'));

    f = single(rand(3,3,ch,f_num, 'gpuArray'));

    bias = single(rand(f_num,1, 'gpuArray'));
    conv1_2 = dlconv(conv1_1,f,bias, 'padding','same');
    
    %ReLu
    res = relu(conv1_2);

    res = gather(res);
    wait(gpuDevice);
    
    el=toc(st);
    sprintf("Conv %d.2 : %0.2f ms",i,el*1000)
end

