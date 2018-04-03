"use strict";

/*
 * global variables
 */

var video = d3.select('video').node();
var margin = {top: 80, right: 70, bottom: 50, left: 80},
    // adjust width by a fudge factor to fit the plot with default settings
    // into the browser window
    width = video.width/2, height = video.width/2,
    data = [];

/* get parameters from URL query strings */
var url_parameters = getURLParameters();
var min_angle = parseFloat(url_parameters.min_angle),
    max_angle = parseFloat(url_parameters.max_angle),
    model_length = parseFloat(url_parameters.model_length);
var angle_breadth = max_angle - min_angle;
var num_lines = 20, axes_angle_increment = 10;
var polar_style = d3.select('#polarstyle').node().value;

function fill_data_lines() {
    for ( var i = 0; i < num_lines; i++ ) {
        data.push([]);
    }
}

function fill_data_arcs() {
    for ( var i = 0; i < num_lines*model_length; i++ ) {
        data.push(0);
    }
}

if ( polar_style === 'lines' ) {
    fill_data_lines();
} else if ( polar_style === 'arcs' ) {
    fill_data_arcs();
}


/*
 * helper functions
 */

var data_scale = 0.2;
function make_line(k) {
    return d3.svg.line.radial()
        .interpolate("monotone")
        .angle(function(d, i) {
            return deg2rad(i*angle_breadth/(model_length - 1) + min_angle);
        })
        .radius(function(d) { return y(-d*interval/500 + k*interval/1000); });
}

function make_arcs(k) {
    var arcs = [];
    for ( var n = 0; n < model_length; n++ ) {
        arcs.push(d3.svg.arc()
            .startAngle(
                deg2rad(((n-0.5)*angle_breadth/(model_length - 1) + min_angle) % 360)
            )
            .endAngle(
                deg2rad(((n+0.5)*angle_breadth/(model_length - 1) + min_angle) % 360)
            )
            .innerRadius(function() { return y(    k*interval/1000); })
            .outerRadius(function() { return y((k+1)*interval/1000); })
        );
    }
    return arcs;
}

function append_lines_hook(lines, i) { lines.push(make_line(i)); }
function append_arcs_hook(lines, i) { lines.push.apply(lines, make_arcs(i)); }
if ( polar_style === 'lines' ) {
    var make_lines_func = append_lines_hook;
} else if ( polar_style === 'arcs' ) {
    var make_lines_func = append_arcs_hook;
}

function make_lines(N) {
    var lines = [];
    for ( var i = 0; i < N; i++ ) {
        make_lines_func(lines, i);
    }
    return lines;
}

function make_axis() {
    var y = d3.scale.linear()
        .domain([0, 1])
        .range([height, 0]);

    var yAxis = d3.svg.axis()
        .scale(y)
        .orient("left")

    var yaxisg = svg.append("g")
        .attr("class", "y axis")
        .call(yAxis);

    return {scale: y, axis: yAxis, group: yaxisg};

}

function make_radial_axes(increment) {
    var axes = [];
    var N = 360/increment + 1, start_angle = increment - 180, start_angle_label = increment;
    for ( var i = 0; i < N; i++ ) {
        var axis = make_axis();

        axis.group.attr("transform", "rotate(" + (increment*i + start_angle) + ")");

        // make sure the labels go from (-180+increment)° to +180°
        if ( i >= N/2-1 ) {
            start_angle_label = increment - 360;
        }

        // add an axis label
        var axis_label_angle = i*increment + start_angle_label;
        if ( i < N-1 ) {
            if ( Math.abs(axis_label_angle) < 90 ) {
                var additional_transform = "rotate(180)";
                var y_offset = -0.71;
            } else {
                var additional_transform = "";
                var y_offset = 0.71;
            }

            axis.group.append("text")
                .attr("transform", "translate(0," + height + ") " + additional_transform)
                .attr("y", y_offset + "em")
                .text(axis_label_angle + "°");
        }

        axes.push(axis);
    }

    return axes;
}

