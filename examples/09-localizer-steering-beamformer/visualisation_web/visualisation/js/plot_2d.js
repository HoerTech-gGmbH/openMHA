"use strict";
// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

/*
 * global variables
 */

// TODO: accurately align the video with the plot
var vidwidth = d3.select('video').node().width;
var margin = {top: 20, right: 40, bottom: 50, left: 80},
    width = vidwidth, height = 300 - margin.top - margin.bottom,
    data = [];

/* get parameters from URL query strings */
var url_parameters = getURLParameters();
var min_angle = parseFloat(url_parameters.min_angle),
    max_angle = parseFloat(url_parameters.max_angle),
    model_length = parseFloat(url_parameters.model_length);
var angle_breadth = max_angle - min_angle;

/*
 * plot handling
 */

var x = d3.scale.linear()
    .domain([min_angle, max_angle])
    .range([0, width]);

var y = d3.scale.linear()
    .domain([0, 1])
    .range([height, 0]);

var xAxis = d3.svg.axis()
    .scale(x)
    .orient("bottom")
    .ticks(15);

var yAxis = d3.svg.axis()
    .scale(y)
    .orient("left");

var line = d3.svg.line()
    .interpolate("monotone")
    .x(function(d, i) { return x(i*angle_breadth/(model_length - 1) +
                                 min_angle); })
    .y(function(d) { return y(d); });

var svg = d3.select("#d3plot").append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
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
    .attr("dy", "-4.21em")
    .text("Probability");

/* SVG paths do not have sub-elements, therefor there is nothing to group (the
 * line is constructed using the d attribute).  Thus, it makes no sense to use
 * a data join, since there are no elements to map the data to. */
var path = svg.append("path")
    .datum(data)
    .attr("class", "line")
    .attr("d", line);

/*
 * WebSocket handling
 */

var ws = new WebSocket("ws://" + location.hostname + ":" + url_parameters.ws_port + "/ws");

function plot_data() {
    /* update the plot using a transition */
    path.attr("d", line);
}

ws.onmessage = function(event) {
    var new_data = JSON.parse(event.data).data;
    var N = data.length;

    /* replace the underlying data and update the plot */
    data.push.apply(data, new_data);
    data.splice(0, N);
    plot_data();

    rate_limit();
};
