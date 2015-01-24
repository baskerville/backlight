# NAME

*backlight* — get and set the backlight brightness.

# SYNOPSIS

	backlight COMMAND
	backlight [+|-]VALUE[%]

# DESCRIPTION

*backlight* lets you adjust the brightness of your display through the *Linux* *sysfs*.

# COMMANDS

- `current` — Shows the current brightness value.
- `maximum` — Shows the maximum brightness value that can be set.

Command names can be truncated if there's no ambiguity regarding their explicit counterpart.

A command consisting of a number, optionally followed by a percent sign, sets the brightness. If the number is prefixed by a plus or minus sign, the value is relative to the current brightness.

# EXAMPLES

This will output the current brightness level as percentage:

	perl -e 'print int(`backlight c` * 100 / `backlight m` + 0.5)'

This sxhkd configuration snippet will bind the brightness function keys (mostly found on laptops) to *backlight*:

	XF86MonBrightness{Up,Down}
		backlight {+,-}5%
