# SuperButtons
Enables fancy home automation tasks using cheap 433 MHz remotes.

Monitors repeating code buttons (such as 433 remotes but can be used with any type of buttons) and distills the repeating code down to a small number of messages appropriate for an MQTT topic or scripting.

The elegant part (unlike my previous library [fancybutton](https://github.com/leifclaesson/fancybutton)) is that buttons _do not need to be pre-defined!_ SuperButtons keeps track of multiple buttons simultaneously, automatically.

The messages are:

- TALLY [count]: Happens each time we see a particular button code if we haven't seen it for at least 100 milliseconds.
- SOLID happens the _second_ time we see this particular button code, if we haven't seen it for at least 750 milliseconds.
- RELEASE [count] happens 100 milliseconds after we last saw this particular button code.
- MEDIUMPRESS happens when we've seen this code at least once every 100 milliseconds for 500 milliseconds.
- LONGPRESS happens when we've seen this code at least once every 100 milliseconds for 500 milliseconds.
- VERYLONGPRESS happens when we've seen this code at least once every 100 milliseconds for 1000 milliseconds.
- DONE [count] [flags] happens 750 milliseconds after we last saw this particular button code. This also reports the total number of individual pushes, and resets the count. Flags include: was solid, was medium press, was long press, was very long press

So, what's so useful about that?
Well, it turns out that with just these few messages, we get _all_ the information we need to do _anything we like_!

For example:

- Garage door opener (single, individual triggers)
Use the SOLID message, ignore all others.
You could alternatively use just LONGPRESS or VERYLONGPRESS.

Differentiate between short and long presses?
For short, monitor DONE with no long press flag
For long, monitor LONGPRESS.

Volume control (repeating)?
One click on TALLY 1 or SOLID. Start your repeat timer on LONGPRESS. Stop your timer on DONE. Ignore everything else.

Door chime?
Proper ding-dong action is achieved by closing your circuit on TALLY, and open it again on RELEASE. Ignore the count, and other messages.

Single-button internet radio station selector?
Play a "Ding" in a little speaker on each TALLY so you can keep track.
Take your station preset number based on DONE [count]. Five pushes = preset five.
LONGPRESS to stop.


There may be other use cases I haven't thought of. Let me know if I've missed one. See the example, too.

The few messages cas easily be transported on a single MQTT topic, in order to use as a universal 433 MHz receiver in every room. The rest can be scripted in openHAB or HomeAssistant (or whatever your preferred flavor is) without having to touch the microcontroller.


## Dependencies

No direct dependencies, but you'll need to feed it unique button codes from somewhere, for example the [rc-switch](https://github.com/sui77/rc-switch) library for decoding 433 MHz remotes.
