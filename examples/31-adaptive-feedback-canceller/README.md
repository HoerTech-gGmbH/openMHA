# How to use the adaptive feedback cancellation examples

There are three different examples of the adaptive feedback canceller (AFC). They vary in
applicability and usage to the real world. Note that the current state of the plugin is a 
very basic implementation. The only measure for decorrelation is a delay in the forward path. 
The plugin is still vulnerable against correlated signals, such as harmonic signals (e.g. 
speech, music, etc.).

General Information for configuring your Jack Server in example 2 and 3:
- sample rate is 44100
- frames/period is 512
- the first input and output channel of the audio device are used, respectively

1. The most rudimentary and basic example is **debugging**. In order to properly debug an AFC,
this example provides the possibility to examine different variable states after each audio block
processing. To start the example, run _run_afc_debug.m_ and follow the instructions on the screen.
For additional comparison in your debugging you can run _afc_standalone.m_, which is a Matlab-only
implementation of an AFC algorithm.
2. The **simulation** example runs in real-time via a Jack Server but the output is sent directly
to the input, so no loudspeakers or microphones are needed for this setup. Start a Jack Server and
run _run_afc_sim.m_ to start the example.
3. **live_example** shows how to use the AFC in a real-world application. Before starting the
algorithm you have to calibrate your microphone-loudspeaker-setup. There is no absolute calibration
needed, instead it is necessary that a 1.0 at the output is a 1.0 at the input. That way, the gain
value in the AFC plugin is calibrated. Run _get_calibration_level.m_ and add the resulting value
to the mha.afc.gain.gains value in _afc_live_example.cfg_. Next, in order to properly estimate
the feedback filter, the roundtrip latency has to be measured. You can do this via _jack_iodelay_:
    - Start a Jack Server
    - Start _jack_iodelay_ in your terminal
    - In your Jack server connect the output of _jack_iodelay_ to the playback channel of your device 
        and the capture channel of the device to the input of _jack_iodelay_ 
    - Take the 'total roundtrip latency' value (in samples, rounded down) and put it in _afc_live_example.cfg_ 
        as mha.afc.measured_roundtrip_latency
    - Stop _jack_iodelay_

    Then, all preparations are done and you can run _run_afc_live_example.m_.
    There might be a few problems to run into:
    - Depending on your individual configuration the CPU might be quite overloaded, since AFC is 
        a computationally intensive operation. For Linux users: make sure you are working with the lowlatency 
        kernel.
    - During development we encountered strange behaviour while testing with real equipment. There were 
        noises that interfered with the filter estimation and made the AFC unusable. We could not detect whether 
        this problem came from Jack or the soundcard. Stopping and restarting the Jack server solved the problem. 
        Note that after restarting Jack, the roundtrip latency might change. It is better to use a 
        measured_roundtrip_latency value that is a few samples below the actually measured value.
