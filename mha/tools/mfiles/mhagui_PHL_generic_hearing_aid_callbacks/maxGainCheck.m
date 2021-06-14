function [handle] = maxGainCheck(handle)
max_gain = mha_get(handle.mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain');
max_all_right = get(handle.maxgain_all_edit_right,'String');
max_right = strsplit(max_all_right,' ');
max_all_left = get(handle.maxgain_all_edit_left,'String');
max_left = strsplit(max_all_left,' ');

if str2double(max_right(1)) == 0
    set(handle.G50_all_edit_right,'enable','off');
    set(handle.G50_all_slider_right,'enable','off');
%     set(handle.G80_all_edit_right,'enable','off');
    set(handle.G80_all_slider_right,'enable','off');
    
else
    set(handle.G80_all_slider_right,'max',str2double(max_right(1)),'sliderstep',[1/str2double(max_right(1)) 3/str2double(max_right(1))]);
        if handle.sel_all_G80_right.Value ~= 0
%             set(handle.G80_all_edit_right,'enable','on');
            set(handle.G80_all_slider_right,'enable','on');
        end
    set(handle.G50_all_slider_right,'max',str2double(max_right(1)),'sliderstep',[1/str2double(max_right(1)) 3/str2double(max_right(1))]);
        if handle.sel_all_G50_right.Value ~= 0
%             set(handle.G50_all_edit_right,'enable','on');
            set(handle.G50_all_slider_right,'enable','on');
        end
    
end
if str2double(max_left(1)) == 0
    set(handle.G50_all_edit_left,'enable','off');
    set(handle.G50_all_slider_left,'enable','off');
%     set(handle.G80_all_edit_left,'enable','off');
    set(handle.G80_all_slider_left,'enable','off');
else
set(handle.G80_all_slider_left,'max',str2double(max_left(1)),'sliderstep',[1/str2double(max_left(1)) 3/str2double(max_left(1))]);
    if handle.sel_all_G80_left.Value ~= 0
%         set(handle.G80_all_edit_left,'enable','on');
        set(handle.G80_all_slider_left,'enable','on');
    end
set(handle.G50_all_slider_left,'max',str2double(max_left(1)),'sliderstep',[1/str2double(max_left(1)) 3/str2double(max_left(1))]);
    if handle.sel_all_G50_left.Value ~= 0
%         set(handle.G50_all_edit_left,'enable','on');
        set(handle.G50_all_slider_left,'enable','on');
    end
end
    
if max_gain(1) == 0
    set(handle.G80_left_slider1,'min',0,'max',max_gain(1));
    set(handle.G50_left_slider1,'min',0,'max',max_gain(1));
    set(handle.G80_left_slider1,'enable','off');
    set(handle.G50_left_slider1,'enable','off');
    set(handle.G80_left_edit1,'enable','off');
    set(handle.G50_left_edit1,'enable','off');
    
    handle.dat.g1.max_left = max([handle.G80_left_slider1.Max-handle.G80_left_slider1.Value; handle.G50_left_slider1.Max-handle.G50_left_slider1.Value ]);
    handle.dat.g1.min_left = -max([handle.G80_left_slider1.Value-handle.G80_left_slider1.Min; handle.G50_left_slider1.Value-handle.G50_left_slider1.Min]);
else
    set(handle.G80_left_slider1,'min',0,'max',max_gain(1),'sliderstep',[1/max_gain(1) 3/max_gain(1)]);
    set(handle.G50_left_slider1,'min',0,'max',max_gain(1),'sliderstep',[1/max_gain(1) 3/max_gain(1)]);
    if handle.sel_all_G50_left.Value == 0
        set(handle.G50_left_slider1,'enable','on');
        set(handle.G50_left_edit1,'enable','on');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.G80_left_edit1,'enable','on');
        set(handle.G80_left_slider1,'enable','on');
    end
    handle.dat.g1.max_left = max([handle.G80_left_slider1.Max-handle.G80_left_slider1.Value; handle.G50_left_slider1.Max-handle.G50_left_slider1.Value ]);
    handle.dat.g1.min_left = -max([handle.G80_left_slider1.Value-handle.G80_left_slider1.Min; handle.G50_left_slider1.Value-handle.G50_left_slider1.Min]);
    
