"use strict";

/*
 * global variables
 */

var video = d3.select('video').node();
var margin = {top: 10, right: 40, bottom: 60, left: 140},
    // adjust width by a fudge factor to fit the plot with default settings
    // into the browser window
    width = video.width,
    height = 400 - margin.top - margin.bottom,
    data = [];

/* get parameters from URL query strings */
var url_parameters = getURLParameters();
var min_angle = parseFloat(url_parameters.min_angle),
    max_angle = parseFloat(url_parameters.max_angle),
    model_length = parseFloat(url_parameters.model_length);
var angle_breadth = max_angle - min_angle;
var num_lines = 20;

for ( var i = 0; i < num_lines*model_length; i++ ) {
    data.push(0);
}

/*
 * helper functions
 */

function make_axes() {
    var x = d3.scale.linear()
        .domain([min_angle, max_angle])
        .range([0, width]);

    var y = d3.scale.linear()
        .domain([0, interval])
        .range([0, height]);

    var xAxis = d3.svg.axis()
        .scale(x)
        .orient("bottom")
        .ticks(15);

    var yAxis = d3.svg.axis()
        .scale(y)
        .orient("left");

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
        .text("Time in seconds");

    return {x: {scale: x, axis: xAxis, group: xaxisg},
            y: {scale: y, axis: yAxis, group: yaxisg}};
}

/*
 * plot handling
 */

// create the axes
var axes = make_axes();

// Create global x and y objects for convenience; the x object especially is
// needed so that the video callbacks can be reused as-is.
var x = axes.x.scale,
    y = axes.y.scale;

/*
 * WebSocket handling
 */

var svg_group = d3.selectAll('#svg_group')

function fill_function(d) {
    // TODO: use a color map (like in MATLAB/matplotlib/etc.)
    var k = 1 - d;
    return "rgb(" + k*100 + "%," + k*100 + "%," + k*100 + "%)";
}

function plot_data() {
    var path_selection = svg_group.selectAll('rect')
        .data(data);

    var rect_width = x(angle_breadth/(model_length - 1)) - x(0);
    var rect_height = y(interval/1000) - y(0);

    // Update existing elements.  Any pre-existing path elements now contain
    // data from the past, so they need to be moved down the pseudo-z-axis,
    // hence why the "d" attribute needs to be set here, too.
    //
    // Then add a path element for the new data array, which will be the last
    // element of the selection (i.e., i == num_lines - 1).
    path_selection
        .attr("class", "filled rect")
        .attr("x", function(d, i) { return x((i%model_length)*angle_breadth/(model_length - 1) + min_angle) - rect_width/2; })
        .attr("y", function(d, i) { return y(Math.floor(i/model_length)*interval/1000) - rect_height/2; })
        .attr('width', rect_width)
        .attr('height', rect_height)
        .style("fill", fill_function);

    path_selection.enter()
        .append("rect")
        .attr("class", "filled rect")
        .attr("x", function(d, i) { return x((i%model_length)*angle_breadth/(model_length - 1) + min_angle) - rect_width/2; })
        .attr("y", function(d, i) { return y(Math.floor(i/model_length)*interval/1000) - rect_height/2; })
        .attr('width', rect_width)
        .attr('height', rect_height)
        .style("fill", fill_function);

    // Remove any superfluous elements.
    path_selection.exit()
        .remove();
}

var ws = new WebSocket("ws://" + location.hostname + ":" + url_parameters.ws_port + "/ws");

ws.onmessage = function(event) {
    var new_data = JSON.parse(event.data).data;

    data.unshift.apply(data, new_data);
    data.splice(model_length*num_lines, new_data.length);
    plot_data();

    rate_limit();
};
