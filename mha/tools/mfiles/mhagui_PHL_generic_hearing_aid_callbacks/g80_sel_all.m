function [handle] = g80_sel_all(src,event)
handle = guidata(src);
dat = handle.dat;
edit_tag = get(gcbo,'tag');
max_all_right = get(handle.maxgain_all_edit_right,'String');
max_right = strsplit(max_all_right,' ');
max_all_left = get(handle.maxgain_all_edit_left,'String');
max_left = strsplit(max_all_left,' ');

dat.g80.ref_right = [handle.G80_right_slider1.Value handle.G80_right_slider2.Value handle.G80_right_slider3.Value handle.G80_right_slider4.Value handle.G80_right_slider5.Value];
dat.g80.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G80_right_slider2.Max-handle.G80_right_slider2.Value ;...
    handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G80_right_slider4.Max-handle.G80_right_slider4.Value;...
    handle.G80_right_slider5.Max-handle.G80_right_slider5.Value]);
dat.g80.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G80_right_slider2.Value-handle.G80_right_slider2.Min;...
    handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G80_right_slider4.Value-handle.G80_right_slider4.Min;...
    handle.G80_right_slider5.Value-handle.G80_right_slider5.Min]);

dat.g80.ref_left = [handle.G80_left_slider1.Value handle.G80_left_slider2.Value handle.G80_left_slider3.Value handle.G80_left_slider4.Value handle.G80_left_slider5.Value];
dat.g80.max_left = max([handle.G80_left_slider1.Max-handle.G80_left_slider1.Value; handle.G80_left_slider2.Max-handle.G80_left_slider2.Value ;...
    handle.G80_left_slider3.Max-handle.G80_left_slider3.Value; handle.G80_left_slider4.Max-handle.G80_left_slider4.Value;...
    handle.G80_left_slider5.Max-handle.G80_left_slider5.Value]);
dat.g80.min_left = -max([handle.G80_left_slider1.Value-handle.G80_left_slider1.Min; handle.G80_left_slider2.Value-handle.G80_left_slider2.Min;...
    handle.G80_left_slider3.Value-handle.G80_left_slider3.Min; handle.G80_left_slider4.Value-handle.G80_left_slider4.Min;...
    handle.G80_left_slider5.Value-handle.G80_left_slider5.Min]);

switch edit_tag
    case 'right'
        if handle.sel_all_G80_right.Value == 0
            set([handle.G80_right_slider1 handle.G80_right_slider2 handle.G80_right_slider3 handle.G80_right_slider4 handle.G80_right_slider5],...
                    'backgroundcolor',[1 .5 .5]);
            if str2double(max_right(1)) ~= 0
                set([handle.G80_right_slider1 handle.G80_right_slider2 handle.G80_right_slider3 handle.G80_right_slider4 handle.G80_right_slider5],...
                    'enable','on');
                set([handle.G80_right_edit1 handle.G80_right_edit2 handle.G80_right_edit3 handle.G80_right_edit4 handle.G80_right_edit5],...
                    'enable','on');
            end
%             set(handle.G80_all_edit_right,'enable','off');
            set(handle.G80_all_slider_right ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_G80_right.Value == 1
            set([handle.G80_right_slider1 handle.G80_right_slider2 handle.G80_right_slider3 handle.G80_right_slider4 handle.G80_right_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.G80_right_edit1 handle.G80_right_edit2 handle.G80_right_edit3 handle.G80_right_edit4 handle.G80_right_edit5],...
                'enable','off');
            set(handle.G80_all_slider_right,'backgroundcolor',[1 .5 .5],'min',dat.g80.min_right,'max',dat.g80.max_right,...
                'sliderstep',[1/(dat.g80.max_right-dat.g80.min_right) 3/(dat.g80.max_right-dat.g80.min_right)]);
            if str2double(max_right(1)) ~= 0
                set(handle.G80_all_slider_right,'enable','on');
%                 set(handle.G80_all_edit_right,'enable','on');
            end
        end
    case 'left'
        if handle.sel_all_G80_left.Value == 0
            set([handle.G80_left_slider1 handle.G80_left_slider2 handle.G80_left_slider3 handle.G80_left_slider4 handle.G80_left_slider5],...
                    'backgroundcolor',[.5 .5 1]);
            if str2double(max_left(1)) ~= 0
                set([handle.G80_left_slider1 handle.G80_left_slider2 handle.G80_left_slider3 handle.G80_left_slider4 handle.G80_left_slider5],...
                    'enable','on');
                set([handle.G80_left_edit1 handle.G80_left_edit2 handle.G80_left_edit3 handle.G80_left_edit4 handle.G80_left_edit5],...
                    'enable','on');
            end
%             set(handle.G80_all_edit_left,'enable','off');
            set(handle.G80_all_slider_left ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_G80_left.Value == 1
            set([handle.G80_left_slider1 handle.G80_left_slider2 handle.G80_left_slider3 handle.G80_left_slider4 handle.G80_left_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.G80_left_edit1 handle.G80_left_edit2 handle.G80_left_edit3 handle.G80_left_edit4 handle.G80_left_edit5],...
                'enable','off');
            set(handle.G80_all_slider_left,'backgroundcolor',[.5 .5 1],'min',dat.g80.min_left,'max',dat.g80.max_left,...
                'sliderstep',[1/(dat.g80.max_left-dat.g80.min_left) 3/(dat.g80.max_left-dat.g80.min_left)]);
            if str2double(max_left(1)) ~= 0
                set(handle.G80_all_slider_left,'enable','on');
%                 set(handle.G80_all_edit_left,'enable','on');
            end
        end
end


handle.dat = dat;
guidata(src,handle);