end
if max_gain(2) == 0
    set(handle.G80_left_slider2,'min',0,'max',max_gain(2));
    set(handle.G50_left_slider2,'min',0,'max',max_gain(2));
    set(handle.G80_left_slider2,'enable','off');
    set(handle.G50_left_slider2,'enable','off');
    set(handle.G80_left_edit2,'enable','off');
    set(handle.G50_left_edit2,'enable','off');
    
    handle.dat.g2.max_left = max([handle.G80_left_slider2.Max-handle.G80_left_slider2.Value; handle.G50_left_slider2.Max-handle.G50_left_slider2.Value ]);
    handle.dat.g2.min_left = -max([handle.G80_left_slider2.Value-handle.G80_left_slider2.Min; handle.G50_left_slider2.Value-handle.G50_left_slider2.Min]);
else
    set(handle.G80_left_slider2,'min',0,'max',max_gain(2),'sliderstep',[1/max_gain(2) 3/max_gain(2)]);
    set(handle.G50_left_slider2,'min',0,'max',max_gain(2),'sliderstep',[1/max_gain(2) 3/max_gain(2)]);
    if handle.sel_all_G50_left.Value == 0
        set(handle.G50_left_slider2,'enable','on');
        set(handle.G50_left_edit2,'enable','on');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.G80_left_edit2,'enable','on');
        set(handle.G80_left_slider2,'enable','on');
    end
    
    handle.dat.g2.max_left = max([handle.G80_left_slider2.Max-handle.G80_left_slider2.Value; handle.G50_left_slider2.Max-handle.G50_left_slider2.Value ]);
    handle.dat.g2.min_left = -max([handle.G80_left_slider2.Value-handle.G80_left_slider2.Min; handle.G50_left_slider2.Value-handle.G50_left_slider2.Min]);
end
if max_gain(3) == 0
    set(handle.G80_left_slider3,'min',0,'max',max_gain(3));
    set(handle.G50_left_slider3,'min',0,'max',max_gain(3));
    set(handle.G80_left_slider3,'enable','off');
    set(handle.G50_left_slider3,'enable','off');
    set(handle.G80_left_edit3,'enable','off');
    set(handle.G50_left_edit3,'enable','off');
    
    handle.dat.g3.max_left = max([handle.G80_left_slider3.Max-handle.G80_left_slider3.Value; handle.G50_left_slider3.Max-handle.G50_left_slider3.Value ]);
    handle.dat.g3.min_left = -max([handle.G80_left_slider3.Value-handle.G80_left_slider3.Min; handle.G50_left_slider3.Value-handle.G50_left_slider3.Min]);
else
    set(handle.G80_left_slider3,'min',0,'max',max_gain(3),'sliderstep',[1/max_gain(3) 3/max_gain(3)]);
    set(handle.G50_left_slider3,'min',0,'max',max_gain(3),'sliderstep',[1/max_gain(3) 3/max_gain(3)]);
    if handle.sel_all_G50_left.Value == 0
        set(handle.G50_left_slider3,'enable','on');
        set(handle.G50_left_edit3,'enable','on');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.G80_left_edit3,'enable','on');
        set(handle.G80_left_slider3,'enable','on');
    end
    
    handle.dat.g3.max_left = max([handle.G80_left_slider3.Max-handle.G80_left_slider3.Value; handle.G50_left_slider3.Max-handle.G50_left_slider3.Value ]);
    handle.dat.g3.min_left = -max([handle.G80_left_slider3.Value-handle.G80_left_slider3.Min; handle.G50_left_slider3.Value-handle.G50_left_slider3.Min]);
end
if max_gain(4) == 0
    set(handle.G80_left_slider4,'min',0,'max',max_gain(4));
    set(handle.G50_left_slider4,'min',0,'max',max_gain(4));
    set(handle.G80_left_slider4,'enable','off');
    set(handle.G50_left_slider4,'enable','off');
    set(handle.G80_left_edit4,'enable','off');
    set(handle.G50_left_edit4,'enable','off');
    
    handle.dat.g4.max_left = max([handle.G80_left_slider4.Max-handle.G80_left_slider4.Value; handle.G50_left_slider4.Max-handle.G50_left_slider4.Value ]);
    handle.dat.g4.min_left = -max([handle.G80_left_slider4.Value-handle.G80_left_slider4.Min; handle.G50_left_slider4.Value-handle.G50_left_slider4.Min]);