function make_time_axes() {

    var orientations = ["right", "top"];
    var translations = [
        "translate(" + (width+margin.right/2) + "," + -height + ")",
        "translate(" + 0 + "," + -(height + margin.top/2) + ")",
    ];
    var ranges = [[0, height], [height, 0]];

    var t_axes = [];
    for ( var i in orientations ) {
        var t = d3.scale.linear()
            .domain([0, 1])
            .range(ranges[i]);

        var tAxis = d3.svg.axis()
            .scale(t)
            .orient(orientations[i]);

        var taxisg = svg.append("g")
            .attr("class", "t axis")
            .attr("transform", translations[i])
            .call(tAxis);

        // add an axis label
        if ( orientations[i] === "right" ) {
            taxisg.append("text")
                .attr("dy", "-1.21em")
                .text("Time [s]");
        }

        t_axes.push({scale: t, axis: tAxis, group: taxisg});
    }

    return t_axes;
}

function init_video_dims() {
    var video_container = d3.select('#video-container');
    var y_height = y(0) - y(1);

    video_container.style('height', y_height + 'px')
        .style('width', 2*width + 'px')
        .style('margin-left', margin.left + 'px');
}

/*
 * plot handling
 */

var svg = d3.select("#d3plot").append("svg")
    .attr("width", 2*width + margin.left + margin.right)
    .attr("height", 2*height + margin.top + margin.bottom)
    .append("g")
    .attr("id", "svg_group")
    // center the plot
    .attr("transform", "translate(" + (margin.left + width) + "," + (margin.top + height) + ")");

// Increase the width of the document body so that the UI elements are centered
// identically to the plot.
d3.select("body").style("width", 2*width + margin.left + margin.right + "px");

// create the axes
var axes = make_radial_axes(axes_angle_increment);
var time_axes = make_time_axes();

// Create global x and y objects for convenience; the x object especially is
// needed so that the video callbacks can be reused as-is.
var x = d3.scale.linear()
    .domain([min_angle, max_angle])
    .range([0, 2*width]),
    y = axes[Math.floor(axes.length/2)].scale;

init_video_dims();

/*
 * WebSocket handling
 */

var lines = make_lines(num_lines);
var svg_group = d3.selectAll('#svg_group')

function fill_function(d) {
    // TODO: use a color map (like in MATLAB/matplotlib/etc.)
    var k = 1 - d;
    return "rgb(" + k*100 + "%," + k*100 + "%," + k*100 + "%)";
}

function plot_data() {
    var path_selection = svg_group.selectAll('.line')
        .data(data);

    // Update existing elements.  Any pre-existing path elements now contain
    // data from the past, so they need to be moved down the pseudo-z-axis,
    // hence why the "d" attribute needs to be set here, too.
    //
    // Then add a path element for the new data array, which will be the last
    // element of the selection (i.e., i == num_lines - 1).
    if ( polar_style === 'lines' ) {
        path_selection
            .attr("class", "line")
            .attr("d", function(d, i) { return lines[i](d); } );

        path_selection.enter()
            .append("path")
            .attr("class", "line")
            .attr("d", function(d, i) { return lines[i](d); } );
    } else if ( polar_style === 'arcs' ) {
        path_selection
            .attr("class", "filled line")
            .attr("d", function(d, i) { return lines[i](); } )
            .style("fill", fill_function);

        path_selection.enter()
            .append("path")
            .attr("class", "filled line")
            .attr("d", function(d, i) { return lines[i](); } )
            .style("fill", fill_function);
    }

    // Remove any superfluous elements.
    path_selection.exit()
        .remove();
}

var ws = new WebSocket("ws://" + location.hostname + ":" + url_parameters.ws_port + "/ws");

function data_change_arcs(new_data) {
    data.unshift.apply(data, new_data);
    data.splice(model_length*num_lines, new_data.length);
}
function data_change_lines(new_data) {
    data.unshift(new_data);
    data.pop();
}
if ( polar_style === 'lines' ) {
    var data_change_hook = data_change_lines;
} else if ( polar_style === 'arcs' ) {
    var data_change_hook = data_change_arcs;
}

ws.onmessage = function(event) {
    var new_data = JSON.parse(event.data).data;

    data_change_hook(new_data);
    plot_data();

    rate_limit();
};
