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

// create an Interval to periodically retrieve data from the python server
function start_interval() {
    stop_interval();
    send_new_pooling_wndlen();
    send_new_pooling_type();
    send_new_interval(interval);

    // periodically retrieve data from the Python server
    interval_id = setInterval(function() {
        timer.start = Date.now();
        ws.send(JSON.stringify({command: 'send_data'}));
    }, interval);
}

// stop the running Interval, if any
function stop_interval() {
    clearInterval(interval_id);
    interval_id = null;
}

function set_interval() {
    interval = parseInt(d3.select('#period').node().value);
    start_interval();
}

// tell the Python server the new pooling window length
function send_new_pooling_wndlen() {
    var new_pooling_wndlen = d3.select('#poolingwndlen').node().value;
    ws.send(JSON.stringify({new_pooling_wndlen: new_pooling_wndlen}));
    d3.select('#wndlen_txt').text(new_pooling_wndlen);
}

// tell the Python server the new pooling alpha value
function send_new_pooling_alpha() {
    var new_pooling_alpha = d3.select('#poolingalpha').node().value;
    ws.send(JSON.stringify({new_pooling_alpha: new_pooling_alpha}));
    d3.select('#alpha_txt').text(new_pooling_alpha);
}

// tell the Python server the new pooling type
function send_new_pooling_type() {
    var new_pooling_type = d3.select('#poolingtype').node().value;
    ws.send(JSON.stringify({new_pooling_type: new_pooling_type}));
}

function send_new_interval(interval) {
    ws.send(JSON.stringify({new_interval: interval/1000}));
}

// update the ratelimit-status span
function rate_limit_status_red(msg) {
    d3.select('#ratelimit-status-txt').text(msg);
    d3.select('#ratelimit-status-sym')
        .selectAll('circle')
        .style('fill', 'red');
}

// initialise the contents of the ratelimit-status span
function init_ratelimit_status() {
    var radius = 0.5;
    d3.select('#ratelimit-status-sym')
        .attr("width", radius + 'cm')
        .attr("height", radius + 'cm')
      .append('g')
      .append('circle')
        .attr('r', radius/2 + 'cm')
        .attr('cx', radius/2 + 'cm')
        .attr('cy', radius/2 + 'cm')
        .style('fill', 'green');
}

// reset the rate limit status display to "everything OK"
function reset_rate_limit_status() {
    d3.select('#ratelimit-status-txt').text('');
    d3.select('#ratelimit-status-sym')
        .selectAll('circle')
        .style('fill', 'green');
}

function toggle_rate_limiting() {
    do_rate_limiting = d3.select('#ratelimit').node().checked;
    timer.delta = 0;
}

//Toggle beamformer
function toggle_beamforming() {
    var beamformer=d3.select('#beamformer').node().checked
    ws.send(JSON.stringify({beamformer:beamformer}))
}

// toggle visibility of the rate limiting status display
function toggle_rate_limit_display() {
    if ( d3.select('#show-ratelimit-status').node().checked ) {
        d3.select('#ratelimit-status').style('visibility', 'visible');
    } else {
        d3.select('#ratelimit-status').style('visibility', 'hidden');
    }
}

// Handle page reload; since the WebSocket is closed and re-opened when the
// page is reloaded, waiting for it to open should work just fine.
ws.onopen = function() {
    send_new_pooling_wndlen();
    send_new_pooling_alpha();
    send_new_pooling_type();
    init_ratelimit_status();
    toggle_rate_limiting();

    if ( url_parameters.server_type === 'tcp' ) {
        d3.select('#pooling-controls')
            .style('visibility', 'hidden')
            .style('height', '0px');
    }

    if ( typeof twod_onopen != "undefined" ) {
        twod_onopen();
    }

    if ( typeof pseudo3d_onopen != "undefined" ) {
        pseudo3d_onopen();
    }

    if ( typeof polar_onopen != "undefined" ) {
        polar_onopen();
    }

    if ( typeof tiled_onopen != "undefined" ) {
        tiled_onopen();
    }
}
