function sAud = audprof_db_xlsimport( xlsName )
% load air conduction threshold from XLS file
% xlsName : name of XLS file
% cDB : database to insert auditory profiles (optional)
% if no database is provided, then a new database will be created
  warning('off','MATLAB:xlsread:Mode');
  [mNum,mTxt,mRaw] = xlsread( xlsName, 1, '', 'basic' );
  csHead = mRaw(1,2:end);
  csData = mRaw(2:end,2:end);
  csClient = mRaw(2:end,1);
  vValidF = unique(round([1000*2.^[-4:4],750*2.^[0:4]]));
  vValidS = 'lr';
  csValidHead = {};
  vValidF_ = [];
  vValidS_ = '';
  for sSide=vValidS
    for fFreq=vValidF
      csValidHead{end+1} = sprintf('%s%d',sSide,fFreq);
      vValidF_(end+1) = fFreq;
      vValidS_(end+1) = sSide;
    end
  end
  csHead = strrep(lower(csHead),' ','');
  vInd = [];
  for k=1:numel(csHead)
    idx = strmatch(csHead{k},csValidHead,'exact');
    if isempty(idx)
      error(sprintf('the field "%s" is not a valid field', ...
		    csHead{k}));
    end
    vInd(end+1) = idx;
  end
  vF = vValidF_(vInd);
  idx = struct();
  idx.l = find(vValidS_(vInd) == 'l');
  idx.r = find(vValidS_(vInd) == 'r');
  vFreq = struct;
  vFreq.l = vF(idx.l);
  vFreq.r = vF(idx.r);
  for kClient = 1:size(csData,1)
    sClient = upper(csClient{kClient});
    if ~audprof_client_exists( sClient )
      cClient = audprof_client_new(sClient);
      cDB = audprof_db_load();
      cDB = audprof_db_client_add( cDB, cClient );
      audprof_db_save( cDB );
    end
    sAud = audprof_audprof_new( sClient );
    sAud.id = xlsName;
    for sSide=vValidS
      vThr = cell2mat(csData(kClient,idx.(sSide)));
      vs = audprof_threshold_entry_add(audprof_threshold_new(),...
				       vFreq.(sSide),...
				       vThr);
      sAud.(sSide).htl_ac = vs;
    end
    %% Tobias N. suggests to remove the next line:
    sAud = audprof_audprof_cleanup( sAud );
    audprof_db_audprof_add( sAud );
  end
  %csValidHead
  %vValidF_
  %vValidS_