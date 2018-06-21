"use strict";

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

    if ( interval_id === null ) {
        plot_data();
    }
}

function twod_onopen() {
    /* interval is fully initialised starting here */
    set_y_max();
}
