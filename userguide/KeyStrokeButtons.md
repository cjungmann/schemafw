# Keystroke-matched Buttons

This page assumes a basic understanding of buttons in the Schema Framework.
[Adding Buttons](AddingButtons.md) is available for review or study.

Providing keystroke shortcuts for web page actions can greatly speed usage of
the page for experienced users.

Rather than attempting to design a generalized language for providing page-based
keystroke actions, the framework piggy-backs on the facilities in place for handling
buttons.  The designer simple designates a keystroke for a button, and when a user
presses the keystroke, the framework will behave as if the button had been pressed.
That ensures that when several button attributes are necessary to initiate an action,
pressing the matching keystroke will submit the same data as the already-working button.
It also more easily allows for an expansion of button utilities without leaving keystroke
actions behind.

## Example: Add a Keystroke to a Button

The following example is a simple mode that presents a table with an **add** button
for introducing new records to the database.

~~~srm
main
   procedure     : App_Main_Page
   type          : table
   on_line_click : edit
   buttons
      button
         type  : add
         label : Add Record
         task  : ?add
         key   : alt-a
~~~

Note the final line under the button: `key : alt-a`.  With this instruction, if the
user presses Alt-A, the *Add Record* button will fire. 

## The Key Instruction

The key instruction associates a keystroke with the button.  When a keystroke can be matched
to a button, the button's action is executed.  A key instruction is a key name, optionally
preceded by zero or more key modifiers.  The modifiers and keyname are to be separated by a
single hyphen and no spaces.  Valid key instruction values include,

- Shift-A
- Control-Shift-Delete
- Control-Alt-Q

### Matching a Keystroke

The user keystroke matches the button if the *key value* matches, and the *key modifiers*
are the same.  If a user presses <nobr>Control-A,</nobr> it will not match <nobr>Control-Alt-A.</nobr>

### The Key Value

The key value corresponds to the key pressed.  Single characters are obvious whether they
are a letter, numeral, punctuation, or a symbol.  There are several keys on normal keyboard
whose names are more than one character in length.  They include:

- function keys *F1* - *F12*
- Keypad keys *Home*, *End*, *Insert*, *Delete*, *PgUp*, *PgDn*
- Arrow keys, named for the direction they point, *up*, *right*, *down*, and *left*.
- Special keys *Escape*, *Tab*, and *Backspace*
- The hyphen, or minus symbol should be named to avoid conflicts with the *-* delimiter
  of the key value.  That is, <nobr>*Control-minus*</nobr> and **not** <nobr>*Control--*</nobr>

The framework assumes the names of the named keys are as they are spelled in the list above.
For example, the framework recognized *backspace*, but not the common synonym *bs*.  Likewise,
Page up is *pgup*, not *pageup* and page down is *pgdn*.

### Key Modifiers

The framework recognizes three key modifiers: **Shift**, **Control**, and **Alt**
When specifying a *key* value, any modifiers should come before the key

### The Shift-key Modifier

Note that the **Shift** modifier is not recognized for characters on keys with two characters.
Examples of two-character keys are the numeric keys that contain a numeral and a symbol.  Holding
the shift-key while pressing the numeral 2 key will result in a *@* character.  Neither the *2*
nor the *@* can be shifted, so the shift value is completely ignored.

On the initial implementation, a capital C should be listed as <nobr>*shift-c*</nobr>.  This may
turn out to be too confusing, in which case the design decision will be reevaluated.

### Abbreviations

All values are set to lower case before making comparisons.

In practice, the framework matches only the first three characters of a multiple-character keyname,
so *ins* is the same as *insert* and *esc* is the same as *escape*.  This is to accommodate the
common abbreviations of these key values.

Similarly, the modifiers are abbreviated, but they are shorted to single characters, *a*, *c*, and
*s* for *Alt*, *Control*, and *Shift*, respectively.  This allows the designer to use alternate
spellings like *Control* vs *Ctrl* or *Alternate* vs *Alt*.

## Final Considerations

Although a the framework allows the designer to specify nearly any possible keystroke, be aware
that some keystrokes may result in conflicts.  Operating Systems and browsers map certain
keystrokes to specific actions within their domain.  Some examples of operating system key maps
are <nobr>Control-Alt-B</nobr> to open the default browser, and <nobr>Control-Alt-T</nobr> to open
a terminal window. F1 in almost every context opens a help window.  Browsers generally zoom in or
out with <nobr>Control-plus</nobr> and <nobr>Control-minus.</nobr>


