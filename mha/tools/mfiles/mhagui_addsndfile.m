   % This file is part of the HörTech Open Master Hearing Aid (openMHA)
   % Copyright © 2013 2016 2019 HörTech gGmbH
   %
   % openMHA is free software: you can redistribute it and/or modify
   % it under the terms of the GNU Affero General Public License as published by
   % the Free Software Foundation, version 3 of the License.
   %
   % openMHA is distributed in the hope that it will be useful,
   % but WITHOUT ANY WARRANTY; without even the implied warranty of
   % MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   % GNU Affero General Public License, version 3 for more details.
   %
   % You should have received a copy of the GNU Affero General Public License,
   % version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

function mhagui_addsndfile(mha,base)
  if nargin < 1
    mha = struct('host','localhost','port',33337);
  end
  sCfg = mha_get_basic_cfg_network( mha );
  if nargin > 1
    sCfg.base.addsndfile = base;
    if ~strcmp(mha_query(mha,base,'id'),'addsndfile')
      error(sprintf(['The plugin at ''%s'' has not the ID' ' ''addsndfile''.'],base));
    end
  end
  if ~isfield(sCfg.base,'addsndfile')
    error('no addsndfile plugin found');
  end
  sCfg.orig_cfg = mha_get(mha,sCfg.base.addsndfile,'writable');
  sCfg.status = mha_get(mha,sCfg.base.addsndfile,'monitor');
  close(findobj('Tag','mhagui_insitu_window'));
  libmhagui();
  fh = mhagui.figure('in-situ signals','mhagui_insitu_window',...
                     [630 470],'UserData',sCfg,...
                     'Color',[0.7 0.7 0.7] );
  sTestSigNames = sCfg.status.files;
  sTestSigNames{end+1} = '';
  fname = sCfg.orig_cfg.filename;
  cval = strmatch(fname,sTestSigNames,'exact');
  if isempty(cval)
    cval = 1;
  end
  uicontrol('Style','listbox','String',sTestSigNames,...
            'Position',[10 195 300 265],...
            'Callback',@calib_testsig_selection,...
            'Value',cval,...
            'Tag','calib_testsig_selector');
  uicontrol('Style','PushButton','String','rescan',...
            'Position',[10 155 145 30],...
            'Callback',@testsig_rescan);
  pos = [320 235 0 0];
  uicontrol('style','frame','position',[0,0,300,225]+pos);
  csMics = {};
  for k=1:sCfg.status.mhachannels
    csMics{k} = sprintf('in_%d',k);
  end
  if isfield(sCfg.base,'MHAIOJack')
    csJackMics = mha_get(sCfg.mha,[sCfg.base.MHAIOJack, ...
                                   '.names_in']);
    if length(csJackMics) == length(csMics)
      csMics = csJackMics;
    end
  end
  for k=1:length(csMics)
    col = [0 0.2 0];
    if ~isempty(strfind(csMics{k},'left'))
      col = [0 0 1];
    end
    if ~isempty(strfind(csMics{k},'right'))
      col = [1 0 0];
    end
    uicontrol('Style','CheckBox',...
              'String',csMics{k},...
              'value',1,...
              'Position',[10 215-k*25 280 25]+pos,...
              'Tag',sprintf('calib_testsig_mic%d',k),...
              'Callback',@calib_testsig_channels,...
              'ForeGroundColor',col);
  end
  uicontrol('Style','text','Tag','calib_testsig_unsused',...
            'Position',[10 10 280 25]+pos,...
            'HorizontalAlignment','left',...
            'Fontweight','bold');
  pos = [320,10,300,215];
  uicontrol('style','frame','position',pos);
  mhagui_keyword(...
                  mha,[sCfg.base.addsndfile,'.mode'],...
                  [pos(1:2)+[10,170],120]);
  mhagui_bool(...
               mha,[sCfg.base.addsndfile,'.loop'],...
               [pos(1:2)+[10,130],120]);
  mhagui_keyword(...
                  mha,[sCfg.base.addsndfile,'.levelmode'],...
                  [pos(1:2)+[10,90],120]);
  mhagui_scalar(...
                 mha,[sCfg.base.addsndfile,'.level'],...
                 [pos(1:2)+[10,50],120],[-40,120]);
  mhagui_scalar(...
                 mha,[sCfg.base.addsndfile,'.ramplen'],...
                 [pos(1:2)+[10,10],120],[0,1]);
  pos = [10,10,300,95];
  uicontrol('style','frame','position',pos);
  mhagui_edit(...
               mha,[sCfg.base.addsndfile,'.path'],...
               [pos(1:2)+[10,50],120]);
  mhagui_edit(...
               mha,[sCfg.base.addsndfile,'.search_pattern'],...
               [pos(1:2)+[10,10],120]);
  calib_testsig_init;

  function calib_testsig_update_mapping(varargin)
    cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
    mapping = mha_get(cfg.mha,...
                      [cfg.base.addsndfile,'.mapping'])+1;
    file_channels = ...
    mha_get(cfg.mha,[cfg.base.addsndfile,'.filechannels']);
    vInCh = ones(1,file_channels);
    for k=1:cfg.status.mhachannels
      uih = findobj('Tag',sprintf('calib_testsig_mic%d',k));
      str = get(uih,'String');
      idx = strfind(str,' (');
      if ~isempty(idx)
        str(idx(1):end) = '';
      end
      if( mapping(k) > 0 )
        str = sprintf('%s (%d)',str,mapping(k));
        vInCh(mapping(k)) = 0;
      end
      set(uih,'String',str);
    end
    uih = findobj('Tag','calib_testsig_unsused');
    if isempty(find(vInCh))
      set(uih,'String','');
    else
      set(uih,'String',...
          sprintf('Unused:%s',...
                  sprintf(' %d',find(vInCh))));
    end

    function calib_testsig_toggle(varargin)
      cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
      uih = findobj('Tag','calib_testsig_toggle');
      if getobjval('calib_testsig_toggle')
        mode = 'replace';
        set(uih,'BackgroundColor',[0.7 1 0.7]);
      else
        mode = 'input';
        set(uih,'BackgroundColor',[0.7 0.7 0.7]);
      end
      mha_set(cfg.mha,[cfg.base.addsndfile,'.mode'],mode);

      function calib_testsig_selection(varargin)
        cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
        idx = getobjval( 'calib_testsig_selector' );
        testsigs = getobjstr( 'calib_testsig_selector' );
        testsig = testsigs{idx};
        mha_set(cfg.mha,[cfg.base.addsndfile,'.filename'],testsig);
        calib_testsig_update_mapping;

        function calib_testsig_level(varargin)
          uih = findobj('Tag','calib_testsig_rmsedit');
          rms = str2num(get(uih,'String'));
          if rms > 120
            rms = 120;
            set(uih,'String',num2str(rms));
            errordlg('RMS level limited to 120 dB SPL!');
          end
          cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
          mha_set(cfg.mha,[cfg.base.addsndfile,'.level'],rms);

          function calib_testsig_channels(varargin)
            ch = [];
            for k=1:6
              if getobjval(sprintf('calib_testsig_mic%d',k))
                ch(end+1) = k-1;
              end
            end
            cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
            mha_set(cfg.mha,[cfg.base.addsndfile,'.channels'],ch);
            calib_testsig_update_mapping;

            function calib_testsig_init(varargin)
              calib_testsig_channels;
              calib_testsig_selection;
              calib_testsig_update_mapping;

              function v = getobjval( name )
                v = get(findobj('Tag',name),'value');

                function v = getobjstr( name )
                  v = get(findobj('Tag',name),'string');

                  function testsig_rescan( varargin )
                    cfg = get(findobj('Tag','mhagui_insitu_window'),'UserData');
                    h = findobj('tag','calib_testsig_selector');
                    sFiles = mha_get(cfg.mha,...
                                     [cfg.base.addsndfile,'.files']);
                    sFiles{end+1} = '';
                    fname = mha_get(cfg.mha,...
                                    [cfg.base.addsndfile,'.filename']);
                    cval = strmatch(fname,sFiles,'exact');
                    if isempty(cval)
                      cval = 1;
                    end
                    set(h,'String',sFiles,'Value',cval);
