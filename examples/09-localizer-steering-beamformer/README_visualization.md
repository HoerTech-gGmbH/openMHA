# A localisation plug-in for the openMHA framework.

## Introduction

This project is at its core an implementation of a localisation algorithm based
on GCC-PHAT (see [0] and [1]).  This is done as a triplet of plug-ins for openMHA.

- feature extraction, i.e., the actual GCC-PHAT computation,
- classification (linear SVM followed by a sigmoid transform), and
- pooling (i.e., grouping values over a specified duration, by sum, maximum, or
  mean).

Furthermore, this project contains an assortment of (vaguely) related software,
namely:

- a Python class for communicating with an openMHA instance
- an HTML5 based visualisation of the localisation


[0] C. Knapp and G. C. Carter, “The generalized correlation method for
estimation of time delay,” IEEE Transactions on Acoustics, Speech and Signal
Processing, vol. 24, no. 4, pp. 320–327, Aug. 1976.

[1] H. Kayser and J. Anemüller, “A discriminative learning approach to
probabilistic acoustic source localization,” In: International Workshop on
Acoustic Echo and Noise Control (IWAENC 2014), pp. 100 -- 104, Antibes, France,
2014

## Project Structure

The project is roughly structured as follows:

- The Python class and the HTML5 based visualisation are both in the directory
  `visualisation_web/`.

## Installation

The preferred way of installing this project is, naturally, "The Easy Way".
If you cannot use conda for some reason, see "Getting the Python Dependencies 
Without Conda".

### The Easy Way

You will, of course, require an openMHA installation.

