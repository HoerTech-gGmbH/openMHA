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

function rate_limit_action(overshoot) {
    var duration = d3.select('#duration').node().value;
    d3.select('#numlines').node().value = Math.round(duration/(interval + overshoot));
    set_numlines();
}

function set_duration() {
    var duration = d3.select('#duration').node().value;

    time_axes.forEach(function (t) {
        t.scale.domain([0, duration/1000]);
        t.axis.scale(t.scale);
        t.group.call(t.axis);
    });

    interval = duration/num_lines;

    axes.forEach(function(a) {
        a.scale.domain([0, duration/1000]);
        a.group.call(a.axis);
    });

    // restart the Interval now because "interval" changed (but only if an
    // interval is currently active)
    if ( interval_id !== null ) {
        start_interval();
    }
}

function set_polar_style() {
    polar_style = d3.select('#polarstyle').node().value;

    if ( polar_style === 'lines' ) {
        data = data.reduce(function(prev, cur, i, arr) {
            prev.push(arr.splice(0, model_length)); return prev;
        }, [])
        make_lines_func = append_lines_hook;
        data_change_hook = data_change_lines;
        numlines_hook = numlines_lines;
    } else if ( polar_style === 'arcs' ) {
        data = Array.prototype.concat.apply([], data);
        make_lines_func = append_arcs_hook;
        data_change_hook = data_change_arcs;
        numlines_hook = numlines_arcs;
    }

    lines = make_lines(num_lines);

    plot_data();
}

function numlines_lines(new_num_lines) {
    if ( new_num_lines > num_lines ) {
        for ( var i = num_lines; i < new_num_lines; i++ ) {
            data.push([]);
        }
    } else {
        data.splice(new_num_lines, num_lines - new_num_lines);
    }
}

function numlines_arcs(new_num_lines) {
    if ( new_num_lines > num_lines ) {
        for ( var i = num_lines*model_length; i < new_num_lines*model_length; i++ ) {
            data.push(0);
        }
    } else {
        data.splice(new_num_lines*model_length, (num_lines - new_num_lines)*model_length);
    }
}

if ( polar_style === 'lines' ) {
    var numlines_hook = numlines_lines;
} else if ( polar_style === 'arcs' ) {
    var numlines_hook = numlines_arcs;
}

function set_numlines() {
    var new_num_lines = parseInt(d3.select('#numlines').node().value);

    numlines_hook(new_num_lines);
    num_lines = new_num_lines;
    lines = make_lines(num_lines);

    // update the duration now because num_lines changed
    set_duration();

    // normally set_duration() would restart the plotting interval, and thus
    // update the plot; however, if no interval is running we need to update
    // the plot now
    if ( interval_id === null ) {
        plot_data();
    }
}

function polar_onopen() {
    // only call this because it already calls set_duration()
    set_numlines();
}
