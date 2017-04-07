% example configuration file for localization test:

%sCfg.sound.prepare = @loctest_sound_prepare;
%sCfg.sound.release = @loctest_sound_release;
sCfg.sound.play = @loctest_sound_play;

sCfg.par.az = [0:15:345];
sCfg.par.method = {'amb','speaker'};

sCfg.gui.fontsize = 20;
sCfg.gui.az = [0:15:345];
sCfg.gui.label = {'0','a','1','b','2','c','3','d','4','e','5','f',...
		  '6','g','7','h','8','i','9','k','10','l','11', ...
		  'm'};
sCfg.gui.marker = {'ro','','ko','','ko','','ko','','ko','','ko','',...
		   'ko','','ko','','ko','','ko','','ko','','ko',''};
sCfg.gui.markersize = 40;
sCfg.gui.linewidth = 4;