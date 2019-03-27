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
    interval += Math.round(overshoot);
    d3.select('#period').node().value = interval;
    start_interval();
}

function set_y_max() {
    var y_max = parseFloat(d3.select('#ymax').node().value);

    d3.select('#ymax_val').text(y_max);

    y.domain([0, y_max]);
    yAxis.scale(y)
        .tickValues(y.nice().ticks(10));
    yaxisg.call(yAxis);

    if ( typeof update_svg_height != "undefined" ) {
        update_svg_height();
    }

    if ( typeof update_zaxis != "undefined" ) {
        update_zaxis();
    }

    if ( typeof update_video_pos != "undefined" ) {
        update_video_pos();
    }

    // normally set_duration() would restart the plotting interval, and thus
    // update the plot; however, if no interval is running we need to update
    // the plot now
    if ( interval_id === null ) {
        plot_data();
    }
}

function twod_onopen() {
    /* interval is fully initialised starting here */
    set_y_max();
}
