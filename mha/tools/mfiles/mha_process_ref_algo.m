function y = mha_process_ref_algo( x, fs, sAlgo, sPar, sAlgoPath )
% mha_process_ref_algo - process sound with openMHA algorithm
%
% Usage:
% y = mha_process_ref_algo( x, fs, sAlgo, sPar, sAlgoPath );
% or
% algos = mha_process_ref_algo()
% algos = mha_process_ref_algo('','','','',sAlgoPath)
%
% Input:
% x: sound of hearing aid microphones with 6 channels (columns): front-left,
% front-right, middle-left, middle-right, rear-left, rear-right
% fs: sampling frequency of x
% sAlgo: string with algorithm name, one of csAlgos
% sPar: string with parameters of algorithm that have to be changed, use
% sPar = '"parameter1 = [value value]" "parameter2 = [value]"'. Which
% parameters can be changed can be found in the algorithm documentation.
% sAlgoPath: path to main directory of the reference algorithms, leave
% empty to use default installation directories, provide path when you use
% non-default installation directories as provided by openMHA installer
% packages
%
% Output:
% y: processed sound with 2 channels (columns): left and right
% or
% algos: cell array containing the names of the algorithms that can be used
%
% Authors: Giso Grimm, Hendrik Kayser 2020

if isoctave 
    pkg load signal
end

if nargin < 5
    sAlgoPath = mha_install_dirs('reference_algorithms');
end

% Find all algorithms by folders
stDir = dir(sAlgoPath);
csAlgos = {stDir(~ismember({stDir.name},{'.','..'}) & [stDir.isdir]).name};

if nargin < 1 || nargin == 5 && isempty(x)
    y = csAlgos;
    return;
end

sConfig = fullfile(sAlgoPath,sAlgo,[sAlgo '.cfg']);

% This is needed to enable configurations to find additional files to be
% loaded
setenv('MHA_CFG_DIR',fullfile(sAlgoPath,sAlgo));

if nargin < 4
    sPar = '';
end

% Read sampling rate and input channel selection from openMHA
% configuration files
cfgfile = fopen(sConfig,'r');
cfgtext=textscan(cfgfile,'%s','Delimiter','\n');
fclose(cfgfile);
sSrate=cfgtext{1}{~cellfun('isempty',strfind(cfgtext{1},'srate ='))};
fsAlgo = str2double(sSrate(strfind(sSrate,'=')+1:end));
sChAlgo=cfgtext{1}{~cellfun('isempty',strfind(cfgtext{1},'chAlgo ='))};
chAlgo = str2num(sChAlgo(strfind(sChAlgo,'=')+1:end));

%
if ~any(~cellfun('isempty',strfind(csAlgos,sAlgo)))
    sAlgos = '';
    for k=1:numel(csAlgos)
        sAlgos = [sAlgos,' ''',csAlgos{k},''''];
    end
    error(['Unsupported algorithm "',sAlgo,'" (valid:',sAlgos,').']);
end


if fsAlgo ~= fs
    xAlgo = resample( x(:,chAlgo), fsAlgo, fs );
else
    xAlgo = x(:,chAlgo);
end

sName = tempname();
sNameIn = [sName,'_in.wav'];
sNameOut = [sName,'_out.wav'];
audiowrite(sNameIn,xAlgo,fsAlgo,'BitsPerSample',32);
sCmd = ['(mha',...
    ' ?read:',sConfig,...
    ' io.in=',sNameIn,...
    ' io.out=',sNameOut,...
    ' ',sPar,...
    ' cmd=start cmd=quit)'];
[stat,msg] =system(sCmd);
if stat ~= 0
    error(msg);
end
yAlgo = audioread(sNameOut);
if fsAlgo ~= fs
    y = resample( yAlgo, fs, fsAlgo );
else
    y = yAlgo;
end
delete(sNameIn);
delete(sNameOut);
