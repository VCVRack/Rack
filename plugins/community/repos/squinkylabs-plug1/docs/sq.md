# S 
Work in progress

Keyboard mappings [here](./keymap.md)

## What works now

Plays sequence looped all the time.

Edit note attributes (pitch, start time, duration)

Draw Piano roll.

Undo / Redo

## What doesn't work.

There are obviously many missing features (clocking, start/stop, undo, saving sequences). There is no mouse interface. It looks horrible. There is no screen area top or bottom like you might expect to show where you are and what your are doing.

Note editor things that don't work:

* Viewport doesn't scroll in pitch, although time works.
* While you can edit and play at the same time, it's just by luck.
* Time units are always 1/16 notes.
* Insert note is always 1/4 (I think).

## Extending the length

There is a temporary hack to make it possible to lengthen a track. You may move the cursor past the end of the track. If you insert a note there, the track will be extended in units of 4/4 bars to accommodate the new note.

## Piano roll

It works like you would expect. Details are in [Keyboard Summary](./keymap.md).

Note that there is a blinking "DOS cursor". It is not connected to the mouse cursor in any way. All note editing is done via the keyboard an this cursor.