The next step is installing the dependencies of the visualisation itself.  The
recommended way is to use [conda](http://conda.pydata.org/docs/).  For this
purpose, an environment file `environment.yml` is included in this repository.
By executing the commands

    conda env create
    conda activate doasvm_demo

in the directory where this README_visualization.md file is located.
A conda environment that is identical to the one used to develop and
test this project will be created and activated.

If you wish to send data to the TCP server (see "Structure of the
Visualisation" below) from within MATLAB, you will additionally require the
Instrument Control Toolbox or Java support (i.e., do not pass the `-nojvm`
option).  For Octave, you will need the instrument-control package (installable
with the command `pkg install -forge instrument-control`) or Java support.

Finally, you will require a web browser with HTML5, JavaScript, and — most
importantly — *WebSocket* support.  See
[here](http://caniuse.com/#feat=websockets) for a list of browsers that
*should* work.  The visualisation was tested with Firefox, but any modern
browser should work.  Note that newer browsers should generally yield better
performance and should thus be given preference.

### Getting the Python Dependencies Without Conda

If you cannot use conda, or do not want to, then you must install the following
packages (e.g., with `pip` or the system package manager):

- [Python](http://www.python.org) (version 2.7 or 3.x, tested with 2.7 and
  3.3), and
- [Tornado](http://www.tornadoweb.org) (tested with 4.0.1; earlier versions
  tested with 3.1.1).

## Users Guide

The general structure of the visualisation will be explained, followed by a
description of the Python servers.

### Launching the Visualisation

To start the visualisation using the default options, simply run

    python visualisation_web/mha_server.py

Or, if you wish to feed your own data into the visualisation, run

    python visualisation_web/tcp_server.py

This will start the server and open the visualisation in a web browser.  See
the output of `python visualisation_web/mha_server.py --help` or `python
visualisation_web/tcp_server.py --help` for a list of options (e.g., the MHA's
host and port), and read on for a more thorough description.

### Structure of the Visualisation

The visualisation consists of two core components: a Python server (which
exists in two variants), and a set of web applications that run in a browser.

The web applications are written in HTML 5 and JavaScript.  Each of them
displays a live webcam feed and show a plot on top of it (except for the polar
plot, which lacks the webcam feed).  The plotting itself — which is implemented
using the [D3](http://d3js.org) JavaScript library — takes up most of
the code.  The data it displays is retrieved from either of the Python servers
over a WebSocket in the form of plain JSON (i.e., the data is sent as a string,
as opposed to being serialised and transferred as binary data).

The MHA Python server fulfills two purposes: it *serves* the web applications
over HTTP (thus enabling remote viewing), and it acts as a *bridge* between the
web application and an MHA instance.  For the latter, it receives commands from
the web app over the aforementioned WebSocket, and communicates with an MHA
instance over TCP via MHA's simple network protocol (see the file
`visualisation_web/MHAConnection.py` for the details).  Note that the TCP
connection is made on-demand, in order to accommodate other MHA clients, since
the MHA only supports a single TCP connection.  However, this does not appear
to add an appreciable amount of overhead.  Both the HTTP and WebSocket
components are implemented with the [Tornado](http://www.tornadoweb.org/)
Python library.

In summary, the basic network structure looks like this:

                     -[WebSocket]-
                    /             \
    Visualisation <-               -> Python Server <--[TCP]--> MHA
                    \             /
                     ----[HTTP]---

HTTP is for serving the web applications, and the WebSocket is for sending data
and commands.

The TCP server is identical to the MHA server, only that it receives data from
a TCP client instead of requesting it from an MHA instance.  It was written for
visualising data from different programming languages — primarily MATLAB, but
any language that supports TCP/IP will work, e.g., Python.

Two support files are provided: `visualisation_web/connect_to_webapp.m` for
MATLAB and Octave, and `visualisation_web/connect_to_webapp.py` for Python.
Both work basically the same way: they return a function (in MATLAB/Octave a
function *handle*) with which you can send data to the TCP server, along with
the object that encapsulates the TCP connection.  Additionally, the test file
`./tools/test_tcp_server.m` is provided for MATLAB/Octave, which sends test
data to the TCP server. See the documentation in the aforementioned files for
more information (e.g., `help test_tcp_server`,
`pydoc visualisation_web/connect_to_webapp.py`).

### The Visualisation Web Applications

The user facing part of the visualisation is the web application.
It consists of three components: a live webcam feed, a real-time plot, and
various user input elements.

There are currently four such web applications: a simple 2D plot, a pseudo-3D
plot, a polar plot, and a tiled waterfall plot.  Each will be described in
their own section.

In all cases, the plot is updated periodically and displayed in real-time.  It
does not display the raw data, but the *pooled* data, created from the raw
values by the pooling plug-in.  This results in a smoother, and hence more
useful, plot.

The individual plots will be described in the following sections, after the
web-application basics and their common controls are explained.

#### Web-app basics

There are just a couple of things to note here, so this section will be
relatively brief.

As mentioned above, the web-apps are served over HTTP, so they are accessed via
URLs.  Since these URLs can be somewhat complex due to the query strings used
to parameterise the web-apps, the HTTP server knows shortcuts for each plot
type: "2d", "pseudo3d", "polar", and "tiled".  They can be used in a URL like
so:

    http://example.com:8080/polar

In this case, the URL will redirect to the full URL of the polar plot (see "The
polar visualisation" below).

These shortcuts are primarily useful when trying to load a web-app when the
Python server is not running on the same PC and it is necessary to launch a
browser manually.

In addition to the above, each web-app has a list of plot types at the top.
The other plot types show up as clickable URLs, so it is easy to switch to a
different type of visualisation, or to open several in different browser
windows and/or tabs.  Note, however, that the Python servers and the web-apps
are not designed for this, so it might be too slow to be usable (except for
tabs, where it appears that modern browsers optimise away inactive tabs).

#### Common controls

The different web applications have almost the same controls throughout, hence
these will be explained separately in this section.

All web-apps have the same three basic sections:

- plot controls,
- pooling controls, and
- video controls (if there is a video element).

The plot controls consist of:

- "Start" (start the real-time plot),
- "Stop" (stop the real-time plot),
- "Period" (the duration between data retrievals from MHA), and
- "Maximum y value" (the upper limit of the y axis).  This is only available
  for line plots.

The pooling controls are made up of:

- "Window length" (the duration in milliseconds over which data is pooled),
- "Alpha" (a smoothing parameter used for some pooling types, lower means more
  smoothing), and
- "Type" (the pooling type, either "maximum", "sum", or "mean").

The video controls are self-explanatory:

- "Angular width", and
- "Flip horizontally".

Finally, each web-app provides rate limiting UI elements as the last set of
controls (which are also mostly self-explanatory):

- "Activate rate limiting",
- "Display rate limiting status", and
- "Status:".

The latter is followed by a status icon.  If everything is OK, it is colored
green.  If the rate limiting threshold is reached it turns red and is followed
by some text that shows by how much the update time has been exceeded.  The
status icon and text are only visible if the "Display rate limiting status"
checkbox is unchecked.  Furthermore, they can both be reset by clicking on the
status icon.

Note that the web-apps may add or remove controls, which is documented in their
respective sections.  It is also important to note that the pooling controls
are ineffective when using the TCP server.

#### The 2D visualisation

In the 2D visualisation, the live webcam feed and the plot are displayed over
each other, so that you can associate the localisation results with actual
objects/speakers.  To better align the two, you can specify the angular width
of the video in degrees.  In the future, a form of optical correction will be
applied to remove simple distortions from the webcam's lens.  You can also flip
the video horizontally, depending on which way the webcam is facing relative to
the display (the default assumes that both are facing the same way).

The user input elements are identical to those explained in the section "Common
controls" above.

#### The pseudo-3D visualisation

The pseudo-3D visualisation is basically a fake 3D view in which you can move a
virtual camera within the upper-right quadrant of the plane spanned by the x
and y axes.

Here, the pseudo-z-axis represents time, with the point at the origin being the
farthest into the past.  The live webcam is projected at the end of the
pseudo-z-axis (at t=0), so that the most recent line plot is displayed on top
of it, as with the 2D visualisation.  To better align the two, you can specify
the angular width of the video in degrees.  In the future, a form of optical
correction will be applied to remove simple distortions from the webcam's lens.
You can also flip the video horizontally, depending on which way the webcam is
facing relative to the display (the default assumes that both are facing the
same way).

The user input elements are identical to those explained in the section "Common
controls" above, with some exceptions:  in the main plot controls the
"Duration" control element has been replaced by two others:

- "Duration" (the amount of time captured by the visualisation), and
- "Number of line plots" (the number of plots along the pseudo-z-axis, or how
  many samples are taken from MHA).

Together, they determine the period between updates (i.e., the sampling
period).  Furthermore, there is a new section for controlling the perspective,
which consists of:

- "X Offset in degrees", and
- "Y Offset".

They determine the relative X and Y offset of the line plots to each other,
thus controlling the angle of the pseudo-z-axis (or the virtual camera, if you
will).  Note that the maximum y value also influences the perspective
indirectly, because it increases the length of the y axis, thus acting as a
sort of zoom.

As with all UIs, it is best to just play around with it to see the effect of
the different controls.

#### The polar visualisation

This visualisation uses polar coordinates, and consequently does not display a
live webcam.

The plot spans the full range from -180° to +180°, with one radial axis every
10°.  These radial axes represent time, with the point at the origin being the
farthest into the past, and their rotation angle corresponding to the
localisation angle.

This plot is available in two styles: the usual line plots, which are mapped to
polar coordinates and are scaled so as to not overlap too much, and filled
arcs, whose fill color is determined by the value of the probability function
(the higher the value, the darker the color).

The user input elements are identical to those explained in the section "Common
controls" above, with some exceptions:  the "Duration" control element in the
main plot controls has been replaced with three others:

- "Duration" (the amount of time captured by the visualisation),
- "Number of line plots" (the number of plots along the radial axes, or how
  many samples are taken from MHA), and
- "Style" (the style of the polar plot, one of "Line plots" or "Arcs", as
  described above).

Together, "Duration" and "Number of line plots" determine the period between
updates (i.e., the sampling period).  By increasing the number of line plots,
the plot will gain a pseudo-3D look, as if the viewer is looking from above.
However, it will also become slower.  Note that the "Arcs" style is
significantly slower than the "Line plots" style, but it also looks much
better.

Furthermore, the "Maximum y value" control element is missing, since it is does
not make sense in conjunction with the scaling mentioned above.  Also, since
there is no video element, there are no video controls.

As with all UIs, it is best to just play around with it to see the effect of
the different controls.

#### The tiled visualisation

This visualisation is stylistically similar to the "Arcs" style polar plot, and
depicts the same information, only that it uses Cartesian instead of polar
coordinates.  As such, it uses rectangles in place of arcs.

Here, the y axis represents time, with new values showing up at the top (0
seconds) and flowing down to the bottom.  Magnitude is encoded by the fill
color of the rectangle, with darker colors representing values closer to 1.

The user input elements are identical to those explained in the section "Common
controls" above, with one exception: the "Duration" control element in the main
plot controls has been replaced with two others:

- "Duration" (the amount of time captured by the visualisation), and
- "Number of line plots" (the number of plots along the y axis, or how many
  samples are taken from MHA).

Together, they determine the period between updates (i.e., the sampling
period).

As with all UIs, it is best to just play around with it to see the effect of
the different controls.

#### Changing the look

You can change the look of the visualisation (line colors, line thickness, font
size, etc.) by creating a file with the name
`visualisation_web/visualisation/css/site_custom.css`.  This file is loaded
after all other CSS files, thus it will override all default styles, with the
exception of styles that are dynamically computed via JavaScript (but it
doesn't make sense to override those anyway).  If it does not exist, then the
server will return a "file not found" error and nothing will happen.

#### Security Concerns

The web applications and Python servers are *not* designed with
security in mind. Thus, the HTTP and WebSocket servers only listen on localhost
by default.  If you want to make them accessible within a network, it is
*strongly* recommended to make sure that they cannot be reached from the internet.

Note that the TCP server *only* listens on localhost, since it may receive
arbitrary data and is thus a security hole.  Running it on a different
host than the one that sends it data is *not* supported.

#### Usability Notes

As of this writing, stopping the visualisation can lag by several seconds
(sometimes almost half a minute, at least when run over the network).  This is
not due to network or rendering lag, which can be verified by stopping or
altering whatever sends the data to the Python server.  More likely this is
because the browser still has several calls to the timer function queued up by
the time the "Stop" button is pressed.  This could be fixed by switching from
`setInterval()` to recursively calling `setTimeout()`, as mentioned
[here](https://developer.mozilla.org/en-US/docs/Web/API/WindowTimers/setInterval#Dangerous_usage).

## Credits

The following people contributed to this project:

- Hendrik Kayser: the author of the original Matlab implementation of the
  algorithm, and steered development of the visualisation.

- Marc Joliet: wrote the initial implementation of two of the MHA plug-ins (and
  added a feature to the later rewrite by Kamil), all of the prototype work,
  and authored the visualisation software.

- Kamil Adiloglu: rewrote the MHA plug-ins and authored the pooling plug-in.

Thanks also go to Giso Grimm (also of Hörtech), who gave valuable advice on the
plug-ins and on MHA itself.

D3.js is copyright (c) 2010-2015, Michael Bostock.  The full license can be
found in the file LICENSE.D3js.
