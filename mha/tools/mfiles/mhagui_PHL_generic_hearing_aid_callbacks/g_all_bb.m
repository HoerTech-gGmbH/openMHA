function handle = g_all_bb(src,event)
handle = guidata(src);
sel_tag = get(gcbo,'tag');
switch sel_tag
    case 'bb_right'
        if handle.sel_gall_right.Value == 0
            set([handle.all_bb_right handle.all_bb_rightp3 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightm1],'visible','off');
            set([handle.all_gain_right1 handle.all_gain_rightp31 handle.all_gain_rightm31 handle.all_gain_rightp11 handle.all_gain_rightm11],'visible','on');
            set([handle.all_gain_right2 handle.all_gain_rightp32 handle.all_gain_rightm32 handle.all_gain_rightp12 handle.all_gain_rightm12],'visible','on');
            set([handle.all_gain_right3 handle.all_gain_rightp33 handle.all_gain_rightm33 handle.all_gain_rightp13 handle.all_gain_rightm13],'visible','on');
            set([handle.all_gain_right4 handle.all_gain_rightp34 handle.all_gain_rightm34 handle.all_gain_rightp14 handle.all_gain_rightm14],'visible','on');
            set([handle.all_gain_right5 handle.all_gain_rightp35 handle.all_gain_rightm35 handle.all_gain_rightp15 handle.all_gain_rightm15],'visible','on');
            set(handle.sel_gall_right,'position',[0.43 0.1 0.1 0.05]);
        else
            set([handle.all_bb_right handle.all_bb_rightp3 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightm1],'visible','on');
            set([handle.all_gain_right1 handle.all_gain_rightp31 handle.all_gain_rightm31 handle.all_gain_rightp11 handle.all_gain_rightm11],'visible','off');
            set([handle.all_gain_right2 handle.all_gain_rightp32 handle.all_gain_rightm32 handle.all_gain_rightp12 handle.all_gain_rightm12],'visible','off');
            set([handle.all_gain_right3 handle.all_gain_rightp33 handle.all_gain_rightm33 handle.all_gain_rightp13 handle.all_gain_rightm13],'visible','off');
            set([handle.all_gain_right4 handle.all_gain_rightp34 handle.all_gain_rightm34 handle.all_gain_rightp14 handle.all_gain_rightm14],'visible','off');
            set([handle.all_gain_right5 handle.all_gain_rightp35 handle.all_gain_rightm35 handle.all_gain_rightp15 handle.all_gain_rightm15],'visible','off');
            set(handle.sel_gall_right,'position',[0.28 0.1 0.1 0.05]);
        end
    case 'bb_left'
        if handle.sel_gall_left.Value == 0
            set([handle.all_bb_left handle.all_bb_leftp3 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftm1],'visible','off');
            set([handle.all_gain_left1 handle.all_gain_leftp31 handle.all_gain_leftm31 handle.all_gain_leftp11 handle.all_gain_leftm11],'visible','on');
            set([handle.all_gain_left2 handle.all_gain_leftp32 handle.all_gain_leftm32 handle.all_gain_leftp12 handle.all_gain_leftm12],'visible','on');
            set([handle.all_gain_left3 handle.all_gain_leftp33 handle.all_gain_leftm33 handle.all_gain_leftp13 handle.all_gain_leftm13],'visible','on');
            set([handle.all_gain_left4 handle.all_gain_leftp34 handle.all_gain_leftm34 handle.all_gain_leftp14 handle.all_gain_leftm14],'visible','on');
            set([handle.all_gain_left5 handle.all_gain_leftp35 handle.all_gain_leftm35 handle.all_gain_leftp15 handle.all_gain_leftm15],'visible','on');
            set(handle.sel_gall_left,'position',[0.43 0.1 0.1 0.05]);
        else
            set([handle.all_bb_left handle.all_bb_leftp3 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftm1],'visible','on');
            set([handle.all_gain_left1 handle.all_gain_leftp31 handle.all_gain_leftm31 handle.all_gain_leftp11 handle.all_gain_leftm11],'visible','off');
            set([handle.all_gain_left2 handle.all_gain_leftp32 handle.all_gain_leftm32 handle.all_gain_leftp12 handle.all_gain_leftm12],'visible','off');
            set([handle.all_gain_left3 handle.all_gain_leftp33 handle.all_gain_leftm33 handle.all_gain_leftp13 handle.all_gain_leftm13],'visible','off');
            set([handle.all_gain_left4 handle.all_gain_leftp34 handle.all_gain_leftm34 handle.all_gain_leftp14 handle.all_gain_leftm14],'visible','off');
            set([handle.all_gain_left5 handle.all_gain_leftp35 handle.all_gain_leftm35 handle.all_gain_leftp15 handle.all_gain_leftm15],'visible','off');
            set(handle.sel_gall_left,'position',[0.28 0.1 0.1 0.05]);
        end
end
