"use strict";

/*
 * Display video from the webcam
 */

var errorCallback = function(e) {
    console.log('WebCam access rejected by the user!', e);
};

// ugh, not all browsers support the unprefixed function, and some not at all,
// see
// https://developer.mozilla.org/en-US/docs/NavigatorUserMedia.getUserMedia#Browser_compatibility
navigator.getUserMedia = navigator.getUserMedia ||
                            navigator.webkitGetUserMedia ||
                            navigator.mozGetUserMedia ||
                            navigator.msGetUserMedia;

navigator.getUserMedia({video: true}, function(localMediaStream) {
    var video = document.querySelector('video');
    video.src = window.URL.createObjectURL(localMediaStream);
}, errorCallback);
