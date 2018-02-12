"use strict";

function rate_limit_action(overshoot) {
    var duration = d3.select('#duration').node().value;
    d3.select('#numlines').node().value = Math.round(duration/(interval + overshoot));
    set_numlines();
}

function set_duration() {
    var duration = d3.select('#duration').node().value;

    interval = duration/num_lines;

    z.domain([0, duration/1000]);
    zaxisg.call(zAxis);

    // restart the Interval now because "interval" changed (but only if an
    // interval is currently active)
    if ( interval_id !== null ) {
        start_interval();
    }
}

function update_zaxis() {
    z.range([zaxis_length(), 0]);
    z_label.attr('x', z(0));
    zaxisg.attr("transform", "translate(0 " + height + ") rotate(" + zaxis_rotation_angle() + ")")
        .call(zAxis);
}

function set_numlines() {
    var new_num_lines = parseInt(d3.select('#numlines').node().value);

    if ( new_num_lines > num_lines ) {
        for ( var i = num_lines; i < new_num_lines; i++ ) {
            data.push([]);
        }
    } else {
        data.splice(new_num_lines, num_lines - new_num_lines);
    }

    num_lines = new_num_lines;
    lines = make_lines(num_lines);

    update_svg_height();
    update_zaxis();
    update_video_pos();

    if ( interval_id === null ) {
        plot_data();
    }

    // update the duration now because num_lines changed
    set_duration();
}

function update_video_pos() {
    var new_margin = x((num_lines-1)*x_offset) - x(0)
        + yaxisg.node().getBoundingClientRect().right
        - parseInt(d3.select('body').style('margin-left'));

    d3.select('#video-container')
        .style('margin-left', new_margin + 'px');
}

function set_xoffset() {
    x_offset = parseFloat(d3.select('#xoffset').node().value);

    d3.select('svg')
        .attr("width", width + margin.left + margin.right + x((num_lines-1)*x_offset) - x(0))
    update_zaxis();

    if ( interval_id === null ) {
        plot_data();
    }

    update_video_pos();
}

function update_svg_height() {
    y_offset = parseFloat(d3.select('#yoffset').node().value);
    var dy = y(0) - y((num_lines-1)*y_offset);
    d3.select('svg')
        .attr("height", height + margin.top + margin.bottom + dy)
    svg.attr("transform", "translate(" + margin.left + "," + (margin.top + dy) + ")");
    xaxisg.attr("transform", "translate(0," + height + ")")
        .call(xAxis);
}

function set_yoffset() {

    update_svg_height();
    update_zaxis();

    if ( interval_id === null ) {
        plot_data();
    }
}

function pseudo3d_onopen() {
    set_xoffset();
    set_yoffset()
    // only call this because it already calls set_duration()
    set_numlines();
}
