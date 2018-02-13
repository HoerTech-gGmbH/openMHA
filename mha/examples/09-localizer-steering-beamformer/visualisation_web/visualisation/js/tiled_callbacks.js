"use strict";

function rate_limit_action(overshoot) {
    var duration = d3.select('#duration').node().value;
    d3.select('#numlines').node().value = Math.round(duration/(interval + overshoot));
    set_numlines();
}

function set_axis_offsets() {
    var half_square_size = axes.x.scale(angle_breadth/(2*model_length)) - axes.x.scale(0);
    var line_offset = axes.y.scale(1)*interval/1000;
    var y_offset = Math.max(0, half_square_size - line_offset/2);
    var x_offset = -half_square_size - x(1) + x(0);

    axes.y.group
        .attr("transform", "translate(" + x_offset + ",0)")
        .call(axes.y.axis);

    d3.select('svg')
        .attr("width", width + margin.left + margin.right - 2*x_offset)
        .attr("height", height + margin.top + margin.bottom + 2*y_offset);
}

function init_video_dims() {
    var new_margin = (axes.x.group.node().getBoundingClientRect().left
                      - parseInt(d3.select('body').style('margin-left')));

    d3.select('#video-container')
        .style("width", axes.x.group.node().getBoundingClientRect().width + "px")
        .style('margin-left', new_margin + "px");
}

function set_duration() {
    var duration = d3.select('#duration').node().value;

    axes.y.scale.domain([0, duration/1000]);
    axes.y.axis.scale(axes.y.scale);
    axes.y.group.call(axes.y.axis);

    interval = duration/num_lines;

    // restart the Interval now because "interval" changed (but only if an
    // interval is currently active)
    if ( interval_id !== null ) {
        start_interval();
    }
}

function set_numlines() {
    var new_num_lines = parseInt(d3.select('#numlines').node().value);

    if ( new_num_lines > num_lines ) {
        for ( var i = num_lines*model_length; i < new_num_lines*model_length; i++ ) {
            data.push(0);
        }
    } else {
        data.splice(new_num_lines*model_length, (num_lines - new_num_lines)*model_length);
    }

    num_lines = new_num_lines;

    // update the duration now because num_lines changed
    set_duration();

    if ( interval_id === null ) {
        plot_data();
    }
}

function tiled_onopen() {
    // only call this because it already calls set_duration()
    set_numlines();
    set_axis_offsets();
    init_video_dims();
}