else
    set(handle.G80_left_slider4,'min',0,'max',max_gain(4),'sliderstep',[1/max_gain(4) 3/max_gain(4)]);
    set(handle.G50_left_slider4,'min',0,'max',max_gain(4),'sliderstep',[1/max_gain(4) 3/max_gain(4)]);
    if handle.sel_all_G50_left.Value == 0
        set(handle.G50_left_slider4,'enable','on');
        set(handle.G50_left_edit4,'enable','on');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.G80_left_edit4,'enable','on');
        set(handle.G80_left_slider4,'enable','on');
    end
    
    handle.dat.g4.max_left = max([handle.G80_left_slider4.Max-handle.G80_left_slider4.Value; handle.G50_left_slider4.Max-handle.G50_left_slider4.Value ]);
    handle.dat.g4.min_left = -max([handle.G80_left_slider4.Value-handle.G80_left_slider4.Min; handle.G50_left_slider4.Value-handle.G50_left_slider4.Min]);
end
if max_gain(5) == 0
    set(handle.G80_left_slider5,'min',0,'max',max_gain(5));
    set(handle.G50_left_slider5,'min',0,'max',max_gain(5));
    set(handle.G80_left_slider5,'enable','off');
    set(handle.G50_left_slider5,'enable','off');
    set(handle.G80_left_edit5,'enable','off');
    set(handle.G50_left_edit5,'enable','off');
    
    handle.dat.g5.max_left = max([handle.G80_left_slider5.Max-handle.G80_left_slider5.Value; handle.G50_left_slider5.Max-handle.G50_left_slider5.Value ]);
    handle.dat.g5.min_left = -max([handle.G80_left_slider5.Value-handle.G80_left_slider5.Min; handle.G50_left_slider5.Value-handle.G50_left_slider5.Min]);
else
    set(handle.G80_left_slider5,'min',0,'max',max_gain(5),'sliderstep',[1/max_gain(5) 3/max_gain(5)]);
    set(handle.G50_left_slider5,'min',0,'max',max_gain(5),'sliderstep',[1/max_gain(5) 3/max_gain(5)]);
    if handle.sel_all_G50_left.Value == 0
        set(handle.G50_left_slider5,'enable','on');
        set(handle.G50_left_edit5,'enable','on');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.G80_left_edit5,'enable','on');
        set(handle.G80_left_slider5,'enable','on');
    end
    
    handle.dat.g5.max_left = max([handle.G80_left_slider5.Max-handle.G80_left_slider5.Value; handle.G50_left_slider5.Max-handle.G50_left_slider5.Value ]);
    handle.dat.g5.min_left = -max([handle.G80_left_slider5.Value-handle.G80_left_slider5.Min; handle.G50_left_slider5.Value-handle.G50_left_slider5.Min]);
end




if max_gain(6) == 0
    set(handle.G80_right_slider1,'min',0,'max',max_gain(6));
    set(handle.G50_right_slider1,'min',0,'max',max_gain(6));
    set(handle.G80_right_slider1,'enable','off');
    set(handle.G50_right_slider1,'enable','off');
    set(handle.G80_right_edit1,'enable','off');
    set(handle.G50_right_edit1,'enable','off');
    
    handle.dat.g1.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G50_right_slider1.Max-handle.G50_right_slider1.Value ]);
    handle.dat.g1.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);
else
    set(handle.G80_right_slider1,'min',0,'max',max_gain(6),'sliderstep',[1/max_gain(6) 3/max_gain(6)]);
    set(handle.G50_right_slider1,'min',0,'max',max_gain(6),'sliderstep',[1/max_gain(6) 3/max_gain(6)]);
    if handle.sel_all_G50_right.Value == 0
        set(handle.G50_right_slider1,'enable','on');
        set(handle.G50_right_edit1,'enable','on');
    end
    if handle.sel_all_G80_right.Value == 0
        set(handle.G80_right_edit1,'enable','on');
        set(handle.G80_right_slider1,'enable','on');
    end
    
    handle.dat.g1.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G50_right_slider1.Max-handle.G50_right_slider1.Value ]);
    handle.dat.g1.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);
end
if max_gain(7) == 0
    set(handle.G80_right_slider2,'min',0,'max',max_gain(7));
    set(handle.G50_right_slider2,'min',0,'max',max_gain(7));
    set(handle.G80_right_slider2,'enable','off');
    set(handle.G50_right_slider2,'enable','off');
    set(handle.G80_right_edit2,'enable','off');
    set(handle.G50_right_edit2,'enable','off');
    
    handle.dat.g2.max_right = max([handle.G80_right_slider2.Max-handle.G80_right_slider2.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ]);
    handle.dat.g2.min_right = -max([handle.G80_right_slider2.Value-handle.G80_right_slider2.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min]);

