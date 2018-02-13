"use strict";

var interval_id = null, interval = 20;
var plot_types = ['2d', 'pseudo3d', 'polar', 'tiled'];
var type_regex = /(.*)\/(\w+).html/;
var do_rate_limiting = true;
var timer = {start: 0, coeff: 0.95, delta: 0, thres: 10};

function rate_limit() {

    var delta = Date.now() - timer.start;

    timer.delta = timer.delta*timer.coeff + delta*(1 - timer.coeff);

    var overshoot = timer.delta - interval;
    if ( overshoot > timer.thres ) {
        rate_limit_status_red('(took ' + overshoot.toFixed(2) + ' ms longer than expected.)');

        if (do_rate_limiting) {
            console.log("Warning: waited " + overshoot + " ms too long.");
            rate_limit_action(overshoot);

            /* Reset timer.delta in order to keep the rate limiting from
             * engaging again too quickly. */
            timer.delta = 0;
        }
    }
}

/* return an object containing the URL parameters */
function getURLParameters() {
    var params = {};
    if (location.search) {
        var search = decodeURIComponent(location.search);
        var parts = search.substring(1).split('&');
        parts.forEach(function(p) {
            var pair = p.split('=');
            if (!pair[0]) return;
            params[pair[0]] = pair[1] || "";
        })
    }
    return params
}

function insert_plot_links() {
    var links_div = d3.select('#links-to-other-plots');
    var cur_type = location.pathname.replace(type_regex, "$2");

    links_div.append('span')
        .html("Plot types: ");

    // add one link for every other plot type
    for ( var i = 0; i < plot_types.length; i++ ) {
        if ( plot_types[i] === cur_type ) {
            links_div.append('span')
                .html(plot_types[i]);
        } else {
            links_div.append('a')
                .attr('href', "http://" + location.host + "/" + plot_types[i])
                .text(plot_types[i]);
        }
        links_div.append('span')
            .html("  ");
    }

    // Add *two* newlines to add some space between the <div> and the plots.
    links_div.append('br');
    links_div.append('br');
}

function rad2deg(rad) {
    return rad/Math.PI*180;
}

function deg2rad(deg) {
    return deg/180*Math.PI;
}

insert_plot_links();
