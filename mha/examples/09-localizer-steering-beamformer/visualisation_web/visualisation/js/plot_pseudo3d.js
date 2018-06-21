"use strict";

/*
 * global variables
 */

var video = d3.select('video').node();
var margin = {top: 20, right: 70, bottom: 50, left: 80},
    // adjust width by a fudge factor to fit the plot with default settings
    // into the browser window
    width = video.width,
    height = 400 - margin.top - margin.bottom,
    data = [];
var x_offset = 2, y_offset = 0.03;

/* get parameters from URL query strings */
var url_parameters = getURLParameters();
var min_angle = parseFloat(url_parameters.min_angle),
    max_angle = parseFloat(url_parameters.max_angle),
    model_length = parseFloat(url_parameters.model_length);
var angle_breadth = max_angle - min_angle;
var num_lines = 20;

for ( var i = 0; i < num_lines; i++ ) {
    data.push([]);
}

/*
 * helper functions
 */

function zaxis_rotation_angle() {
    return -rad2deg(Math.atan((y(0) - y(y_offset))/(x(x_offset) - x(0))));
}

function zaxis_length() {
    var dy = y(0) - y((num_lines-1)*y_offset);
    var dx = x((num_lines-1)*x_offset) - x(0);
    var dz = Math.sqrt(Math.pow(dy, 2) + Math.pow(dx, 2));
    return dz;
}

function make_line(k) {
    return d3.svg.line()
        .interpolate("monotone")
        .x(function(d, i) { return x(i*angle_breadth/(model_length - 1) +
                                     min_angle + k*x_offset); })
        .y(function(d) { return y(d + k*y_offset); });
}

function make_lines(N) {
    var lines = [];
    for ( var i = 0; i < N; i++ ) {
        lines.push(make_line(i));
    }
    return lines;
}

function init_video_dims() {
    var video_container = d3.select('#video-container');
    var y_height = y(0) - y(1);

    d3.select('#d3plot')
        .style('margin-top', -(margin.top + y_height) + 'px');

    video_container.style('height', y_height + 'px')
        .style('width', width + 'px');
}

/*
 * plot handling
 */

var x = d3.scale.linear()
    .domain([min_angle, max_angle])
    .range([0, width]);

var y = d3.scale.linear()
    .domain([0, 1])
    .range([height, 0]);

var z = d3.scale.linear()
    .domain([0, interval])
    .range([zaxis_length(), 0]);

var xAxis = d3.svg.axis()
    .scale(x)
    .orient("bottom")
    .ticks(15);

var yAxis = d3.svg.axis()
    .scale(y)
    .orient("left");

var zAxis = d3.svg.axis()
    .scale(z)
    .orient("top");

var svg = d3.select("#d3plot").append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("id", "svg_group")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

var xaxisg = svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .call(xAxis);

// add minor ticks to the x axis
xaxisg.selectAll("line")
  .data(x.ticks(angle_breadth/2), function(d) { return d; })
    .enter()
    .append("line")
    .attr("class", "minor")
    .attr("y1", 0)
    .attr("y2", 3)
    .attr("x1", x)
    .attr("x2", x);

// add an x axis label
xaxisg.append("text")
    .attr("x", x(max_angle))
    .attr("dy", "2.71em")
    .text("Angle in degrees");

var yaxisg = svg.append("g")
    .attr("class", "y axis")
    .call(yAxis);

// add a y axis label
yaxisg.append("text")
    .attr("transform", "rotate(-90)")
    .attr("y", 6)
    .attr("dy", "-3.21em")
    .text("Probability");

var zaxisg = svg.append("g")
    .attr("class", "z axis")
    .attr("transform", "translate(0 " + height + ") rotate(" + zaxis_rotation_angle() + ")")
    .call(zAxis);

var z_label = zaxisg.append("text")
    .attr("x", z(0))
    .attr("dy", "-2.21em")
    .text("Time in seconds")

init_video_dims();

/*
 * WebSocket handling
 */

var lines = make_lines(num_lines);
var svg_group = d3.selectAll('#svg_group')

function plot_data() {
    var path_selection = svg_group.selectAll('.line')
        .data(data);

    // Update existing elements.  Any pre-existing path elements now contain
    // data from the past, so they need to be moved down the pseudo-z-axis,
    // hence why the "d" attribute needs to be set here, too.
    path_selection
        .attr("d", function(d, i) { return lines[i](d); } );

    // Add a path element for the new data array, which will be the last
    // element of the selection (i.e., i == num_lines - 1).
    path_selection.enter()
        .append("path")
        .attr("class", "line")
        .attr("d", function(d, i) { return lines[i](d); } );

    // Remove any superfluous elements.
    path_selection.exit()
        .remove();
}

var ws = new WebSocket("ws://" + location.hostname + ":" + url_parameters.ws_port + "/ws");

ws.onmessage = function(event) {
    var new_data = JSON.parse(event.data).data;

    data.push(new_data);
    data.shift();
    plot_data();

    rate_limit();
};