else
    set(handle.G80_right_slider2,'min',0,'max',max_gain(7),'sliderstep',[1/max_gain(7) 3/max_gain(7)]);
    set(handle.G50_right_slider2,'min',0,'max',max_gain(7),'sliderstep',[1/max_gain(7) 3/max_gain(7)]);
    if handle.sel_all_G50_right.Value == 0
        set(handle.G50_right_slider2,'enable','on');
        set(handle.G50_right_edit2,'enable','on');
    end
    if handle.sel_all_G80_right.Value == 0
        set(handle.G80_right_edit2,'enable','on');
        set(handle.G80_right_slider2,'enable','on');
    end
    
    handle.dat.g2.max_right = max([handle.G80_right_slider2.Max-handle.G80_right_slider2.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ]);
    handle.dat.g2.min_right = -max([handle.G80_right_slider2.Value-handle.G80_right_slider2.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min]);

end
if max_gain(8) == 0
    set(handle.G80_right_slider3,'min',0,'max',max_gain(8));
    set(handle.G50_right_slider3,'min',0,'max',max_gain(8));
    set(handle.G80_right_slider3,'enable','off');
    set(handle.G50_right_slider3,'enable','off');
    set(handle.G80_right_edit3,'enable','off');
    set(handle.G50_right_edit3,'enable','off');
    
    handle.dat.g3.max_right = max([handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G50_right_slider3.Max-handle.G50_right_slider3.Value ]);
    handle.dat.g3.min_right = -max([handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G50_right_slider3.Value-handle.G50_right_slider3.Min]);
else
    set(handle.G80_right_slider3,'min',0,'max',max_gain(8),'sliderstep',[1/max_gain(8) 3/max_gain(8)]);
    set(handle.G50_right_slider3,'min',0,'max',max_gain(8),'sliderstep',[1/max_gain(8) 3/max_gain(8)]);
    if handle.sel_all_G50_right.Value == 0
        set(handle.G50_right_slider3,'enable','on');
        set(handle.G50_right_edit3,'enable','on');
    end
    if handle.sel_all_G80_right.Value == 0
        set(handle.G80_right_edit3,'enable','on');
        set(handle.G80_right_slider3,'enable','on');
    end
    
    handle.dat.g3.max_right = max([handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G50_right_slider3.Max-handle.G50_right_slider3.Value ]);
    handle.dat.g3.min_right = -max([handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G50_right_slider3.Value-handle.G50_right_slider3.Min]);
end
if max_gain(9) == 0
    set(handle.G80_right_slider4,'min',0,'max',max_gain(9));
    set(handle.G50_right_slider4,'min',0,'max',max_gain(9));
    set(handle.G80_right_slider4,'enable','off');
    set(handle.G50_right_slider4,'enable','off');
    set(handle.G80_right_edit4,'enable','off');
    set(handle.G50_right_edit4,'enable','off');
    
    handle.dat.g4.max_right = max([handle.G80_right_slider4.Max-handle.G80_right_slider4.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value ]);
    handle.dat.g4.min_right = -max([handle.G80_right_slider4.Value-handle.G80_right_slider4.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min]);
else
    set(handle.G80_right_slider4,'min',0,'max',max_gain(9),'sliderstep',[1/max_gain(9) 3/max_gain(9)]);
    set(handle.G50_right_slider4,'min',0,'max',max_gain(9),'sliderstep',[1/max_gain(9) 3/max_gain(9)]);
    if handle.sel_all_G50_right.Value == 0
        set(handle.G50_right_slider4,'enable','on');
        set(handle.G50_right_edit4,'enable','on');
    end
    if handle.sel_all_G80_right.Value == 0
        set(handle.G80_right_edit4,'enable','on');
        set(handle.G80_right_slider4,'enable','on');
    end
    
    handle.dat.g4.max_right = max([handle.G80_right_slider4.Max-handle.G80_right_slider4.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value ]);
    handle.dat.g4.min_right = -max([handle.G80_right_slider4.Value-handle.G80_right_slider4.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min]);
