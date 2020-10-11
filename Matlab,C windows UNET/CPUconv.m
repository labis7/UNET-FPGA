%%% CPU
clear all
clearvars -global
ch=1;
dim=128;
f_num=16;
tic
skips={};
for i = 1:5
    if i ~= 1
        f_num=f_num*2;%16,32,64,128
    end
    res = conv_block(f_num,ch,dim);
    ch=f_num;
    %MAXPOOL
    if i == 5
        continue;
    else
        skips{i} = res; %skip neuron
        res = maxpool(res,2,'Stride',2);
        dim=dim/2;
    end
end


%ar={};
%ar{1}=res;


for i = 6:9
    %transposed conv
    f_num=f_num/2;%128,64,32,16

    f_t =single(rand(2,2,f_num,ch));
    b_t = single(rand(f_num,1));

    tconv = dltranspconv(res,f_t,b_t,'Stride',2);

    dim = dim*2;

    C = cat(3,tconv,skips{10-i});
    
    
    res = conv_block(f_num,ch,dim);
    ch=f_num;
    
    %MAXPOOL
end


%%%%%%%%%% Final 1x1 Convolution %%%%%%%%%%%%
f_num=1;
f = single(rand(1,1,ch,f_num));

bias = single(rand(f_num,1));

out = dlconv(res,f,bias, 'padding','same');

size(out)
%start = tic; someCode; elapsed = toc(start);



%rescpu = gather(res);
%rescpu = gather(dlY);
%wait(gpuDevice
toc

function res = conv_block(f_num,ch,dim)
    %CONV1
    img = single(rand(dim,dim,ch));

    f = single(rand(3,3,ch,f_num));

    bias = single(rand(f_num,1));

    imgdl =dlarray(img,'SSC');

    conv1_1 = dlconv(imgdl,f,bias, 'padding','same');
    
    %ReLu
    conv1_1 = relu(conv1_1);
    
    %CONV2--------------------------------------------------
    
    ch=f_num;
    img = single(rand(dim,dim,ch));

    f = single(rand(3,3,ch,f_num));

    bias = single(rand(f_num,1));
    conv1_2 = dlconv(conv1_1,f,bias, 'padding','same');
    
    %ReLu
    res = relu(conv1_2);

end