end
if max_gain(10) == 0
    set(handle.G80_right_slider5,'min',0,'max',max_gain(10));
    set(handle.G50_right_slider5,'min',0,'max',max_gain(10));
    set(handle.G80_right_slider5,'enable','off');
    set(handle.G50_right_slider5,'enable','off');
    set(handle.G80_right_edit5,'enable','off');
    set(handle.G50_right_edit5,'enable','off');
    
    handle.dat.g5.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value; handle.G50_right_slider5.Max-handle.G50_right_slider5.Value ]);
    handle.dat.g5.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min; handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);
else
    set(handle.G80_right_slider5,'min',0,'max',max_gain(10),'sliderstep',[1/max_gain(10) 3/max_gain(10)]);
    set(handle.G50_right_slider5,'min',0,'max',max_gain(10),'sliderstep',[1/max_gain(10) 3/max_gain(10)]);
    if handle.sel_all_G50_right.Value == 0
        set(handle.G50_right_slider5,'enable','on');
        set(handle.G50_right_edit5,'enable','on');
    end
    if handle.sel_all_G80_right.Value == 0
        set(handle.G80_right_edit5,'enable','on');
        set(handle.G80_right_slider5,'enable','on');
    end
    
    handle.dat.g5.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value; handle.G50_right_slider5.Max-handle.G50_right_slider5.Value ]);
    handle.dat.g5.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min; handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);
end
handle.dat.bb.max_left = max([handle.G80_left_slider5.Max-handle.G80_left_slider5.Value;handle.G80_left_slider4.Max-handle.G80_left_slider4.Value;...
    handle.G80_left_slider3.Max-handle.G80_left_slider3.Value;handle.G80_left_slider2.Max-handle.G80_left_slider2.Value;...
    handle.G80_left_slider1.Max-handle.G80_left_slider1.Value;handle.G50_left_slider5.Max-handle.G50_left_slider5.Value;...
    handle.G50_left_slider4.Max-handle.G50_left_slider4.Value;handle.G50_left_slider3.Max-handle.G50_left_slider3.Value;...
    handle.G50_left_slider2.Max-handle.G50_left_slider2.Value;handle.G50_left_slider1.Max-handle.G50_left_slider1.Value]);
handle.dat.bb.min_left = -max([handle.G80_left_slider5.Value-handle.G80_left_slider5.Min;handle.G80_left_slider4.Value-handle.G80_left_slider4.Min;...
    handle.G80_left_slider3.Value-handle.G80_left_slider3.Min;handle.G80_left_slider2.Value-handle.G80_left_slider2.Min;...
    handle.G80_left_slider1.Value-handle.G80_left_slider1.Min;handle.G50_left_slider5.Value-handle.G50_left_slider5.Min;...
    handle.G50_left_slider4.Value-handle.G50_left_slider4.Min;handle.G50_left_slider3.Value-handle.G50_left_slider3.Min;...
    handle.G50_left_slider2.Value-handle.G50_left_slider2.Min;handle.G50_left_slider1.Value-handle.G50_left_slider1.Min]);
handle.dat.bb.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value;handle.G80_right_slider4.Max-handle.G80_right_slider4.Value;...
    handle.G80_right_slider3.Max-handle.G80_right_slider3.Value;handle.G80_right_slider2.Max-handle.G80_right_slider2.Value;...
    handle.G80_right_slider1.Max-handle.G80_right_slider1.Value;handle.G50_right_slider5.Max-handle.G50_right_slider5.Value;...
    handle.G50_right_slider4.Max-handle.G50_right_slider4.Value;handle.G50_right_slider3.Max-handle.G50_right_slider3.Value;...
    handle.G50_right_slider2.Max-handle.G50_right_slider2.Value;handle.G50_right_slider1.Max-handle.G50_right_slider1.Value]);
handle.dat.bb.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min;handle.G80_right_slider4.Value-handle.G80_right_slider4.Min;...
    handle.G80_right_slider3.Value-handle.G80_right_slider3.Min;handle.G80_right_slider2.Value-handle.G80_right_slider2.Min;...
    handle.G80_right_slider1.Value-handle.G80_right_slider1.Min;handle.G50_right_slider5.Value-handle.G50_right_slider5.Min;...
    handle.G50_right_slider4.Value-handle.G50_right_slider4.Min;handle.G50_right_slider3.Value-handle.G50_right_slider3.Min;...
    handle.G50_right_slider2.Value-handle.G50_right_slider2.Min;handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);
end
